# LibMYS

Mayeths' library for daily development (wow@mayeths.com).

```bash
# Clone libmys
git clone https://github.com/mayeths/libmys.git
# Enjoy libmys
export MYS_DIR=~/project/libmys
source "$MYS_DIR/etc/rc"
# Rsync libmys
cd libmys
rsync --filter=':- .gitignore' -avz ./ REMOTE:~/project/libmys/
```
