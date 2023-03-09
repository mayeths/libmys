## Instruction to setup may world

I love ZSH with [oh-my-zsh](https://github.com/ohmyzsh/ohmyzsh) and TMUX with [oh-my-tmux](https://github.com/gpakosz/.tmux). By keeping me away from annoying interaction of bash and instability of the remote connection, they truly saved my life.
- ZSH [v5.9](https://sourceforge.net/projects/zsh/files/zsh/5.9/zsh-5.9.tar.xz/download)
- TMUX [v3.3](https://github.com/tmux/tmux/releases/tag/3.3a)

### Scenario 1: Local Machine
From my own Macbook Pro, no need for driver code. I wrote these lines **at the top** of `~/.zshrc`:
```bash
source /opt/homebrew/opt/lmod/init/profile
module unuse "/opt/homebrew/Cellar/lmod/8.7.19/modulefiles/Core"

export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
source $MYS_DIR/etc/profile
```

Note that we source Lmod script first so that `$MODULEPATH` prepended in libmys won't be cleared. Sourcing profile in libmys should be the first thing before any to enable [Powerlevel10k instant prompt](https://github.com/romkatv/powerlevel10k#instant-prompt) as soon as possible.

### Scenario 2: Remote Machine with personal account
I have a personal account in the cluster of my lab. Then I wrote these lines **at the bottom** of `~/.bashrc` as driver:
```bash
export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
[[ "$PS1" && -f "$MYS_MODDIR/BASE/bin/zsh" ]] && exec -la zsh "$MYS_MODDIR/BASE/bin/zsh"
```
and in `~/.zshrc`
```bash
export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
source $MYS_DIR/etc/profile
```

### Scenario 3: Remote Machine with shared account
On several supercomputers, I have to use the shared account with my peers in the lab (We can't request another new account if someone joins us). So I use a private directory as my own `$HOME`, and wrote these lines to driver `$HOME/enter.sh`:

```bash
export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
export HOME="/some/dir/has/.zshrc/sourcing/MYS_DIR/etc/profile"
_TMUX="$MYS_MODDIR/BASE/bin/tmux"
_CONF="$MYS_DIR/etc/omt/tmux.conf"
_SOCKET="mayeths-tmux-socket"
_SESSION="main"
_CMD="$_TMUX -L $_SOCKET a || $_TMUX -f $_CONF -L $_SOCKET new -s $_SESSION"
# _HOST="psn002" # if you want to enter mys world on fixed jumper machine
echo "$_CMD"
[[ -z "$_HOST" ]] && $_CMD || ssh -t "$_HOST" "$_CMD"
```

and in `$HOME/.zshrc`:

```bash
export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
source $MYS_DIR/etc/profile
```
