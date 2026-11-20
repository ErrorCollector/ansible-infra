# Managed by Ansible (errorcollector.base.common): show the full working directory (\w) in the prompt
if [ -n "$BASH_VERSION" ] && [[ $- == *i* ]]; then
    PS1='[\u@\h \w]\$ '
fi
