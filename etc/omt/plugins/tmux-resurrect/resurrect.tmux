#!/usr/bin/env bash

CURRENT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$CURRENT_DIR/scripts/variables.sh"
source "$CURRENT_DIR/scripts/helpers.sh"

set_save_bindings() {
	local key_bindings=$(get_tmux_option "$save_option" "$default_save_key")
	local key
	for key in $key_bindings; do
		$TMUX_PROGRAM bind-key "$key" run-shell "$CURRENT_DIR/scripts/save.sh"
	done
}

set_restore_bindings() {
	local key_bindings=$(get_tmux_option "$restore_option" "$default_restore_key")
	local key
	for key in $key_bindings; do
		$TMUX_PROGRAM bind-key "$key" run-shell "$CURRENT_DIR/scripts/restore.sh"
	done
}

set_default_strategies() {
	$TMUX_PROGRAM set-option -gq "${restore_process_strategy_option}irb" "default_strategy"
	$TMUX_PROGRAM set-option -gq "${restore_process_strategy_option}mosh-client" "default_strategy"
}

set_script_path_options() {
	$TMUX_PROGRAM set-option -gq "$save_path_option" "$CURRENT_DIR/scripts/save.sh"
	$TMUX_PROGRAM set-option -gq "$restore_path_option" "$CURRENT_DIR/scripts/restore.sh"
}

set_auto_run() {
	local auto_run=$(get_tmux_option "$auto_run_option" "$auto_run_default")
	if [[ "$auto_run" == "on" ]]; then
		local interval=$(get_tmux_option "$auto_run_interval_option" "$auto_run_interval_default")
		$TMUX_PROGRAM run-shell -b "$CURRENT_DIR/scripts/auto.sh $interval"
	fi
}

main() {
	set_save_bindings
	set_restore_bindings
	set_default_strategies
	set_script_path_options
	set_auto_run
}
main
