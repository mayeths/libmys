
mys_backup_command() {
    local s
    $SHELL --version | grep -q bash && s=bash
    $SHELL --version | grep -q zsh && s=zsh
    local t
    [[ $s == "bash" ]] && t=$(type -t $1)
    [[ $s == "zsh"  ]] && t=$(whence -w $1 | awk '{ print $NF }')
    if [[ $t == "function" ]]; then
        eval "$2() $(declare -f $1 | sed 1d)"
    elif [[ $t == "builtin" || $t == "file" || $t == "command" ]]; then
        eval "$2() { command $1 \"\$@\" }"
    elif [[ $t == "alias" ]]; then
        local p
        [[ "$s" == "bash" ]] && p="$2"
        [[ "$s" == "zsh" ]] && p="alias $2"
        eval $(alias $1 | awk "NR==1,/$1/{sub(/$1/, \"$p\")} 1")
    else # "keyword" or "reserved"
        return 1
    fi
}

mys_load_script() {
    local path
    for path in "$@"; do
        [[ -f "$path" ]] && source $path
    done
}

mys_realpath() {
    echo "$(cd "$(dirname -- "$1")" >/dev/null; pwd -P)/$(basename -- "$1")"
}

# Borrow from lmod/init/profile
mys_find_exec () {
  local Nm=$1
  local confPath=$2
  local execNm=$3
  eval $Nm=$confPath
  if [ ! -x $confPath ]; then
    if [ -x /bin/$execNm ]; then
       eval $Nm=/bin/$execNm
    elif [ -x /usr/bin/$execNm ]; then
       eval $Nm=/usr/bin/$execNm
    fi
  fi
}

# Borrow from lmod/init/profile
mys_find_shell() {
    local READLINK_CMD PS_CMD EXPR_CMD BASENAME_CMD my_shell
    mys_find_exec READLINK_CMD /usr/bin/readlink  readlink
    mys_find_exec PS_CMD       /bin/ps        ps
    mys_find_exec EXPR_CMD     /bin/expr      expr
    mys_find_exec BASENAME_CMD /usr/bin/basename  basename

    if [ -f /proc/$$/exe ]; then
        my_shell=$($READLINK_CMD /proc/$$/exe)
    else
        my_shell=$($PS_CMD -p $$ -ocomm=)
    fi
    my_shell=$($EXPR_CMD    "$my_shell" : '-*\(.*\)')
    my_shell=$($BASENAME_CMD $my_shell)

    case ${my_shell} in
        bash|zsh|sh) ;;
        ksh*) my_shell="ksh";;
        *) my_shell="sh";;
    esac
    echo $my_shell
}
