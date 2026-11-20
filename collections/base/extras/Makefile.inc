# --------------------------------------------------
# errorcollector.base – shared Make targets of the project repos
# Included by the project Makefile (see README.md of errorcollector.base).
# Targets with "## " show up in `make help`, project targets as well.
# --------------------------------------------------

# pipefail: errors in pipes (e.g. docker compose | sed | sort) are not swallowed
SHELL         := /bin/bash
.SHELLFLAGS   := -o pipefail -c
.DEFAULT_GOAL := help

EXTRAS_DIR      := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
COLLECTIONS_DIR := collections/ansible_collections/errorcollector
DOMAINS         := base pangolin gitlab web

INVENTORY      ?= inventory.yml
INIT_INVENTORY ?= $(INVENTORY)
INIT_PLAYBOOK  ?= playbooks/init.yml
PLAYBOOK       ?= site.yml
VAULT_FILE     ?= .ansible_vault_pass

# In CI the paths only come from the project Makefile or these defaults, never from pipeline variables: with a comma
# INVENTORY becomes a host list (secrets deployed to any host), PLAYBOOK and VAULT_FILE could point anywhere.
ifdef GITLAB_CI
$(foreach var,INVENTORY INIT_INVENTORY INIT_PLAYBOOK PLAYBOOK VAULT_FILE,$(if $(filter environment command,$(firstword $(origin $(var)))),$(error $(var) must not be set by the environment or the command line in CI)))
endif

# In GitLab CI the job image is the Ansible runner, locally it is the container from docker-compose.yaml.
# Collections linked by `make dev` point outside the project and are mounted at the same path.
ifdef GITLAB_CI
ANSIBLE    :=
ANSIBLE_SH := sh -c
else
DEV_MOUNTS := $(foreach link,$(shell find $(COLLECTIONS_DIR) -mindepth 1 -maxdepth 1 -type l 2>/dev/null),-v $(realpath $(link)):$(realpath $(link)):ro)
ANSIBLE    := docker compose -f $(EXTRAS_DIR)/docker-compose.yaml --project-directory . run --rm $(DEV_MOUNTS) ansible
ANSIBLE_SH := $(ANSIBLE) sh -c
endif

# HOSTS=all or empty: no --limit. TAGS wins over DOMAIN (the domain tag).
LIMIT_ARG = $(if $(filter-out all,$(HOSTS)),--limit $(HOSTS))
TAGS_ARG  = $(if $(or $(TAGS),$(DOMAIN)),--tags $(or $(TAGS),$(DOMAIN)))

ifneq ($(DOMAIN),)
ifeq ($(filter $(DOMAIN),$(DOMAINS)),)
$(error DOMAIN must be one of: $(DOMAINS))
endif
endif

# --------------------------------------------------
# Safety checks
# --------------------------------------------------

.PHONY: check-vault
check-vault:
	@test -f $(VAULT_FILE) || (echo "❌ Missing $(VAULT_FILE)"; exit 1)

# Targets with 2>/dev/null would otherwise stay silently empty when Docker is not running
# HOSTS, TAGS & co. end up in the command line. In CI they can come from pipeline variables (Run pipeline), so only
# plain host patterns and tags are accepted; the shell reads the values from the environment, not Make. The paths
# (INVENTORY, PLAYBOOK, VAULT_FILE) cannot be set from outside in CI at all (see above).
.PHONY: check-args
check-args:
	@for value in "$$HOSTS" "$$TAGS" "$$DOMAIN" "$$INVENTORY" "$$PLAYBOOK" "$$VAULT_FILE"; do \
		case "$$value" in *[!A-Za-z0-9_.,:*!/-]*) \
			echo "❌ HOSTS, TAGS, DOMAIN, INVENTORY, PLAYBOOK and VAULT_FILE may only contain letters, digits and _ . , : * ! / -"; \
			exit 1;; \
		esac; \
	done

.PHONY: check-docker
check-docker:
ifndef GITLAB_CI
	@docker info >/dev/null 2>&1 || (echo "❌ Docker is not running (start Docker Desktop: open -a Docker)"; exit 1)
endif

# Never roll out an outdated collection, e.g. after pulling a Renovate MR without `make install`
.PHONY: check-collections
check-collections:
	@bash $(EXTRAS_DIR)/collections.sh --check

# --------------------------------------------------
# Collections
# --------------------------------------------------

# ansible-galaxy install takes role and collection requirements; they land in roles_path and
# collections_path of the project ansible.cfg (./roles, ./collections)
.PHONY: install
install: check-docker ## Install the collections and roles pinned in requirements.d/ (also undoes make dev)
	@# make dev links are removed inside the container: base may be one of them and holds docker-compose.yaml
	$(ANSIBLE_SH) 'command -v git >/dev/null || apk add -q --no-cache git; \
		find $(COLLECTIONS_DIR) -mindepth 1 -maxdepth 1 -type l -exec rm {} \; 2>/dev/null; \
		for file in requirements.d/*.yml; do ansible-galaxy install -r "$$file" --force || exit 1; done'

.PHONY: versions
versions: ## Pinned vs. installed collection versions
	@bash $(EXTRAS_DIR)/collections.sh

.PHONY: dev
dev: ## Use a local checkout of a collection: DOMAIN=pangolin SRC=../ansible-infra (undo: make install)
	@test -n "$(DOMAIN)" -a -n "$(SRC)" || (echo "Usage: make dev DOMAIN=pangolin SRC=../ansible-infra"; exit 1)
	@test -f "$(SRC)/collections/$(DOMAIN)/galaxy.yml" || (echo "❌ $(SRC)/collections/$(DOMAIN)/galaxy.yml not found"; exit 1)
	@mkdir -p $(COLLECTIONS_DIR)
	@rm -rf $(COLLECTIONS_DIR)/$(DOMAIN)
	@ln -s $(realpath $(SRC))/collections/$(DOMAIN) $(COLLECTIONS_DIR)/$(DOMAIN)
	@echo "errorcollector.$(DOMAIN) → $(realpath $(SRC))/collections/$(DOMAIN) (back to the pinned version: make install)"

# --------------------------------------------------
# Rollout
# --------------------------------------------------

.PHONY: plan
plan: check-args check-vault check-collections ## Dry run with diff: [DOMAIN=…] [HOSTS=h1,h2] [TAGS=…]
	$(ANSIBLE) ansible-playbook -i $(INVENTORY) $(LIMIT_ARG) $(TAGS_ARG) --check --diff $(PLAYBOOK)

.PHONY: deploy
deploy: check-args check-vault check-collections ## Roll out: [DOMAIN=…] [HOSTS=h1,h2|all] [TAGS=…], without HOSTS: choose interactively
	@if [ -n "$(HOSTS)" ]; then \
		$(ANSIBLE) ansible-playbook -i $(INVENTORY) $(LIMIT_ARG) $(TAGS_ARG) $(PLAYBOOK); \
	else \
		$(MAKE) --no-print-directory _deploy_interactive; \
	fi

.PHONY: _deploy_interactive
_deploy_interactive:
	@hosts=$$($(ANSIBLE) ansible all -i $(INVENTORY) --list-hosts | tail -n +2 | sed 's/^ *//'); \
	echo ""; \
	echo "Hosts:"; \
	echo "$$hosts" | nl -w2 -s'. '; \
	echo ""; \
	echo "  all            → all hosts"; \
	echo "  host1,host2    → these hosts"; \
	echo ""; \
	read -p "Deploy to: " selection; \
	test -n "$$selection" || { echo "Nothing selected"; exit 1; }; \
	$(MAKE) --no-print-directory deploy HOSTS="$$selection"

# Alias of the old Makefiles
.PHONY: provisioning
provisioning: deploy

.PHONY: shell
shell: check-args check-vault ## Shell environment only (no container restarts): [HOSTS=h1,h2], default all hosts
	$(ANSIBLE) ansible-playbook -i $(INVENTORY) $(LIMIT_ARG) --tags shell $(PLAYBOOK)

# --------------------------------------------------
# GitLab CI (extras/gitlab-ci.yml)
# The inventory group gitlab_server is excluded from CI deploys. Only a job of its own opts in with
# INFRA_GITLAB_SERVER=true and HOSTS=<server>: it restarts the GitLab that runs it.
# --------------------------------------------------

ifeq ($(INFRA_GITLAB_SERVER),true)
CI_LIMIT = --limit '$(HOSTS)'
else
CI_LIMIT = --limit '$(if $(filter-out all,$(HOSTS)),$(HOSTS),all),!gitlab_server'
endif

.PHONY: ci-plan
# Without --diff: job logs are readable for every developer of the project, and not every template keeps its
# secrets out of the diff. The full diff: make plan (locally).
ci-plan: check-args check-vault check-collections ## CI: dry run (changed tasks per host, no diff), then the hosts with changes: [DOMAIN=…] [HOSTS=…]
	$(ANSIBLE) ansible-playbook -i $(INVENTORY) $(LIMIT_ARG) $(TAGS_ARG) --check $(PLAYBOOK) | tee ci-plan.log
	@echo ""
	@sed 's/\x1b\[[0-9;]*m//g' ci-plan.log | awk '/^PLAY RECAP/ { recap = 1; next } \
		recap && /changed=[1-9]|failed=[1-9]|unreachable=[1-9]/ { found = 1; print "  " $$0 } \
		END { if (!found) print "  no changes" }' | (echo "Hosts with changes:"; cat)

.PHONY: ci-deploy
ci-deploy: check-args check-ci check-vault check-collections ## CI: roll out without the GitLab server: [DOMAIN=…] [HOSTS=…|all] [INFRA_GITLAB_SERVER=true HOSTS=<server>]
	$(ANSIBLE) ansible-playbook -i $(INVENTORY) $(CI_LIMIT) $(TAGS_ARG) $(PLAYBOOK)

.PHONY: ci-reboot
ci-reboot: check-args check-ci check-vault check-collections ## CI: reboot the hosts that need it, one at a time, without the GitLab server: HOSTS=… [FORCE=1] [INFRA_GITLAB_SERVER=true]
	@test -n "$(HOSTS)" || (echo "❌ HOSTS is required (a runner must not reboot itself, see extras/gitlab-ci.yml)"; exit 1)
	$(ANSIBLE) ansible-playbook -i $(INVENTORY) $(CI_LIMIT) $(if $(FORCE),-e reboot_force=true) errorcollector.base.reboot

.PHONY: check-ci
check-ci:
ifndef GITLAB_CI
	$(error ci-deploy and ci-reboot only run in GitLab CI, locally: make deploy / make reboot)
endif
	@if [ "$(INFRA_GITLAB_SERVER)" = true ] && { [ -z "$(HOSTS)" ] || [ "$(HOSTS)" = all ]; }; then \
		echo "❌ INFRA_GITLAB_SERVER=true needs HOSTS=<server>"; exit 1; fi

# --------------------------------------------------
# Hosts
# --------------------------------------------------

.PHONY: init
init: check-vault ## Bootstrap a new host: HOST=…
ifndef HOST
	$(error HOST is required. Usage: make init HOST=runner01)
endif
	$(ANSIBLE) ansible-playbook -i $(INIT_INVENTORY) --limit $(HOST) $(INIT_PLAYBOOK)

.PHONY: ping
ping: check-args ## Ping all hosts: [HOSTS=h1,h2]
	$(ANSIBLE) ansible $(or $(HOSTS),all) -i $(INVENTORY) -m ping

.PHONY: inventory
inventory: ## Show the resolved inventory
	$(ANSIBLE) ansible-inventory -i $(INVENTORY) --list

.PHONY: os-info
os-info: check-args check-docker ## Distribution and kernel: [HOSTS=h1,h2]
	@$(ANSIBLE) ansible $(or $(HOSTS),all) -i $(INVENTORY) -o -m shell -a \
		'. /etc/os-release; echo "$$PRETTY_NAME;$$(uname -r)"' \
		2>/dev/null | sed 's/ | CHANGED | rc=0 | (stdout) /;/' | sort -t ';' -k3,3V -k1,1 \
		| (echo "HOST;DISTRIBUTION;KERNEL"; cat) | column -t -s ';'

# --------------------------------------------------
# OS updates & reboots
# dnf-automatic installs updates daily, nothing reboots automatically.
# --------------------------------------------------

.PHONY: reboot-status
reboot-status: check-args check-docker ## Which hosts need a reboot? [HOSTS=h1,h2]
	@out=$$($(ANSIBLE) ansible $(or $(HOSTS),all) -i $(INVENTORY) -o -m shell -a \
		'dnf -q needs-restarting -r >/dev/null 2>&1 && r=no || r=YES; echo "$$r;$$(uname -r);$$(uptime -p | sed "s/^up //")"' \
		2>/dev/null | sed -E 's/ \| CHANGED \| rc=0 \| \(stdout\) /;/; s/ \| (UNREACHABLE|FAILED).*/;ERROR;-;-/' \
		| LC_ALL=C sort -t ';' -k2,2 -k1,1); rc=$$?; \
	(echo "HOST;REBOOT;KERNEL;UPTIME"; echo "$$out") | column -t -s ';'; \
	todo=$$(echo "$$out" | awk -F ';' '$$2 == "YES" { print $$1 }' | paste -sd, -); \
	echo ""; \
	if [ -n "$$todo" ]; then echo "→ make reboot HOSTS=$$todo"; else echo "✅ No host needs a reboot"; fi; \
	exit $$rc

.PHONY: reboot
reboot: check-args check-vault check-collections ## Controlled reboot, one host at a time, only if needed: HOSTS=h1,h2|all [FORCE=1]
ifndef HOSTS
	$(error HOSTS is required. Usage: make reboot HOSTS=mail01 (or HOSTS=all))
endif
	$(ANSIBLE) ansible-playbook -i $(INVENTORY) $(LIMIT_ARG) $(if $(FORCE),-e reboot_force=true) errorcollector.base.reboot

# --------------------------------------------------
# Help
# --------------------------------------------------

.PHONY: help
help: ## Show this help
	@echo ""
	@awk 'BEGIN { FS = ":[^#]*## " } /^[a-zA-Z0-9_-]+:[^#]*## / { printf "  make %-15s %s\n", $$1, $$2 }' $(MAKEFILE_LIST)
	@echo ""
	@echo "  DOMAIN = $(DOMAINS)   HOSTS = h1,h2 | all   TAGS = role or sub tag (wins over DOMAIN)"
	@echo ""
