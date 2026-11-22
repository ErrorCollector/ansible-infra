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

With `DOMAIN` and without `HOSTS` (or `TAGS`), `plan` and the CI jobs only touch the hosts of the domain's inventory groups
(`DOMAIN_HOSTS_<domain>` in `extras/Makefile.inc`, e.g. `gitlab_server,gitlab_runners`); `HOSTS=all` lifts that.

`plan`, `deploy` and `reboot` refuse to run when an installed collection does not match its pin
(e.g. after pulling a Renovate MR without `make install`). A collection linked by `make dev` only prints a
warning; `make install` switches back to the pinned version.

`DOMAIN` selects the domain tag (`base`, `pangolin`, `gitlab`, `web`), `TAGS` a role or sub tag and wins over `DOMAIN`.

## Roles

| Role | Tags | |
|---|---|---|
| `errorcollector.base.init` | – | first contact as root: provisioning user, SSH keys, no root login, Docker |
| `errorcollector.base.common` | `base`, `common`, sub tags `shell`, `updates`, `firewall` | SSH keys, packages, registry logins, Docker log rotation, shell environment, dnf-automatic with update metrics, no firewalld |
| `errorcollector.base.node_exporter` | `base`, `node_exporter`, `updates` | node_exporter with the textfile collector |
| `errorcollector.base.collection_version` | domain tag | `infra_collection_version{collection,version}` of the deployed collection (textfile collector), run last by every domain playbook |
| `errorcollector.base.reboot` | – | controlled reboot (`make reboot`, below) |

The project `site.yml` imports `errorcollector.base.site` (common and node_exporter on all hosts). Project values:

```yaml
# group_vars/all
infra_brand: example                   # file and unit names: /etc/profile.d/example-prompt.sh, example-update-status.timer
common_ssh_keys:
    - "ssh-ed25519 AAAA... admin@example.org"
common_packages: [sudo, git, htop]
common_docker_registries:
    -   url: https://registry.example.org
        username: pull-user
        password: "{{ registry_pull_token }}"   # vault
```

Update metrics (textfile collector, refreshed hourly): `infra_reboot_required`, `infra_os_updates_pending`,
`infra_update_status_success`, `infra_update_status_query_success{query=…}`, `infra_update_status_timestamp_seconds`.

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
- `.infra:deploy` (stage `deploy`): manual job on the default branch, one environment per job (`production/<job>`: "Prevent outdated deployment jobs"
  only blocks a button once the same job ran in a newer pipeline), one deploy at a time
  per project (`resource_group`). `HOSTS` limits it; the inventory group `gitlab_server` is excluded unless the job
  sets `INFRA_GITLAB_SERVER: "true"` and `HOSTS: <server>`. Give the GitLab server a button of its own (it restarts
  the GitLab that runs the job) and keep `make deploy` locally as fallback.
- Protected file variables of the project: `INFRA_SSH_KEY` (private key of `provisioning`), `INFRA_VAULT_PASS` and
  `INFRA_KNOWN_HOSTS` (known_hosts of all hosts; host key checking is always on in CI, the jobs fail without it). Use these names rather than `ANSIBLE_*`: with a protected project
  variable of the same name, unprotected branches would silently fall back to a group or instance variable.
- Plan and deploy of a domain only appear when the push changes a file of the domain or one matching
  `INFRA_CHANGES` (default: `inventory.yml`, `ansible.cfg`, `site.yml`). Files of the domain:
  `requirements.d/<domain>.yml`, `{group_vars,host_vars}/*/<domain>.yml` and `<domain>.vault.yml`,
  `templates/<domain>/`. Ansible loads every file of `host_vars/<host>/`, so the variables of a domain go into
  `<domain>.yml` (secrets `<domain>.vault.yml`); what the project's own roles use (alone or together with a domain)
  stays in `vars.yml`/`vault.yml`. A Renovate MR for one collection therefore only offers that domain, a change of
  the GitLab variables only the gitlab jobs; docs, CI or project app roles offer none.
  Jobs without a domain set their own `INFRA_CHANGES` (glob with `{a,b}`, e.g. their role and
  `host_vars/<host>/{vars,vault}.yml`). A pipeline
  started by hand (*Run pipeline*), by the API or a schedule has every job. Jobs that follow a deploy extend
  `.infra:after-deploy` with the same variables.
- `.infra:status` and `.infra:reboot` only exist in pipelines with `INFRA_PIPELINE=reboot` on the default branch
  (*Run pipeline* offers the variable as a choice, a schedule sets it); such a pipeline has no plans and no deploys.
  Project jobs that should not run there either (e.g. a syntax check) extend `.infra:not-reboot`. `status` runs `os-info` and `reboot-status` and ends as warning (exit code 3) when
  hosts need a reboot. `reboot` (`make ci-reboot`, `HOSTS` required) reboots only the hosts that need it, one at a
  time, and shares the `resource_group` with the deploys. Like deploys it leaves out `gitlab_server` unless
  `INFRA_GITLAB_SERVER` is set; a runner must not reboot itself (bind the job to another runner with `tags`).
- The Renovate regex manager for `requirements.d/` also needs `.gitlab-ci.yml` in its file patterns and a second
  match string for the `remote:` line, so the include and `requirements.d/base.yml` are updated in the same MR.

## Collection versions on the hosts

Each domain playbook ends with `errorcollector.base.collection_version`, tagged with the domain only: a run of the whole
domain (`make deploy DOMAIN=…`, the CI deploy jobs, a full run) writes
`infra_collection_version{collection="pangolin",version="0.2.0"} 1` to the textfile collector; runs with other tags keep
the previous value. It never counts as a change in `make plan`. A dev link reports the galaxy.yml version with `-dev`.

Alert on hosts of one project that run different versions of a collection (e.g. a forgotten local deploy of the
GitLab server, or a host that was unreachable during the deploy):

```yaml
- alert: InfraCollectionVersionDrift
  expr: count by (project, collection) (count by (project, collection, version) (infra_collection_version)) > 1
  for: 2d
```
