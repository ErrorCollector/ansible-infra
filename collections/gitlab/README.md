# errorcollector.gitlab

GitLab server (Omnibus in Docker) and GitLab Runner.

Part of [ansible-infra](https://github.com/ErrorCollector/ansible-infra). Install a released version (tag `gitlab-vX.Y.Z`):

```yaml
collections:
    -   name: git+https://github.com/ErrorCollector/ansible-infra.git#/collections/gitlab
        type: git
        version: gitlab-v0.1.0
```
