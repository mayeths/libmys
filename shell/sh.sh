###

export MYS_DIR=$(dirname $(dirname "$0")/../)
export PYTHONDONTWRITEBYTECODE=1

if test $[HISTSIZE] -lt 50000; then
    export HISTSIZE=50000
fi

if test $[HISTFILESIZE] -lt 50000; then
    export HISTFILESIZE=50000
fi

if $SHELL --version | grep -q bash; then
    source bashrc.sh
elif $SHELL --version | grep -q zsh; then
    source zshrc.sh
fi

env_prepend PATH "$MYS_DIR/bin"
env_prepend PYTHONPATH "$MYS_DIR/python"
#env_prepend CPATH "$MYS_DIR/include"


alias code="$MYS_DIR/bin/invoke-vscode"
