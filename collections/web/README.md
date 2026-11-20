# errorcollector.web

Traefik reverse proxy.

Part of [ansible-infra](https://github.com/ErrorCollector/ansible-infra). Install a released version (tag `web-vX.Y.Z`):

```yaml
collections:
    -   name: git+https://github.com/ErrorCollector/ansible-infra.git#/collections/web
        type: git
        version: web-v0.1.0
```

## Roles

| Role | Tags | Inventory group | |
|---|---|---|---|
| `errorcollector.web.traefik` | `web`, `traefik` | `traefik` | Traefik v3 with Let's Encrypt, TLS 1.2+ AEAD only, redirects, local plugins |

The project `site.yml` imports the domain playbook after `errorcollector.pangolin.site` (Traefik joins the pangolin
network) and before the host plays (their containers join the traefik network):

```yaml
-   import_playbook: errorcollector.web.site
```

Project values (options in `roles/traefik/meta/argument_specs.yml`):

```yaml
traefik_acme_email: ops@example.org          # required
traefik_redirects:
    -   host: www.example.org
        target: https://example.org
        permanent: true
traefik_local_plugins:                        # e.g. on one host only (host_vars)
    -   name: myPlugin
        module: github.com/example/plugin
        repo: https://github.com/example/plugin.git
        # renovate: datasource=github-tags depName=example/plugin
        version: v1.0.0
```

Containers join the external docker network `traefik` and set their labels (`traefik.enable: "true"`,
`traefik.http.routers.<name>.rule`). The entrypoint `websecure` uses Let's Encrypt by default. Dashboard, API and ping
are on the internal entrypoint `dashboard` (`traefik_dashboard_port`, default 8080), reachable through the docker network
`pangolin` only, e.g. as a Pangolin resource with the target `traefik:8080`.
