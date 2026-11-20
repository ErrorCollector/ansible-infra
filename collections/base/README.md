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

Variables a project can override before the `include`: `INVENTORY`, `INIT_INVENTORY`, `INIT_PLAYBOOK`,
`PLAYBOOK`, `VAULT_FILE`.

## Make targets

| Target | |
|---|---|
| `make install` | Install the collections pinned in `requirements.d/` |
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
