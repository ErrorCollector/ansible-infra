# Managed by Ansible (errorcollector.base.common): provides `z` / `zi` via zoxide
if [ -n "$BASH_VERSION" ] && [[ $- == *i* ]] && [ -x /usr/bin/zoxide ]; then
    eval "$(/usr/bin/zoxide init bash)"
fi
