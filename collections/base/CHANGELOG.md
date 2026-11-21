# Changelog

## [0.10.0](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.9.0...base-v0.10.0) (2026-11-21)


### Features

* **base:** separate reboot pipeline via INFRA_PIPELINE ([#48](https://github.com/ErrorCollector/ansible-infra/issues/48)) ([d7538de](https://github.com/ErrorCollector/ansible-infra/commit/d7538deae07ec05ffcf0443141b9b649c55bb675))

## [0.9.0](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.8.1...base-v0.9.0) (2026-11-20)


### Features

* **base:** status and reboot jobs for GitLab CI ([#37](https://github.com/ErrorCollector/ansible-infra/issues/37)) ([876dfb6](https://github.com/ErrorCollector/ansible-infra/commit/876dfb647759bc972eb33ea64e551ea2689ffc84))


### Bug Fixes

* let Renovate cover every pinned version ([#39](https://github.com/ErrorCollector/ansible-infra/issues/39)) ([e3f574e](https://github.com/ErrorCollector/ansible-infra/commit/e3f574e9016b2f2ea9b8ae7849184b8b148b2ad1))

## [0.8.1](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.8.0...base-v0.8.1) (2026-11-20)


### Bug Fixes

* **base:** one deploy environment per job ([#35](https://github.com/ErrorCollector/ansible-infra/issues/35)) ([161fb97](https://github.com/ErrorCollector/ansible-infra/commit/161fb97c48baf22b20de198f388138bbc8b57735))

## [0.8.0](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.7.0...base-v0.8.0) (2026-11-20)


### Features

* **gitlab:** deploy the GitLab server from its own CI job ([#33](https://github.com/ErrorCollector/ansible-infra/issues/33)) ([466be63](https://github.com/ErrorCollector/ansible-infra/commit/466be63fc7d391c81788bfbac7931a18ae82e42f))

## [0.7.0](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.6.0...base-v0.7.0) (2026-11-20)


### Features

* export the deployed collection version per host ([#31](https://github.com/ErrorCollector/ansible-infra/issues/31)) ([8484473](https://github.com/ErrorCollector/ansible-infra/commit/8484473f772d21f9b00d422dcafa8de5f20d44b9))

## [0.6.0](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.5.0...base-v0.6.0) (2026-11-20)


### Features

* **base:** only offer plan and deploy of domains a push changes ([#29](https://github.com/ErrorCollector/ansible-infra/issues/29)) ([95838c2](https://github.com/ErrorCollector/ansible-infra/commit/95838c2940721c1dc2815a51f450f5c39c49b2a2))

## [0.5.0](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.4.1...base-v0.5.0) (2026-11-20)


### Features

* **base:** init, common and node_exporter roles ([#21](https://github.com/ErrorCollector/ansible-infra/issues/21)) ([8914e11](https://github.com/ErrorCollector/ansible-infra/commit/8914e110bbd0b26a717132e925502f5b50b53114))


### Bug Fixes

* **base:** CI jobs name ansible.cfg explicitly ([#20](https://github.com/ErrorCollector/ansible-infra/issues/20)) ([edd61fa](https://github.com/ErrorCollector/ansible-infra/commit/edd61fa7e723ac850aa3cb6acf4f4c8d76912fe5))

## [0.4.1](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.4.0...base-v0.4.1) (2026-11-20)


### Bug Fixes

* **base:** CI jobs bootstrap the collection and accept keys without final newline ([#18](https://github.com/ErrorCollector/ansible-infra/issues/18)) ([c2f3a60](https://github.com/ErrorCollector/ansible-infra/commit/c2f3a60e940661a526024362d203c15571f652c4))

## [0.4.0](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.3.0...base-v0.4.0) (2026-11-20)


### Features

* **base:** GitLab CI template with plan and deploy jobs ([#16](https://github.com/ErrorCollector/ansible-infra/issues/16)) ([dd62e72](https://github.com/ErrorCollector/ansible-infra/commit/dd62e7278b02b2b407f382799bdd8e1f8b2d2208))

## [0.3.0](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.2.0...base-v0.3.0) (2026-11-20)


### Features

* **pangolin:** pangolin server, newt and pangolin_client roles ([#3](https://github.com/ErrorCollector/ansible-infra/issues/3)) ([3236a6c](https://github.com/ErrorCollector/ansible-infra/commit/3236a6c19a360abd008e7510f3535ea4916741d2))


### Bug Fixes

* **base:** make install also installs the Galaxy roles of requirements.d ([#9](https://github.com/ErrorCollector/ansible-infra/issues/9)) ([cb5e124](https://github.com/ErrorCollector/ansible-infra/commit/cb5e124d1bb226871a8ac858e23bb94caa2a9fcc))

## [0.2.0](https://github.com/ErrorCollector/ansible-infra/compare/base-v0.1.0...base-v0.2.0) (2026-11-20)


### Features

* **base:** shared Make targets and reboot playbook ([#1](https://github.com/ErrorCollector/ansible-infra/issues/1)) ([3173c03](https://github.com/ErrorCollector/ansible-infra/commit/3173c03272eb19504a5defe11b3bc93d235a77db))
