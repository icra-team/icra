open OUnit
open Abstract
open Syntax
open SrkApron
open Test_pervasives

module QQMatrix = Linear.QQMatrix
module SP = struct
  include Iteration.MakeDomain(Iteration.ProductWedge
                                 (SolvablePolynomial.SolvablePolynomial)
                                 (Iteration.WedgeGuard))
  let star srk symbols phi = closure (abstract srk symbols phi)
end
module SPPR = struct
  include Iteration.MakeDomain(Iteration.ProductWedge
                                 (SolvablePolynomial.SolvablePolynomialPeriodicRational)
                                 (Iteration.WedgeGuard))
  let star srk symbols phi = closure (abstract srk symbols phi)
end
module DLTS = struct
  include Iteration.MakeDomain(Iteration.Product
                                 (SolvablePolynomial.DLTS)
                                 (Iteration.PolyhedronGuard))
  let star srk symbols phi = closure (abstract srk symbols phi)
end

let assert_implies_nonlinear phi psi =
  match Wedge.is_sat srk (mk_and srk [phi; mk_not srk psi]) with
  | `Unsat -> ()
  | `Sat | `Unknown ->
    assert_failure (Printf.sprintf "%s\ndoes not imply\n%s"
                      (Formula.show srk phi)
                      (Formula.show srk psi))

let tr_symbols = [(wsym,wsym');(xsym,xsym');(ysym,ysym');(zsym,zsym')]

let prepost () =
  let phi =
    let open Infix in
    (int 0) <= x && x <= x'
  in
  let closure =
    let open Infix in
    !(x = x')
    && SP.star srk tr_symbols phi
  in
  assert_implies closure (Ctx.mk_leq (int 0) x);
  assert_implies closure (Ctx.mk_leq (int 0) x')

let simple_induction () =
  let phi =
    let open Infix in
    w' = w + (int 1)
    && x' = x + (int 2)
    && y' = y + z
    && z = (int 3)
  in
  let closure = SP.star srk tr_symbols phi in
  let result =
    let open Infix in
    (int 2)*(w' - w) = x' - x
    && (w' - w) + (x' - x) = (y' - y)
  in
  assert_implies closure result

let count_by_1 () =
  let tr_symbols = [(xsym,xsym')] in
  let phi =
    let open Infix in
    x' = x + (int 1)
    && x < y
  in
  let closure =
    let open Infix in
    x = (int 0)
    && SP.star srk tr_symbols phi
    && y <= x'
    && (int 0) <= y
  in
  let result =
    let open Infix in
    x' = y
  in
  assert_implies closure result

let count_by_2 () =
  let tr_symbols = [(xsym,xsym')] in
  let phi =
    let open Infix in
    x' = x + (int 2)
    && x < y
  in
  let closure =
    let open Infix in
    x = (int 0)
    && SP.star srk tr_symbols phi
    && y <= x'
    && (int 0) <= y
  in
  let result =
    let open Infix in
    x' = y
  in
  let y_even =
    let open Infix in
    y = (int 2) * z
  in
  assert_not_implies closure result;
  assert_implies (mk_and srk [closure; y_even]) result

let stratified1 () =
  let tr_symbols = [(xsym,xsym');(ysym,ysym')] in
  let phi =
    let open Infix in
    x' = x + (int 1)
    && y' = y + z
  in
  let closure = SP.star srk tr_symbols phi in
  let result =
    let open Infix in
    z*(x' - x) = (y' - y)
  in
  assert_implies closure result

let stratified2 () =
  let tr_symbols = [(xsym,xsym');(ysym,ysym')] in
  let phi =
    let open Infix in
    x' = x + (int 1)
    && y' = y + x
  in
  let closure =
    let open Infix in
    x = (int 0)
    && y = (int 0)
    && SP.star srk tr_symbols phi
  in
  let result =
    let open Infix in
    (int 2)*y' = x'*(x' - (int 1))
  in
  assert_implies closure result

let count_by_k () =
  let tr_symbols = [(xsym,xsym')] in
  let phi =
    let open Infix in
    x' = x + z
    && x < y
  in
  let closure =
    let open Infix in
    x = (int 0)
    && (int 1) <= z
    && SP.star srk tr_symbols phi
    && y <= x'
  in
  let result =
    let open Infix in
    x' <= (int 100) * z
  in
  let z_div_y =
    let open Infix in
    y = (int 100) * z
  in
  assert_not_implies closure result;
  assert_implies (mk_and srk [closure; z_div_y]) result

let ineq1 () =
  let phi =
    let open Infix in
    z' = z + (int 1)
    && ((x' = x + (int 1) && y' = y)
        || (x' = x && y' = y + (int 1)))
  in
  let closure =
    let open Infix in
    x = (int 0)
    && y = (int 0)
    && z = (int 0)
    && SP.star srk tr_symbols phi
  in
  let result =
    let open Infix in
    x' + y' = z'
    && x' <= z'
    && y' <= z'
    && (int 0) <= x'
    && (int 0) <= y'
  in
  assert_implies closure result

let ineq2 () =
  let phi =
    let open Infix in
    x' = x + (int 1)
    && ((y' = y + (int 1) || y' = y + (int 10)))
  in
  let closure =
    let open Infix in
    x = (int 0)
    && y = (int 0)
    && SP.star srk tr_symbols phi
  in
  let result =
    let open Infix in
    x' <= y'
    && y' <= (int 10) * x'
  in
  assert_implies closure result

let stratified_ineq1 () =
  let phi =
    let open Infix in
    x' = x + (int 1)
    && (int 0) <= x
    && ((y' = y + (int 1) || y' = y + x + (int 1)))
  in
  let closure =
    let open Infix in
    x = (int 0)
    && y = (int 0)
    && SP.star srk tr_symbols phi
  in
  let result =
    let open Infix in
    x' <= y'
    && (int 2)*y' <= x'*(x' + (int 1))
    && (int 0) <= x'
  in
  assert_implies closure result

let periodic_rational1 () =
  let open Infix in
  let phi =
    x' = x + (int 1)
    && y' = z
    && z' = (int 0) - y
  in
  let closure =
    x = (int 0)
    && y = (int 42)
    && SPPR.star srk tr_symbols phi
  in
  assert_implies closure (!(x' = int 8) || y' = (int 42));
  assert_implies closure (!(x' = int 15) || z' = (int 42))

let periodic_rational2 () =
  let open Infix in
  let phi =
    x' = x + (int 1)
    && x < (int 31)
    && y' = z
    && z' = z - y
  in
  let closure =
    x = (int 0)
    && y = (int 42)
    && z = (int 24)
    && SPPR.star srk tr_symbols phi
    && (int 31) <= x'
  in
  assert_implies closure (z' = (int (-18)))

let periodic_rational3 () =
  let open Infix in
  let phi =
    w' = w + (int 1)
    && x' = y
    && y' = z
    && z' = x + (int 1)
  in
  let closure =
    w = (int 0)
    && x = (int 0)
    && y = (int 0)
    && z = (int 0)
    && SPPR.star srk tr_symbols phi
  in
  assert_implies closure (!(w' = (int 9)) || x' = (int 3));
  assert_implies closure (!(w' = (int 10)) || x' = (int 3));
  assert_implies closure (!(w' = (int 11)) || x' = (int 3));
  assert_implies closure (!(w' = (int 12)) || x' = (int 4))

let periodic_rational4 () =
  let open Infix in
  let phi =
    w' = w + (int 1)
    && x' = y
    && y' = z
    && z' = x + w
  in
  let closure =
    w = (int 0)
    && x = (int 0)
    && y = (int 0)
    && z = (int 0)
    && SPPR.star srk tr_symbols phi
  in
  assert_implies closure (!(w' = (int 0)) || (x'-z') = (int 0));
  assert_implies closure (!(w' = (int 1)) || (x'-z') = (int 0));
  assert_implies closure (!(w' = (int 2)) || (x'-z') = (int (-1)));
  assert_implies closure (!(w' = (int 3)) || (x'-z') = (int (-2)));
  assert_implies closure (!(w' = (int 4)) || (x'-z') = (int (-2)));

  assert_implies closure (!(w' = (int 0)) || z' = (int 0));
  assert_implies closure (!(w' = (int 1)) || z' = (int 0));
  assert_implies closure (!(w' = (int 2)) || z' = (int 1));
  assert_implies closure (!(w' = (int 3)) || z' = (int 2));
  assert_implies closure (!(w' = (int 4)) || z' = (int 3));

  assert_implies closure (!(w' = (int 9)) || x' = (int 9));
  assert_implies closure (!(w' = (int 10)) || x' = (int 12));
  assert_implies closure (!(w' = (int 11)) || x' = (int (15)));
  ()

let periodic_rational5 () =
  let open Infix in
  let phi =
    w' = w + (int 1)
    && x' = w
    && y' = x
    && z' = y
  in
  let closure =
    w = (int 0)
    && x = (int 3)
    && y = (int 2)
    && z = (int 1)
    && SPPR.star srk tr_symbols phi
  in
  assert_implies closure (!(w' = (int 1)) || z' = (int 2));
  assert_implies closure (!(w' = (int 2)) || z' = (int 3));
  assert_implies closure (!(w' = (int 3)) || z' = (int 0));
  assert_implies closure (!(w' = (int 4)) || z' = (int 1))

let dlts1 () =
  let open Infix in
  let phi =
    x' = x + (int 1)
    && x = (int 0)
  in
  let closure = DLTS.star srk tr_symbols phi in
  assert_equal (Smt.is_sat srk (closure && x < x')) `Sat;
  assert_implies closure (x' = x || x' = (int 1))

let dlts2 () =
  let open Infix in
  let phi =
    x' = x + z
    && y' = y - z
    && z' = (int 2) * z
    && x = y
  in
  let closure = DLTS.star srk tr_symbols phi in
  assert_equal (Smt.is_sat srk (closure && x < x')) `Sat;
  assert_implies closure (z' = (int 2) * z || z' = z)

let dlts3 () =
  let open Infix in
  let phi =
    x' = (int 0)
    && y' = y + (int 2) * x
    && x = (int 0)
  in
  let closure = DLTS.star srk tr_symbols phi in
  assert_implies closure (x = x' && y = y')

let dlts4 () =
  let open Infix in
  let phi =
    x' = (int 2) * x
    && x' = (int 3) * x + y
    && y' = y + (int 2)
    && z' = z + (int 1)
  in
  let closure = DLTS.star srk tr_symbols phi in
  assert_equal (Smt.is_sat srk (closure && z' = z + (int 2))) `Sat;
  assert_implies closure (z' = z || x + y = (int 0) || x + (int 2) = (int 0));
  assert_implies closure (z' - z <= (int 2))

let dlts5 () =
  let open Infix in
  let phi =
    x' = x + (int 1)
    && y' = y + (int 1)
    && z' = z
    && x = (int 0)
    && y = (int 0)
    && z = (int 1)
  in
  let closure = DLTS.star srk tr_symbols phi in
  assert_equal (Smt.is_sat srk (closure && x' = x + (int 1))) `Sat;
  assert_implies closure (x' - x <= (int 1))

let dlts_false () =
  let open Infix in
  let closure = DLTS.star srk tr_symbols (mk_false srk) in
  assert_implies closure (x' = x && y' = y && z' = z);
  assert_equal (Smt.is_sat srk closure) `Sat

let dlts_one () =
  let open Infix in
  let phi =
    x' = x + (int 1)
    && y' = y + (int 1)
    && z' = z
    && z = (int 1)
    && x = y
  in
  let closure = DLTS.star srk tr_symbols phi in
  assert_equal (Smt.is_sat srk (closure && x' = x + (int 100))) `Sat;
  assert_implies closure (z' = z || z' = (int 1))

let suite = "Iteration" >::: [
    "prepost" >:: prepost;
    "simple_induction" >:: simple_induction;
    "count_by_1" >:: count_by_1;
    "count_by_2" >:: count_by_2;
    "stratified1" >:: stratified1;
    "stratified2" >:: stratified2;
    "count_by_k" >:: count_by_k;
    "ineq1" >:: ineq1;
    "ineq2" >:: ineq2;
    "stratified_ineq1" >:: stratified_ineq1;
    "periodic_rational1" >:: periodic_rational1;
    "periodic_rational2" >:: periodic_rational2;
    "periodic_rational3" >:: periodic_rational3;
    "periodic_rational4" >:: periodic_rational4;
    "periodic_rational5" >:: periodic_rational5;
    "dlts1" >:: dlts1;
    "dlts2" >:: dlts2;
    "dlts3" >:: dlts3;
    "dlts4" >:: dlts4;
    "dlts5" >:: dlts5;
    "dlts_false" >:: dlts_false;
    "dlts_one" >:: dlts_one;
  ]
