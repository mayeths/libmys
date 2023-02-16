###

if $SHELL --version | grep -q bash; then
    source bashrc.sh
elif $SHELL --version | grep -q zsh; then
    source zshrc.sh
fi

command -v realpath >/dev/null 2>&1 || realpath() {
    echo "$(cd "$(dirname -- "$1")" >/dev/null; pwd -P)/$(basename -- "$1")"
}

export MYS_DIR=$(dirname $(dirname "$0")/..)
export PYTHONDONTWRITEBYTECODE=1
export MYS_TMUX_CONF=$(realpath $MYS_DIR/tmux/tmux.conf)
export MYS_TMUX_CONF_LOCAL=$(realpath $MYS_DIR/tmux/tmux.conf.local)

if test $[HISTSIZE] -lt 50000; then
    export HISTSIZE=50000
fi

if test $[HISTFILESIZE] -lt 50000; then
    export HISTFILESIZE=50000
fi

env_prepend PATH "$MYS_DIR/bin"
env_prepend PYTHONPATH "$MYS_DIR/python"
#env_prepend CPATH "$MYS_DIR/include"


alias code="$MYS_DIR/bin/invoke-vscode"
