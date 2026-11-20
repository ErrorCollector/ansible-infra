# errorcollector.pangolin

Pangolin server, Newt sites and the Pangolin machine client.

Part of [ansible-infra](https://github.com/ErrorCollector/ansible-infra). Install a released version (tag `pangolin-vX.Y.Z`):

```yaml
collections:
    -   name: git+https://github.com/ErrorCollector/ansible-infra.git#/collections/pangolin
        type: git
        version: pangolin-v0.1.0
```

## Roles

| Role | Tag | Inventory group | |
|---|---|---|---|
| `errorcollector.pangolin.pangolin` | `pangolin_server` | `pangolin_server` | Pangolin, Gerbil and Traefik, backup before every change |
| `errorcollector.pangolin.newt` | `newt` | `newt` | one Newt container per site (`newt_sites`) |
| `errorcollector.pangolin.pangolin_client` | `pangolin_client` | `pangolin_client` | machine client for private resources |

All of them have the domain tag `pangolin`. The project `site.yml` imports the domain playbook:

```yaml
-   import_playbook: errorcollector.pangolin.site
```

The playbook runs the server first: a Newt site restarts when the Pangolin of its `restart_with` group
(default `pangolin_server`) changed in the same run, because Newt does not rebuild its tunnel on its own.
Containers are only recreated when their compose file or `.env` changes, a run without changes restarts nothing.

Options are documented in `roles/*/meta/argument_specs.yml`.

## Newt sites

The default site uses the variables the projects already have (`pangolin_newt_dir`, `pangolin_newt_endpoint`,
`pangolin_newt_id`, `pangolin_newt_secret`). Further sites, e.g. a second Newt in another Pangolin:

```yaml
# group_vars/all
newt_sites:
    -   name: newt
        dir: /opt/newt
        endpoint: "{{ pangolin_newt_endpoint }}"
        id: "{{ pangolin_newt_id | default('') }}"
        secret: "{{ pangolin_newt_secret | default('') }}"
    -   name: newt-other
        dir: /opt/newt-other
        service: other_newt
        network: ""            # default bridge: the private resource is the host itself
        restart_with: ""       # its Pangolin is not in this inventory
        endpoint: https://pangolin.example.org
        id: "{{ other_newt_id | default('') }}"
        secret: "{{ other_newt_secret | default('') }}"
```

A site with an empty `id` is skipped on that host, so the list lives in `group_vars` and the ids in `host_vars`.
