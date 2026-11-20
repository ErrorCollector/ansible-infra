# errorcollector.base

Base for every host: provisioning user, packages, Docker, shell, OS updates, node_exporter, shared tooling.

Part of [ansible-infra](https://github.com/ErrorCollector/ansible-infra). Install a released version (tag `base-vX.Y.Z`):

```yaml
collections:
    -   name: git+https://github.com/ErrorCollector/ansible-infra.git#/collections/base
        type: git
        version: base-v0.1.0
```
