#!/usr/bin/env bash
# Compares the errorcollector collections pinned in requirements.d/<domain>.yml
# with the ones installed in collections/ (run from the project repo).
#   collections.sh          table of all domains
#   collections.sh --check  only problems; exit 1 if a collection is missing or outdated
# A collection linked by `make dev` is reported but does not fail the check.
set -euo pipefail

dir=collections/ansible_collections/errorcollector
check=false
[[ ${1:-} == --check ]] && check=true

rc=0
rows=()
for file in requirements.d/*.yml; do
    domain=$(basename "$file" .yml)
    pinned=$(sed -nE "s/^[[:space:]]*version:[[:space:]]*[\"']?${domain}-v([^\"'[:space:]]+).*/\1/p" "$file")
    [[ -n $pinned ]] || continue

    path=$dir/$domain
    if [[ -L $path ]]; then
        installed="dev: $(readlink "$path")"
        state=DEV
    elif [[ -f $path/MANIFEST.json ]]; then
        installed=$(grep -oE '"version": *"[^"]+"' "$path/MANIFEST.json" | head -1 | sed -E 's/.*"([^"]+)"$/\1/')
        if [[ $installed == "$pinned" ]]; then state=ok; else state=OUTDATED; rc=1; fi
    else
        installed=-
        state=MISSING
        rc=1
    fi

    if $check; then
        case $state in
            DEV) echo "⚠️  errorcollector.$domain is linked to $(readlink "$path") (back to $pinned: make install)" ;;
            OUTDATED | MISSING) echo "❌ errorcollector.$domain: pinned $pinned, installed $installed → make install" ;;
        esac
    else
        rows+=("errorcollector.$domain;$pinned;$installed;$state")
    fi
done

if ! $check; then
    (echo "COLLECTION;PINNED;INSTALLED;STATE"; printf '%s\n' ${rows[@]+"${rows[@]}"}) | column -t -s ';'
fi
exit $rc
