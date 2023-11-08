# Refer to https://github.com/ohmyzsh/ohmyzsh/blob/master/plugins/pyenv/pyenv.plugin.zsh
# This plugin provides prompt info via the 'compiler_env_prompt_info' function.

# Look for compiler and mpi in $PATH

function get_version_XYZ() {
    if $(grep --version | grep -q "GNU grep"); then # GNU grep
        echo $($1 --version 2>/dev/null | grep -oP '\d*\.\d*\.\d*' | head -1)
    else # BSD grep
        echo $($1 --version 2>/dev/null | grep -oe '\d*\.\d*\.\d*' | head -1)
    fi
}

function compiler_env_prompt_info() {
    local found_gcc found_icc found_clang found_mpi
    command -v gcc &>/dev/null && found_gcc=1 || found_gcc=0
    command -v icc &>/dev/null && found_icc=1 || found_icc=0
    command -v clang &>/dev/null && found_clang=1 || found_clang=0
    command -v mpirun &>/dev/null && found_mpi=1 || found_mpi=0

    local array=()

    # FIXME: What if intel mpi
    # if [[ $found_mpi -eq 1 ]]; then
    #     if [[ $(mpiexec --version 2>/dev/null) == *"OpenRTE"* ]]; then
    #         # is openmpi
    #         local ompi_ver=$(get_version_XYZ mpirun)
    #         array+=("openmpi-$ompi_ver")
    #     else
    #         # is mpich
    #         local mpich_ver=$(mpichversion | grep -e "Version" | awk '{print $NF}')
    #         array+=("mpich-$mpich_ver")
    #     fi
    # fi

    if [[ $found_gcc -eq 1 && $(gcc --version 2>/dev/null) == *"gcc"* ]]; then
        local gcc_ver=$(get_version_XYZ gcc)
        array+=("gcc-$gcc_ver")
    elif [[ $found_clang -eq 1 ]]; then
        local clang_ver=$(get_version_XYZ clang)
        array+=("clang-$clang_ver")
    fi

    if [[ $found_icc -eq 1 ]]; then
        local icc_ver=$(get_version_XYZ icc)
        array+=("icc-$icc_ver")
    fi

    echo "${(j: :)array}"
}

# type 'p10k segment help' to see all example or search prompt_example in config/p10k.sh
function prompt_compiler_env() {
    local text=$(compiler_env_prompt_info)
    p10k segment -f 208 -t "$text"
}

# compiler_env_prompt_info
