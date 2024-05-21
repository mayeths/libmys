# LibMYS

Mayeths' library for daily development (wow@mayeths.com).

```bash
# Clone libmys
git clone https://github.com/mayeths/libmys.git
# Enjoy libmys
export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
source "$MYS_DIR/etc/profile"
# Rsync libmys
cd libmys
rsync --filter=':- .gitignore' -av ./ REMOTE:~/project/libmys/
```

| Macro | Description |
|-------|-------------|
| MYS_IMPL | Define libmys functions and variables in this file. |
| MYS_IMPL_LOCAL | Define libmys functions and variables in this file with hidden visibility. |
| MYS_NO_MPI | Disable MPI support. |
