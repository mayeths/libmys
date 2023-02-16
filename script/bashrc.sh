###

source bash-preexec.sh
source bash-functions.sh

_TIME_IT_THRESHOLD=5
_append_preexec_functions_once _remember_last_command
_append_preexec_functions_once _time_it_begin
_append_precmd_functions_once _time_it_end
_append_precmd_functions_once _git_branch_name
_append_precmd_functions_once _flush_history
_append_precmd_functions_once _customized_prompt
