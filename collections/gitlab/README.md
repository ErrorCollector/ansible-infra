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
| `errorcollector.gitlab.gitlab` | `gitlab`, `gitlab_server` | `gitlab_server` | GitLab CE (omnibus) with registry metadata database and backup cron jobs; **local deploys only** |
| `errorcollector.gitlab.gitlab_runner` | `gitlab`, `gitlab_runner` | `gitlab_runners` | GitLab Runner (docker executor), local or S3 cache, optional ECR pull, optional Docker data volume, cleanup timer |

The project `site.yml` imports `errorcollector.gitlab.site` (runners). Docker is only restarted when `daemon.json` or the
containerd config change and the runner container only when its compose file changes; both abort running jobs, so roll
out a runner from another runner (CI job bound to the other runner) or locally.

```yaml
# group_vars/gitlab_runners
gitlab_runner_url: https://gitlab.example.org
# host_vars/<runner>: gitlab_runner_id, gitlab_runner_concurrent; vault: gitlab_runner_token
```

The GitLab server comes from a separate playbook, never from CI: `ci-deploy` excludes the group `gitlab_server` and the
role refuses to run under GitLab CI (dry runs are fine). Deploy it locally before its runners:

```yaml
# site.yml of the project
-   import_playbook: errorcollector.gitlab.server     # make deploy DOMAIN=gitlab HOSTS=<server>
-   import_playbook: errorcollector.gitlab.site       # runners
```

```yaml
# host_vars/<server>
gitlab_hostname: gitlab.example.org
gitlab_dir: /opt/gitlab
gitlab_home: /srv/gitlab
gitlab_ports: ["80:80", "443:443", "22:22", "5050:5050"]
gitlab_env_template: "{{ playbook_dir }}/templates/gitlab.env.j2"
gitlab_backup_s3_bucket: gitlab-backup.example.org
gitlab_omnibus_config: |
    gitlab_rails['smtp_password'] = '{{ vault_smtp_password }}'
    registry_external_url 'https://gitlab.example.org:5050'
```

`gitlab_omnibus_config` is everything after `external_url` in `GITLAB_OMNIBUS_CONFIG`. A change of it or of the version
recreates GitLab (minutes of downtime, upgrade migrations).
