## Instruction to setup may world

I love ZSH with [oh-my-zsh](https://github.com/ohmyzsh/ohmyzsh) and TMUX with [oh-my-tmux](https://github.com/gpakosz/.tmux). By keeping me away from annoying interaction of bash and instability of the remote connection, they truly saved my life.
- ZSH [v5.9](https://sourceforge.net/projects/zsh/files/zsh/5.9/zsh-5.9.tar.xz/download)
- TMUX [v3.3](https://github.com/tmux/tmux/releases/tag/3.3a)

The instructions to install them to `$MYS_MODDIR` are presented in `libmys/ext`.

### Scenario 1: Local Machine
From my own Macbook Pro, no need for driver code. I wrote these lines **at the top** of `~/.zshrc`:
```bash
source /opt/homebrew/opt/lmod/init/profile
module unuse "/opt/homebrew/Cellar/lmod/8.7.19/modulefiles/Core"

export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
source $MYS_DIR/etc/profile

# ... other commands
```

Note that we source Lmod script first so that `$MODULEPATH` prepended in libmys won't be cleared. Then source `profile` before any other command to enable [Powerlevel10k instant prompt](https://github.com/romkatv/powerlevel10k#instant-prompt) as soon as possible.

### Scenario 2: Remote Machine with personal account
I have a personal account in the cluster of my lab. Then I wrote these lines **at the bottom** of `~/.bashrc` as driver codes:
```bash
[[ -f /etc/bashrc ]] && source /etc/bashrc
export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
[[ "$PS1" && -f "$MYS_MODDIR/BASE/bin/zsh" ]] && exec -la zsh "$MYS_MODDIR/BASE/bin/zsh"
```
and in `~/.zshrc`
```bash
export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
source $MYS_DIR/etc/profile

# ... other commands
```

### Scenario 3: Remote Machine with shared account
On several supercomputers, I have to use the shared account with my peers in the lab (We can't request another new account if someone joins us). So I use a private directory as my own `$HOME`, and wrote these lines in driver scirpt `$HOME/enter.sh`:

```bash
[[ -n "$TMUX" ]] && echo "ERROR: Already inside a tmux session ($TMUX)" && exit 1

_HOST="psn002" # If you want to enter may world only on a specific master machine
_CMD=""
_CMD+='export HOME=/SOMEWHERE/AS/MY/NEW/HOME;' # You may change this line
_CMD+='export MYS_DIR=$HOME/project/libmys;' # You may change this line
_CMD+='export MYS_MODDIR=$HOME/module;' # You may change this line
_CMD+='_TMUX=$MYS_MODDIR/BASE/bin/tmux;' # You may change this line

_CMD+='cd $HOME;'
_CMD+='_CONF=$MYS_DIR/etc/omt/tmux.conf;'
_CMD+='_SOCKET=mayeths-tmux-socket;'
_CMD+='_SESSION=main;'
_CMD+='$_TMUX -L $_SOCKET a || $_TMUX -f $_CONF -L $_SOCKET new -s $_SESSION'
#echo "$_CMD"
[[ -z "$_HOST" || "$_HOST" == $(hostname) ]] && eval "$_CMD" || ssh -t "$_HOST" "eval '$_CMD'"
```

and in `$HOME/.zshrc`:

```bash
export OLD_HOME=/THE/OLD/SHARED/HOME
# DO NOT source $OLD_HOME/bashrc because bash commands like shopt is undefined in zsh
# Use a trick to source it
ENVFILE=$(mktemp)
bash --init-file $OLD_HOME/.bashrc -c "export -p" > $ENVFILE
set -o allexport
emulate sh -c "source $ENVFILE"
set +o allexport
rm -f $ENVFILE
# Now try to source $OLD_HOME/.zshrc
[[ -f "$OLD_HOME/.zshrc" ]] && source "$OLD_HOME/.zshrc"

# For module, etc.
for i in /etc/profile.d/*.sh; do
    if [ -r "$i" ]; then
        if [ "$PS1" ]; then
            . "$i"
        else
            . "$i" >/dev/null
        fi
    fi
done

export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
source $MYS_DIR/etc/profile
# ... other commands
```

TMUX will automatically pick up the `$MYS_MODDIR/BASE/bin/zsh` as default command (in `$MYS_DIR/etc/omt/tmux.conf.custom`). So no need for worring about how to switch the shell.
