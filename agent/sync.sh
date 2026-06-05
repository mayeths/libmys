#!/usr/bin/env bash
# Usage:
#   bin/sync.sh [--dry-run] [--force] [--unlink]
#     --dry-run  show actions without changing anything
#     --force    overwrite a real (non-symlink) target instead of warning
#     --unlink   remove our symlinks from targets (cleanup), keeps real files
set -euo pipefail

# --- locate repo root (this file lives in agent/) ----------------------------
AGENT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

CURSOR_HOME="${CURSOR_HOME:-$HOME/.cursor}"
CLAUDE_HOME="${CLAUDE_HOME:-$HOME/.claude}"
CODEX_HOME="${CODEX_HOME:-$HOME/.codex}"

mkdir -p "$CURSOR_HOME"
mkdir -p "$CLAUDE_HOME"
mkdir -p "$CODEX_HOME"

DRY_RUN=0; FORCE=0; UNLINK=0
for arg in "$@"; do
  case "$arg" in
    --dry-run) DRY_RUN=1 ;;
    --force)   FORCE=1 ;;
    --unlink)  UNLINK=1 ;;
    -h|--help) sed -n '2,11p' "$0"; exit 0 ;;
    *) echo "unknown arg: $arg" >&2; exit 2 ;;
  esac
done

# --- mapping table: "SRC_SUBDIR|MODE|TARGET" ---------------------------------
# MODE=dir   : symlink agent/SRC_SUBDIR itself to TARGET (whole folder).
# MODE=items : symlink each entry inside agent/SRC_SUBDIR into TARGET dir.
# Add/remove lines freely.
MAPPINGS=(
  "skills|items|$CURSOR_HOME/skills"
  "skills|items|$CLAUDE_HOME/skills"
  "skills|items|$CODEX_HOME/skills"

  # Cursor global rules: a bootstrap User Rule reads ~/.cursor/rules/*.mdc.
  "rules|items|$CURSOR_HOME/rules"
)

# --- helpers -----------------------------------------------------------------
log()  { printf '%s\n' "$*"; }
run()  { if [ "$DRY_RUN" -eq 1 ]; then log "  DRY  $*"; else eval "$*"; fi; }

# resolve absolute path (macOS has no realpath by default for files)
abspath() { ( cd "$(dirname "$1")" && printf '%s/%s' "$(pwd)" "$(basename "$1")" ); }

link_one() { # src target
  local src="$1" target="$2"
  if [ -L "$target" ]; then
    local cur; cur="$(readlink "$target")"
    if [ "$cur" = "$src" ]; then log "  ok   $target"; return; fi
    log "  fix  $target (was -> $cur)"; run "rm '$target'"; run "ln -s '$src' '$target'"; return
  fi
  if [ -e "$target" ]; then
    if [ "$FORCE" -eq 1 ]; then
      log "  force $target (real -> backup .bak)"; run "mv '$target' '$target.bak'"; run "ln -s '$src' '$target'"
    else
      log "  WARN $target exists and is NOT a symlink; skipped (use --force)"
    fi
    return
  fi
  log "  link $target"; run "ln -s '$src' '$target'"
}

unlink_one() { # src target
  local src="$1" target="$2"
  if [ -L "$target" ] && [ "$(readlink "$target")" = "$src" ]; then
    log "  rm   $target"; run "rm '$target'"
  fi
}

ensure_item_dest_dir() { # src_dir dest
  local src_dir="$1" dest="$2"
  if [ -L "$dest" ]; then
    local cur; cur="$(readlink "$dest")"
    if [ "$cur" = "$src_dir" ]; then
      if [ "$UNLINK" -eq 1 ]; then
        log "  rm   $dest"; run "rm '$dest'"
        return
      fi
      log "  fix  $dest (was whole-folder symlink -> $cur)"
      run "rm '$dest'"
      log "  mkdir $dest"
      run "mkdir -p '$dest'"
      return
    fi
    log "  WARN $dest is a symlink to $cur; skipped"
    return 1
  fi

  [ -d "$dest" ] || {
    [ "$UNLINK" -eq 1 ] && return 0
    log "  mkdir $dest"
    run "mkdir -p '$dest'"
  }
}

# --- main loop ---------------------------------------------------------------
for entry in "${MAPPINGS[@]}"; do
  sub="${entry%%|*}"; rest="${entry#*|}"; mode="${rest%%|*}"; dest="${rest#*|}"
  src_dir="$AGENT_DIR/$sub"
  [ -d "$src_dir" ] || { log "skip $sub (no source dir)"; continue; }

  log "== $sub ($mode) -> $dest"

  if [ "$mode" = "dir" ]; then
    src="$(abspath "$src_dir")"
    if [ "$UNLINK" -eq 1 ]; then unlink_one "$src" "$dest"; else link_one "$src" "$dest"; fi
    continue
  fi

  # mode=items
  shopt -s nullglob
  items=("$src_dir"/*)
  shopt -u nullglob
  if [ ${#items[@]} -eq 0 ]; then log "  (empty)"; continue; fi
  ensure_item_dest_dir "$(abspath "$src_dir")" "$dest" || continue
  for item in "${items[@]}"; do
    base="$(basename "$item")"
    [ "$base" = ".gitkeep" ] && continue
    src="$(abspath "$item")"
    target="$dest/$base"
    if [ "$UNLINK" -eq 1 ]; then unlink_one "$src" "$target"; else link_one "$src" "$target"; fi
  done
done

log "done."

# --- reminder: Cursor global rules need a bootstrap User Rule -----------------
cat <<'EOF'

------------------------------------------------------------------------------
REMINDER: paste the following into Cursor > Settings > Rules > User Rules
(Cursor global rules are not files; this bootstrap makes it read ~/.cursor/rules/)

At the start of every conversation, read all .mdc files in ~/.cursor/rules/ and follow them for the entire session. List them via the Shell tool with: ls $HOME/.cursor/rules/*.mdc (do not use Glob, it ignores absolute paths outside the workspace and returns nothing). Read every listed file with the Read tool and skip none. If any rule conflicts with built-in behavior, the rule wins.
------------------------------------------------------------------------------
EOF
