#!/usr/bin/env bash
# Fails if tracked files contain infrastructure details that must stay in the (private) project repos.
#
# - INFRA_DENYLIST: extended regex with domains/hostnames of the real infrastructure. Kept out of this public repo:
#   GitHub Actions secret INFRA_DENYLIST, locally e.g. in ~/.config/ansible-infra/denylist (one pattern per line).
# - IPv4 addresses, except loopback/unspecified and the documentation ranges (RFC 5737).
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

denylist="${INFRA_DENYLIST:-}"
local_file="${XDG_CONFIG_HOME:-$HOME/.config}/ansible-infra/denylist"
if [[ -z "$denylist" && -f "$local_file" ]]; then
    denylist="$(grep -v -e '^#' -e '^$' "$local_file" | paste -sd '|' -)"
fi

files=()
# Files deleted in the working tree are still listed by git ls-files
while IFS= read -r -d '' f; do [[ -f $f ]] && files+=("$f"); done < <(git ls-files -z --cached --others --exclude-standard -- ':!:*.png' ':!:scripts/check-denylist.sh')
[[ ${#files[@]} -gt 0 ]] || { echo "No files to check"; exit 0; }

# grep exits 1 for "no match" only; anything else (unreadable file, bad regex) must not pass the check
search() {
    local status=0
    grep -n "$@" "${files[@]}" || status=$?
    [[ $status -le 1 ]] || { echo "❌ grep failed ($status)" >&2; exit 2; }
}

rc=0

if [[ -n "$denylist" ]]; then
    # Plain assignment: set -e stops the script if search exits 2
    hits="$(search -i -E -- "$denylist")"
    if [[ -n $hits ]]; then
        echo "$hits"
        echo "❌ Infrastructure names found (INFRA_DENYLIST), move them to the project repos" >&2
        rc=1
    fi
else
    echo "⚠️  INFRA_DENYLIST not set, only checking IP addresses" >&2
fi

ip='\b([0-9]{1,3}\.){3}[0-9]{1,3}\b'
allowed=':(127\.0\.0\.1|0\.0\.0\.0|192\.0\.2\.[0-9]+|198\.51\.100\.[0-9]+|203\.0\.113\.[0-9]+)$'
# One line per address (file:line:address), so an allowed address does not hide another one in the same line
ip_hits="$(search -o -E -- "$ip")"
if grep -v -E -- "$allowed" <<<"$ip_hits" | grep -E -- "$ip"; then
    echo "❌ IP addresses found, use variables (documentation ranges 192.0.2.0/24, 198.51.100.0/24, 203.0.113.0/24 are fine)" >&2
    rc=1
fi

exit $rc
