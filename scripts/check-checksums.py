#!/usr/bin/env python3
"""Compare the pinned SHA256 checksums of downloaded binaries with the upstream release.

Renovate updates the versions in the role defaults but not the checksums next to them; this check makes such an
update fail in CI until the checksums match. It prints the expected values. GITHUB_TOKEN (optional) avoids the
API rate limit.
"""
import json
import os
import sys
import urllib.request

import yaml

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def fetch(url, api=False):
    headers = {"User-Agent": "ansible-infra-check-checksums"}
    if api and os.environ.get("GITHUB_TOKEN"):
        headers["Authorization"] = "Bearer " + os.environ["GITHUB_TOKEN"]
    with urllib.request.urlopen(urllib.request.Request(url, headers=headers), timeout=30) as response:
        return response.read().decode()


def defaults(path):
    with open(os.path.join(ROOT, path)) as file:
        return yaml.safe_load(file)


def zoxide():
    values = defaults("collections/base/roles/common/defaults/main.yml")
    version = values["common_zoxide_version"]
    release = json.loads(fetch(f"https://api.github.com/repos/ajeetdsouza/zoxide/releases/tags/v{version}", api=True))
    digests = {asset["name"]: asset.get("digest") for asset in release["assets"]}
    for arch, pinned in values["common_zoxide_checksums"].items():
        yield "common_zoxide_checksums", arch, pinned, digests.get(f"zoxide-{version}-{arch}-unknown-linux-musl.tar.gz")


def ecr_helper():
    values = defaults("collections/gitlab/roles/gitlab_runner/defaults/main.yml")
    version = values["gitlab_runner_ecr_helper_version"]
    for arch, pinned in values["gitlab_runner_ecr_helper_checksums"].items():
        go_arch = values["gitlab_runner_ecr_helper_arch"][arch]
        url = (f"https://amazon-ecr-credential-helper-releases.s3.us-east-2.amazonaws.com/{version}"
               f"/linux-{go_arch}/docker-credential-ecr-login.sha256")
        yield "gitlab_runner_ecr_helper_checksums", arch, pinned, "sha256:" + fetch(url).split()[0]


def main():
    failed = False
    for check in (zoxide, ecr_helper):
        for variable, arch, pinned, upstream in check():
            if pinned == upstream:
                print(f"ok    {variable}.{arch}")
            else:
                failed = True
                print(f"FAIL  {variable}.{arch}: pinned {pinned}, upstream {upstream}")
    if failed:
        print("Update the checksums next to the version in the role defaults.")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
