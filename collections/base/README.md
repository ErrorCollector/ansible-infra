# errorcollector.base

Base for every host: provisioning user, packages, Docker, shell, OS updates, node_exporter, shared tooling.

Part of [ansible-infra](https://github.com/ErrorCollector/ansible-infra). Install a released version (tag `base-vX.Y.Z`):

```yaml
collections:
    -   name: git+https://github.com/ErrorCollector/ansible-infra.git#/collections/base
        type: git
        version: base-v0.1.0
```

## Project setup

One requirements file per domain, so Renovate opens one MR per domain and CI knows which domain changed:

```
requirements.d/base.yml       errorcollector.base (see above)
requirements.d/pangolin.yml   errorcollector.pangolin, …
requirements.d/galaxy.yml     everything else (community.docker, geerlingguy.*)
```

`ansible.cfg`:

```ini
[defaults]
collections_path = ./collections
roles_path = ./roles
```

`.gitignore`: `collections/`

`Makefile`, the targets come from `extras/Makefile.inc` (`make help`):

```make
# Shared targets: errorcollector.base (make help)
BASE_MK := collections/ansible_collections/errorcollector/base/extras/Makefile.inc

ifneq ($(wildcard $(BASE_MK)),)
include $(BASE_MK)
else
# First install: errorcollector.base brings Makefile.inc
install:
	docker run --rm -v "$(CURDIR):/work" -w /work alpine/ansible:2.21.0 sh -c \
		'apk add -q --no-cache git && ansible-galaxy collection install -r requirements.d/base.yml -p collections'
	@test -f $(BASE_MK) || (echo "❌ $(BASE_MK) missing in the installed errorcollector.base"; exit 1)
	$(MAKE) install
endif

# Project specific targets below, "## " adds them to make help
```

`docker-compose.yaml`, so `docker compose run --rm ansible …` works as before (e.g. for `ansible-vault edit`):

```yaml
include:
    -   path: collections/ansible_collections/errorcollector/base/extras/docker-compose.yaml
        project_directory: .
```

Variables a project can override before the `include`: `INVENTORY`, `INIT_INVENTORY`, `INIT_PLAYBOOK`,
`PLAYBOOK`, `VAULT_FILE`.

## Make targets

| Target | |
|---|---|
| `make install` | Install the collections and roles pinned in `requirements.d/` |
| `make versions` | Pinned vs. installed versions |
| `make plan [DOMAIN=…] [HOSTS=…] [TAGS=…]` | `--check --diff` |
| `make deploy [DOMAIN=…] [HOSTS=…\|all] [TAGS=…]` | Roll out, without `HOSTS` the hosts are chosen interactively |
| `make dev DOMAIN=pangolin SRC=../ansible-infra` | Use a local checkout of a collection instead of the pinned version |
| `make reboot HOSTS=…\|all [FORCE=1]` | Controlled reboot, one host at a time, only if needed (`errorcollector.base.reboot`) |
| `make reboot-status`, `os-info`, `ping`, `inventory`, `shell`, `init HOST=…` | |

`plan`, `deploy` and `reboot` refuse to run when an installed collection does not match its pin
(e.g. after pulling a Renovate MR without `make install`). A collection linked by `make dev` only prints a
warning; `make install` switches back to the pinned version.

`DOMAIN` selects the domain tag (`base`, `pangolin`, `gitlab`, `web`), `TAGS` a role or sub tag and wins over `DOMAIN`.

## Reboot

`errorcollector.base.reboot` reboots only when `dnf needs-restarting -r` says so and checks afterwards that all
containers with a restart policy run again, healthy containers are healthy again and node_exporter answers.
Options in `roles/reboot/meta/argument_specs.yml`, for example:

```yaml
# group_vars/all: node_exporter check from the Prometheus container (address = node_exporter_address)
reboot_monitoring_host: monitor01

# host_vars of the Pangolin host: Newt does not rebuild its tunnel on its own
reboot_restart_after:
    -   container: newt
        hosts: "{{ groups['newt'] }}"
```

## GitLab CI

`extras/gitlab-ci.yml` holds hidden jobs for plan and deploy; the logic is in the Make targets `ci-plan` and
`ci-deploy`. Include the file pinned to the same base tag as `requirements.d/base.yml`, then extend the jobs per domain
(see the header of the file):

```yaml
include:
    # renovate: datasource=github-tags packageName=ErrorCollector/ansible-infra depName=errorcollector.base extractVersion=^base-v(?<version>.+)$
    -   remote: https://raw.githubusercontent.com/ErrorCollector/ansible-infra/base-vX.Y.Z/collections/base/extras/gitlab-ci.yml

plan:pangolin:
    extends: .infra:plan
    variables:
        DOMAIN: pangolin

deploy:pangolin:
    extends: .infra:deploy
    variables:
        DOMAIN: pangolin
```

- `.infra:plan` (stage `test`): `--check` on protected branches, then the hosts with changes. No `--diff`: job logs
  are readable for every developer of the project and not every template keeps its secrets out of the diff; the
  full diff comes from `make plan` locally.
- `.infra:deploy` (stage `deploy`): manual job on the default branch, environment `production`, one deploy at a time
  per project (`resource_group`). `HOSTS` limits it (e.g. a canary group); the inventory group `gitlab_server` is
  always excluded, the GitLab server is deployed locally only.
- Protected file variables of the project: `INFRA_SSH_KEY` (private key of `provisioning`), `INFRA_VAULT_PASS` and
  `INFRA_KNOWN_HOSTS` (known_hosts of all hosts; host key checking is always on in CI, the jobs fail without it). Use these names rather than `ANSIBLE_*`: with a protected project
  variable of the same name, unprotected branches would silently fall back to a group or instance variable.
- The Renovate regex manager for `requirements.d/` also needs `.gitlab-ci.yml` in its file patterns and a second
  match string for the `remote:` line, so the include and `requirements.d/base.yml` are updated in the same MR.
