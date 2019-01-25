open OUnit

let suite = "Main" >:::
            [
              Test_regex.suite;
              Test_pathexp.suite;
              Test_recgraph.suite;
              Test_sese.suite;
            ]

let _ =
  Printf.printf "Running apak test suite";
  run_test_tt_main suite
