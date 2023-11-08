#-------------------------------------------
# oh-my-zsh functionality configurations
#-------------------------------------------
# Enable Powerlevel10k instant prompt. Should stay close to the top of ~/.zshrc.
if [[ -r "${XDG_CACHE_HOME:-$HOME/.cache}/p10k-instant-prompt-${(%):-%n}.zsh" ]]; then
  source "${XDG_CACHE_HOME:-$HOME/.cache}/p10k-instant-prompt-${(%):-%n}.zsh"
fi

export ZSH="$MYS_DIR/etc/omz"
ZSH_THEME="powerlevel10k/powerlevel10k"
ZSH_COMPDUMP="${XDG_CACHE_HOME:-$HOME/.cache}/zcompdump-$HOST"

zstyle ':omz:update' mode disabled
GITSTATUS_CACHE_DIR=$MYS_MODDIR/BASE/bin
DISABLE_UNTRACKED_FILES_DIRTY="true"
CASE_SENSITIVE="true"
# ENABLE_CORRECTION="true"
# DISABLE_MAGIC_FUNCTIONS="true"
# DISABLE_LS_COLORS="true"
# DISABLE_AUTO_TITLE="true"
# HIST_STAMPS="mm/dd/yyyy"

plugins=(git zsh-autosuggestions zsh-tab-title compiler-env)
source $ZSH/oh-my-zsh.sh

#-------------------------------------------
# oh-my-zsh plugins configurations
#-------------------------------------------
for conf in $ZSH/config/*.sh ; do
    if [ -r "$conf" ]; then
        if [ "${-#*conf}" != "$-" ]; then
            . "$conf"
        else
            . "$conf" >/dev/null
        fi
    fi
done
