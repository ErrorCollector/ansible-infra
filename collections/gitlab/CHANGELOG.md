# Changelog

## [0.6.1](https://github.com/ErrorCollector/ansible-infra/compare/gitlab-v0.6.0...gitlab-v0.6.1) (2026-11-24)


### Bug Fixes

* **gitlab:** update gitlab/gitlab-runner docker tag to alpine-v19.6.1 ([#63](https://github.com/ErrorCollector/ansible-infra/issues/63)) ([56dea11](https://github.com/ErrorCollector/ansible-infra/commit/56dea11955fcba826921b3bc93c25a2d31a3e186))

## [0.6.0](https://github.com/ErrorCollector/ansible-infra/compare/gitlab-v0.5.2...gitlab-v0.6.0) (2026-11-21)


### ⚠ BREAKING CHANGES

* **gitlab:** gitlab_backup_image is gone and the role refuses to run while it is set. Move the sidecar into gitlab_compose_extra_services with the same service name and definition to keep the container as it is.

### Features

* **gitlab:** extra compose services instead of the backup sidecar ([#56](https://github.com/ErrorCollector/ansible-infra/issues/56)) ([fb5a169](https://github.com/ErrorCollector/ansible-infra/commit/fb5a1694eefe99a8c5f0b773df272da92f8fe469))

## [0.5.2](https://github.com/ErrorCollector/ansible-infra/compare/gitlab-v0.5.1...gitlab-v0.5.2) (2026-11-21)


### Bug Fixes

* **gitlab:** keep the backup sidecar running and plans working ([#54](https://github.com/ErrorCollector/ansible-infra/issues/54)) ([b282b8a](https://github.com/ErrorCollector/ansible-infra/commit/b282b8ac7dcb75631ef78709710dd22044e90640))

## [0.5.1](https://github.com/ErrorCollector/ansible-infra/compare/gitlab-v0.5.0...gitlab-v0.5.1) (2026-11-20)


### Bug Fixes

* **deps:** update dependency awslabs/amazon-ecr-credential-helper to v0.13.0 ([#44](https://github.com/ErrorCollector/ansible-infra/issues/44)) ([a2bf97c](https://github.com/ErrorCollector/ansible-infra/commit/a2bf97c9789ad15e1a04920fa6ab667727a62dd3))
* **deps:** update postgres docker tag to v16.16 ([#45](https://github.com/ErrorCollector/ansible-infra/issues/45)) ([b19e533](https://github.com/ErrorCollector/ansible-infra/commit/b19e533bdc28939fc65b1f877dbf07569c21b07d))
* let Renovate cover every pinned version ([#39](https://github.com/ErrorCollector/ansible-infra/issues/39)) ([e3f574e](https://github.com/ErrorCollector/ansible-infra/commit/e3f574e9016b2f2ea9b8ae7849184b8b148b2ad1))

## [0.5.0](https://github.com/ErrorCollector/ansible-infra/compare/gitlab-v0.4.0...gitlab-v0.5.0) (2026-11-20)


### Features

* **gitlab:** deploy the GitLab server from its own CI job ([#33](https://github.com/ErrorCollector/ansible-infra/issues/33)) ([466be63](https://github.com/ErrorCollector/ansible-infra/commit/466be63fc7d391c81788bfbac7931a18ae82e42f))

## [0.4.0](https://github.com/ErrorCollector/ansible-infra/compare/gitlab-v0.3.0...gitlab-v0.4.0) (2026-11-20)


### Features

* export the deployed collection version per host ([#31](https://github.com/ErrorCollector/ansible-infra/issues/31)) ([8484473](https://github.com/ErrorCollector/ansible-infra/commit/8484473f772d21f9b00d422dcafa8de5f20d44b9))

## [0.3.0](https://github.com/ErrorCollector/ansible-infra/compare/gitlab-v0.2.0...gitlab-v0.3.0) (2026-11-20)


### Features

* **gitlab:** gitlab server role, local deploys only ([#27](https://github.com/ErrorCollector/ansible-infra/issues/27)) ([9822f77](https://github.com/ErrorCollector/ansible-infra/commit/9822f77e88a5fe121088a5a3de52d03c439679ff))

## [0.2.0](https://github.com/ErrorCollector/ansible-infra/compare/gitlab-v0.1.0...gitlab-v0.2.0) (2026-11-20)


### Features

* **gitlab:** gitlab_runner role ([#25](https://github.com/ErrorCollector/ansible-infra/issues/25)) ([4060d42](https://github.com/ErrorCollector/ansible-infra/commit/4060d4223f5e71a8b017d2d1a5b05fc692744505))
