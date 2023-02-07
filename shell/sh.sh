###

export MYS_DIR=$(dirname $(dirname "$0")/../)
MYS_MODULE_CONFIG_DIR=$(dirname ~/module/CONFIG)

if $SHELL --version | grep -q bash; then
    source bashrc.sh
elif $SHELL --version | grep -q zsh; then
    source zshrc.sh
fi

export PYTHONDONTWRITEBYTECODE=1
if test $[HISTSIZE] -lt 50000; then
    export HISTSIZE=50000
fi
if test $[HISTFILESIZE] -lt 50000; then
    export HISTFILESIZE=50000
fi

env_prepend PATH "$MYS_DIR/bin"
env_prepend PYTHONPATH "$MYS_DIR/python"
env_prepend MODULEPATH "$MYS_MODULE_CONFIG_DIR"
unset MYS_MODULE_CONFIG_DIR

alias code="invoke-vscode"
