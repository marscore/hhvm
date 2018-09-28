(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


type client_mode =
| MODE_LIST_FILES
| MODE_LIST_MODES
| MODE_TYPE_AT_POS of string
| MODE_TYPE_AT_POS_BATCH of string list
| MODE_TYPED_FULL_FIDELITY_PARSE of string (* filename *)
| MODE_AUTO_COMPLETE
| MODE_IN_MEMORY_DEP_TABLE_SIZE
| MODE_SAVE_STATE of string
| MODE_STATUS
| MODE_STATUS_SINGLE of string (* filename *)
| MODE_SHOW of string
| MODE_COLORING of string
| MODE_COVERAGE of string
| MODE_FIND_REFS of string
| MODE_IDENTIFY_SYMBOL1 of string
| MODE_IDENTIFY_SYMBOL2 of string
| MODE_IDENTIFY_SYMBOL3 of string
| MODE_OUTLINE
| MODE_OUTLINE2
| MODE_METHOD_JUMP_CHILDREN of string
| MODE_METHOD_JUMP_ANCESTORS of string * string
| MODE_METHOD_JUMP_ANCESTORS_BATCH of string list * string
| MODE_REFACTOR of string * string * string
| MODE_IDE_REFACTOR of string
| MODE_FIND_CLASS_REFS of string
(* TODO figure out why we can't reference FuzzySearchService from here *)
| MODE_SEARCH of string * string
| MODE_LINT of string list
| MODE_LINT_STDIN of string
| MODE_LINT_ALL of int
| MODE_DUMP_SYMBOL_INFO of string
| MODE_CREATE_CHECKPOINT of string
| MODE_RETRIEVE_CHECKPOINT of string
| MODE_DELETE_CHECKPOINT of string
| MODE_STATS
| MODE_FORMAT of int * int
| MODE_REMOVE_DEAD_FIXMES of int list
| MODE_IDE_FIND_REFS of string
| MODE_IDE_HIGHLIGHT_REFS of string
| MODE_AI_QUERY of string
| MODE_FULL_FIDELITY_PARSE of string
| MODE_FULL_FIDELITY_SCHEMA
| MODE_INFER_RETURN_TYPE of string
| MODE_CST_SEARCH of string list option

type client_check_env = {
  mode: client_mode;
  root: Path.t;
  from: string;
  output_json: bool;
  retries: int;
  timeout: float option;
  autostart: bool;
  force_dormant_start: bool;
  watchman_debug_logging: bool;
  no_load: bool;
  profile_log: bool;
  ai_mode: string option;
  ignore_hh_version: bool;
  dynamic_view: bool;
  sort_results: bool;
  file_info_on_disk: bool;
  gen_saved_ignore_type_errors: bool;
  prechecked : bool option;
}

let mode_to_string = function
  | MODE_LIST_FILES -> "MODE_LIST_FILES"
  | MODE_LIST_MODES -> "MODE_LIST_MODES"
  | MODE_TYPE_AT_POS _ -> "MODE_TYPE_AT_POS"
  | MODE_TYPE_AT_POS_BATCH _ -> "MODE_TYPE_AT_POS_BATCH"
  | MODE_TYPED_FULL_FIDELITY_PARSE _ -> "MODE_TYPED_FULL_FIDELITY_PARSE"
  | MODE_AUTO_COMPLETE -> "MODE_AUTO_COMPLETE"
  | MODE_IN_MEMORY_DEP_TABLE_SIZE -> "MODE_IN_MEMORY_DEP_TABLE_SIZE"
  | MODE_SAVE_STATE _ -> "MODE_SAVE_STATE"
  | MODE_STATUS -> "MODE_STATUS"
  | MODE_STATUS_SINGLE _ -> "MODE_STATUS_SINGLE"
  | MODE_SHOW _ -> "MODE_SHOW"
  | MODE_COLORING _ -> "MODE_COLORING"
  | MODE_COVERAGE _ -> "MODE_COVERAGE"
  | MODE_FIND_REFS _ -> "MODE_FIND_REFS"
  | MODE_IDENTIFY_SYMBOL1 _ -> "MODE_IDENTIFY_SYMBOL1"
  | MODE_IDENTIFY_SYMBOL2 _ -> "MODE_IDENTIFY_SYMBOL2"
  | MODE_IDENTIFY_SYMBOL3 _ -> "MODE_IDENTIFY_SYMBOL3"
  | MODE_OUTLINE -> "MODE_OUTLINE"
  | MODE_OUTLINE2 -> "MODE_OUTLINE2"
  | MODE_METHOD_JUMP_CHILDREN _ -> "MODE_METHOD_JUMP_CHILDREN"
  | MODE_METHOD_JUMP_ANCESTORS _ -> "MODE_METHOD_JUMP_ANCESTORS"
  | MODE_METHOD_JUMP_ANCESTORS_BATCH _ -> "MODE_METHOD_JUMP_ANCESTORS_BATCH"
  | MODE_REFACTOR _ -> "MODE_REFACTOR"
  | MODE_IDE_REFACTOR _ -> "MODE_IDE_REFACTOR"
  | MODE_FIND_CLASS_REFS _ -> "MODE_FIND_CLASS_REFS"
  | MODE_SEARCH _ -> "MODE_SEARCH"
  | MODE_LINT _ -> "MODE_LINT"
  | MODE_LINT_STDIN _ -> "MODE_LINT_STDIN"
  | MODE_LINT_ALL _ -> "MODE_LINT_ALL"
  | MODE_DUMP_SYMBOL_INFO _ -> "MODE_DUMP_SYMBOL_INFO"
  | MODE_CREATE_CHECKPOINT _ -> "MODE_CREATE_CHECKPOINT"
  | MODE_RETRIEVE_CHECKPOINT _ -> "MODE_RETRIEVE_CHECKPOINT"
  | MODE_DELETE_CHECKPOINT _ -> "MODE_DELETE_CHECKPOINT"
  | MODE_STATS -> "MODE_STATS"
  | MODE_FORMAT _ -> "MODE_FORMAT"
  | MODE_REMOVE_DEAD_FIXMES _ -> "MODE_REMOVE_DEAD_FIXMES"
  | MODE_IDE_FIND_REFS _ -> "MODE_IDE_FIND_REFS"
  | MODE_IDE_HIGHLIGHT_REFS _ -> "MODE_IDE_HIGHLIGHT_REFS"
  | MODE_AI_QUERY _ -> "MODE_AI_QUERY"
  | MODE_FULL_FIDELITY_PARSE _ -> "MODE_FULL_FIDELITY_PARSE"
  | MODE_FULL_FIDELITY_SCHEMA -> "MODE_FULL_FIDELITY_SCHEMA"
  | MODE_INFER_RETURN_TYPE _ -> "MODE_INFER_RETURN_TYPE"
  | MODE_CST_SEARCH _ -> "MODE_CST_SEARCH"
