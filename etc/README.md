### Instruction to setup may world

I love using zsh ([v5.9](https://sourceforge.net/projects/zsh/files/zsh/5.9/zsh-5.9.tar.xz/download)) with [oh-my-zsh](https://github.com/ohmyzsh/ohmyzsh), and tmux ([v3.3](https://github.com/tmux/tmux/releases/tag/3.3a)) with [oh-my-tmux](https://github.com/gpakosz/.tmux). They really save my life from bad bash interaction and bad remote connection. The configuration files are under `etc/` folder.

```bash
# Load Lmod first, then source "$MYS_DIR/etc/profile" at the top of ~/.bashrc or ~/.zshrc.
export MYS_DIR=...
export MYS_MODDIR=...

# Enter may world directly
THE_ZSH="$MYS_MODDIR/BASE/bin/zsh"
exec -la zsh "$THE_ZSH"

# or within a shared account on cluster,
# run the following commands in a secret script
# to enter may world with tmux
HOME="/some/dir/has/.zshrc/sourcing/MYS_DIR/etc/profile"
THE_TMUX="$MYS_MODDIR/BASE/bin/tmux"
THE_CONF="$MYS_DIR/etc/omt/tmux.conf"
THE_SOCKET="mayeths-tmux-socket"
THE_SESSION="main"
THE_CMD="$THE_TMUX -L $THE_SOCKET a || $THE_TMUX -f $THE_CONF -L $THE_SOCKET new -s $THE_SESSION"
THE_HOST="..." # or empty if you want to enter mys world on current jumper machine
echo "$THE_CMD"
if [[ -z "$THE_HOST" ]]; then
    $THE_CMD
else
    ssh -t "$THE_HOST" "$THE_CMD"
fi
```

```bash
zsh compilation options from zsh-autosuggestions
#   ./configure --enable-pcre \
#               --enable-cap \
#               --enable-multibyte \
#               --with-term-lib='ncursesw tinfo' \
#               --with-tcsetpgrp \
#               --program-suffix="-$v"
```
