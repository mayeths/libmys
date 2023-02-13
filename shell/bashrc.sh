###

### https://stackoverflow.com/a/24522107
# SYNOPSIS: env_contain varName fieldVal [sep]
#   SEP defaults to ':'
# EXAMPLE: env_contain PATH /usr/local/bin
env_contain() {
    local varName=$1 fieldVal=$2 IFS=${3:-':'} auxArr
    read -ra auxArr <<< "${!varName}"
    for i in "${!auxArr[@]}"; do
        [[ ${auxArr[i]} == "$fieldVal" ]] && return 0
    done
    return 1
}
# SYNOPSIS: env_prepend varName fieldVal [sep]
#   SEP defaults to ':'
# Note: Forces fieldVal into the first position, if already present.
#       Duplicates are removed, too.
# EXAMPLE: env_prepend PATH /usr/local/bin
env_prepend() {
    local varName=$1 fieldVal=$2 IFS=${3:-':'} auxArr
    read -ra auxArr <<< "${!varName}"
    for i in "${!auxArr[@]}"; do
        [[ ${auxArr[i]} == "$fieldVal" ]] && unset auxArr[i]
    done
    auxArr=("$fieldVal" "${auxArr[@]}")
    printf -v "$varName" '%s' "${auxArr[*]}"
}
# SYNOPSIS: env_append varName fieldVal [sep]
#   SEP defaults to ':'
# Note: Forces fieldVal into the last position, if already present.
#       Duplicates are removed, too.
# EXAMPLE: env_append PATH /usr/local/bin
env_append() {
    local varName=$1 fieldVal=$2 IFS=${3:-':'} auxArr
    read -ra auxArr <<< "${!varName}"
    for i in "${!auxArr[@]}"; do
        [[ ${auxArr[i]} == "$fieldVal" ]] && unset auxArr[i]
    done
    auxArr+=("$fieldVal")
    printf -v "$varName" '%s' "${auxArr[*]}"
}
# SYNOPSIS: env_cut varName fieldVal [sep]
#   SEP defaults to ':'
# Note: Duplicates are removed, too.
# EXAMPLE: env_cut PATH /usr/local/bin
env_cut() {
    local varName=$1 fieldVal=$2 IFS=${3:-':'} auxArr
    read -ra auxArr <<< "${!varName}"
    for i in "${!auxArr[@]}"; do
        [[ ${auxArr[i]} == "$fieldVal" ]] && unset auxArr[i]
    done
    printf -v "$varName" '%s' "${auxArr[*]}"
}


# SYNOPSIS: _colorful_short_path letter_kept width_percent [MAIN_COLOR]
#   MAIN_COLOR defaults to green '\\[\\e[32m\\]'
# @see https://askubuntu.com/a/1355437
_colorful_short_path() {
    local letter_kept=$1
    local width_percent=$2
    local MAIN_COLOR=${3:-'\\[\\e[32m\\]'}
    local NORMAL='\\[\\e[0m\\]'
    local BOLD='\\[\\e[1m\\]'
    local DIM='\\[\\e[2m\\]'

    local current_path=${PWD/#$HOME/'~'}
    if [[ "$current_path" == "~" || "$current_path" == "/" ]]; then
        echo -e "${MAIN_COLOR}${current_path}${NORMAL}"
    else
        local terminal_width=$(tput cols)
        local max_width=$(($terminal_width * $width_percent / 100))
        local curr_width=${#current_path}

        local dirs=()
        local count=0
        local path="$current_path"
        while :
        do
            local dname="${path%\/*}"
            local bname="${path##*\/}"
            dirs[$count]="$bname"
            let count++
            if [[ "$dname" == "$bname" || -z "$bname" ]]; then
                break
            fi
            path="$dname"
        done

        local width=$curr_width
        local i=${#dirs[@]}-1
        while [[ i -ge 1 && $width -ge $max_width ]]; do
            local dir=${dirs[$i]}
            local dir_short=""
            local dir_short="${dir::$letter_kept}"
            local dir_width=${#dir}
            local cut_width=$(($dir_width-$letter_kept))
            if [[ "$dir" == "~" ]]; then
                dirs[$i]="${MAIN_COLOR}${dir_short}${NORMAL}"
            else
                dirs[$i]="${DIM}${dir_short}${NORMAL}"
            fi
            width=$(($width-$cut_width))
            let i--
        done
        while [[ i -ge 1 ]]; do
            local dir=${dirs[$i]}
            dirs[$i]="${MAIN_COLOR}${dir}${NORMAL}"
            let i--
        done
        dirs[0]="${MAIN_COLOR}${dirs[0]}${NORMAL}"

        local result="${dirs[count-1]}"
        for ((i = ${#dirs[@]}-2; i >= 0; i--)); do
            local dir=${dirs[$i]}
            result="${result}${MAIN_COLOR}/${NORMAL}${dir}"
        done
        echo -e "$result"
    fi
}

_colorful_driver() {
    local exit_code=$1
    local RED="\[\e[31m\]"
    local GREEN="\[\e[32m\]"
    local NORMAL="\[\e[0m\]"
    local BASH_SIGINT_CODE=130

    local symbol=""
    if [[ $exit_code -ne 0 && $exit_code -ne $BASH_SIGINT_CODE ]]; then
        symbol="${RED}x${NORMAL}"
    fi
    PS1="${symbol}${GREEN}[\u@\h:$(eval _colorful_short_path 2 40)${GREEN}]\$ ${NORMAL}"
    return $exit_code
}

_flush_history() {
    local exit_code=$1
    history -a
    return $exit_code
}

# Use single quote to escape \$
# Write to .bash_history immediately
PROMPT_COMMAND='_RET=$? ; _colorful_driver $_RET; _flush_history $_RET'
