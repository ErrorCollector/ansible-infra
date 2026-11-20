# errorcollector.gitlab

GitLab server (Omnibus in Docker) and GitLab Runner.

Part of [ansible-infra](https://github.com/ErrorCollector/ansible-infra). Install a released version (tag `gitlab-vX.Y.Z`):

```yaml
collections:
    -   name: git+https://github.com/ErrorCollector/ansible-infra.git#/collections/gitlab
        type: git
        version: gitlab-v0.1.0
```

## Roles

| Role | Tags | Inventory group | |
|---|---|---|---|
| `errorcollector.gitlab.gitlab_runner` | `gitlab`, `gitlab_runner` | `gitlab_runners` | GitLab Runner (docker executor), local or S3 cache, optional ECR pull, optional Docker data volume, cleanup timer |

The project `site.yml` imports `errorcollector.gitlab.site` (runners). Docker is only restarted when `daemon.json` or the
containerd config change and the runner container only when its compose file changes; both abort running jobs, so roll
out a runner from another runner (CI job bound to the other runner) or locally.

```yaml
# group_vars/gitlab_runners
gitlab_runner_url: https://gitlab.example.org
# host_vars/<runner>: gitlab_runner_id, gitlab_runner_concurrent; vault: gitlab_runner_token
```
