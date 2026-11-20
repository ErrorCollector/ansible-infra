#!/bin/bash
# Managed by Ansible (errorcollector.base.common): OS update metrics for node_exporter's textfile collector.
# Usage: <brand>-update-status <textfile dir> (installed by errorcollector.base.common)
# dnf errors go to stderr, i.e. into the journal: journalctl -u <brand>-update-status.service
set -u
dir="${1:-/var/lib/node_exporter/textfile}"
ok_restart=1
ok_updates=1

dnf -q needs-restarting -r >/dev/null
rc=$?
case $rc in
    0) reboot=0 ;;
    1) reboot=1 ;;
    *) reboot=0; ok_restart=0; echo "dnf needs-restarting -r failed (rc=$rc)" >&2 ;;
esac

# Cached metadata only (refreshed by dnf-automatic / dnf-makecache); rc 100 = updates available.
# If the cache is unusable (e.g. after a *-release/*-repos package update), retry once
# with a regular refresh instead of waiting up to 3h for dnf-makecache.timer.
out="$(dnf -q --cacheonly check-update)"
rc=$?
if [ "$rc" -ne 0 ] && [ "$rc" -ne 100 ]; then
    echo "dnf --cacheonly check-update failed (rc=$rc), retrying with metadata refresh" >&2
    out="$(dnf -q check-update)"
    rc=$?
fi
if [ "$rc" -eq 0 ] || [ "$rc" -eq 100 ]; then
    pending=$(printf '%s\n' "$out" | grep -cE '^[^[:space:]]+\.[[:alnum:]_]+[[:space:]]+[^[:space:]]+[[:space:]]+[^[:space:]]+')
else
    pending=0; ok_updates=0
    echo "dnf check-update failed (rc=$rc)" >&2
fi

ok=$(( ok_restart && ok_updates ))

tmp="$(mktemp "$dir/.infra_updates.prom.XXXXXX")" || exit 1
cat > "$tmp" <<METRICS
# HELP infra_reboot_required 1 if dnf needs-restarting -r reports that a reboot is required.
# TYPE infra_reboot_required gauge
infra_reboot_required $reboot
# HELP infra_os_updates_pending Package updates available (cached dnf metadata).
# TYPE infra_os_updates_pending gauge
infra_os_updates_pending $pending
# HELP infra_update_status_success 1 if all dnf queries succeeded.
# TYPE infra_update_status_success gauge
infra_update_status_success $ok
# HELP infra_update_status_query_success 1 if the given dnf query succeeded.
# TYPE infra_update_status_query_success gauge
infra_update_status_query_success{query="needs_restarting"} $ok_restart
infra_update_status_query_success{query="check_update"} $ok_updates
# HELP infra_update_status_timestamp_seconds Last time these metrics were written.
# TYPE infra_update_status_timestamp_seconds gauge
infra_update_status_timestamp_seconds $(date +%s)
METRICS
chmod 0644 "$tmp"
mv -f "$tmp" "$dir/infra_updates.prom"
