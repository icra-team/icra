open Apak
open Arg

module Log = Srk.Log

let parameterized   = ref false

let sanity_checks   = ref true
let display_graphs  = ref false
let show_stats      = ref false
let cflags          = ref "" (** Extra flags for C front-end *)

let widening        = ref true

(** Enable/disable possibility of failure for builtin functions *)
let fail_malloc     = ref false
let fail_fork       = ref false

(* Command line arguments *****************************************************)

let verbosity_arg =
  ("-verbosity",
   Arg.String (fun v -> Log.verbosity_level := (Log.level_of_string v)),
   " Set verbosity level (higher = more verbose; defaults to 0)")

let verbose_arg =
  ("-verbose",
   Arg.String (fun v -> Log.set_verbosity_level v `info),
   " Raise verbosity for a particular module")

let verbose_list_arg =
  ("-verbose-list",
   Arg.Unit (fun () ->
       print_endline "Available modules for setting verbosity:";
       Hashtbl.iter (fun k _ ->
           print_endline (" - " ^ k);
         ) Log.loggers;
       exit 0;
     ),
   " List modules which can be used with -verbose")

let stats_arg =
  ("-stats", Arg.Set show_stats, " Display statistics")

let colorize_arg =
  ("-color", Arg.Set Log.colorize, " Use ANSI colors")

let cflags_arg =
  ("-cflags",
   Arg.Set_string cflags,
   " Command line options for C pre-processor")

(** Set the load path.  The load path should be a colon-delimited list
    of directories.  The load path determines which directories that
    {!find_file} looks in. *)

let load_path_arg =
  ("-loadpath", Arg.String Putil.set_load_path, " Set load path")

(* default load path *)
let _ = Putil.set_load_path "/usr/share/duet:."

let parameterized_arg =
  ("-parameterized", Arg.Set parameterized, " Use parameterized model")

let whole_program_arg =
  ("-no-whole-program",
   Arg.Clear CfgIr.whole_program,
   " Turn off whole-program analysis")

let temp_dir_arg =
  ("-temp-dir",
   Arg.String (Putil.set_temp_dir),
   " Set temp dir")

let fail_malloc_arg =
  ("-fail-malloc", Arg.Set fail_malloc, " Allow mallocs to fail")

let fail_fork_arg =
  ("-fail-fork", Arg.Set fail_fork, " Allow forks to fail")

(** Debug args *)
let debug_arg =
  ("-debug", Arg.Set Log.debug_mode, " Print debugging information")

let debug_phase_arg =
  let add_debug_phase phase =
    Log.debug_phases := phase::(!Log.debug_phases)
  in
  ("-debug-phase",
   Arg.String add_debug_phase,
   " Print debugging information for a particular phase")

let sanity_check_arg =
  ("-no-sanity-check", Arg.Clear sanity_checks, " Disable sanity checks")

let display_graphs_arg =
  ("-display-graphs",
   Arg.String (fun cmd ->
       display_graphs := true;
       Apak.ExtGraph.display_image := cmd),
   " Display graphs (using supplied image viewer)")

let test_mode_arg = 
  ("-test", Arg.Set_string Report.output_file, " Set an output file in test mode")

let pass_args : (key * spec * doc) list ref = ref []

let debug_args = ref
    [
      debug_arg;
      debug_phase_arg;
      sanity_check_arg;
      display_graphs_arg;
      test_mode_arg;
    ]

let config_args = ref
    [
      verbose_arg;
      verbose_list_arg;
      verbosity_arg;
      stats_arg;
      colorize_arg;
      whole_program_arg;
      cflags_arg;
      load_path_arg;
      parameterized_arg;
      temp_dir_arg;
      fail_malloc_arg;
      fail_fork_arg
    ]

let passes : (CfgIr.file -> unit) list ref = ref []
let run file = List.iter (fun f -> f file) (List.rev (!passes))

let register_pass (name, pass, doc) =
  let a = (name, Arg.Unit (fun () -> passes := pass::(!passes)), doc) in
  pass_args := a::!pass_args
let register_debug a = debug_args := a::!debug_args
let register_debug_pass (name, pass, doc) =
  let a = (name, Arg.Unit (fun () -> passes := pass::(!passes)), doc) in
  debug_args := a::!debug_args
let register_config a = config_args := a::!config_args

let spec_list () = (!pass_args) @ (!config_args) @ (!debug_args)

let parsers : (string * (string -> CfgIr.file)) list ref = ref []
let register_parser p = parsers := p::!parsers
let parse filename =
  let rec go = function
    | ((suffix, parse)::tl) ->
      if Filename.check_suffix filename suffix then begin
        let file = Log.phase "Parsing" parse filename in
        CfgIr.gfile := Some file;
        file
      end else go tl
    | [] -> 
      failwith "Unrecognized file extension"
  in
  go (!parsers)


let _ =
  register_pass
    ("-split-atomic",
     CfgIr.split_atomic,
     " Split commands into atomic instructions");
  register_pass
    ("-dump-cfgir",
     (fun file -> CfgIr.write_file file stdout),
     " Dump CFG IR");
  register_parser ("duet",
                   fun filename ->
                     let chan = open_in filename in
                     let file = CfgIr.read_file chan in
                     let open Core in
                     close_in chan;
                     CfgIr.iter_defs (fun def -> Def.set_max_id def.did) file;
                     file)

