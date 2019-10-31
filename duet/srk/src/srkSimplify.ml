open Syntax
open BatPervasives

include Log.Make(struct let name = "srk.simplify" end)

module TermPolynomial = struct
  type 'a polynomial_context =
    { srk : 'a context;
      int_of : 'a term -> int;
      of_int : int -> 'a term }

  module QQXs = Polynomial.QQXs
  module Monomial = Polynomial.Monomial
  module DynArray = BatDynArray

  type 'a t = QQXs.t

  let mk_context srk =
    let table = Expr.HT.create 991 in
    let enum = DynArray.create () in
    let of_int = DynArray.get enum in
    let int_of term =
      if Expr.HT.mem table term then
        Expr.HT.find table term
      else
        let id = DynArray.length enum in
        DynArray.add enum term;
        Expr.HT.add table term id;
        id
    in
    { srk; of_int; int_of }

  let rec of_term ctx =
    let srk = ctx.srk in
    let mvp_term = QQXs.of_dim % ctx.int_of in
    let alg = function
      | `Add xs -> List.fold_left QQXs.add QQXs.zero xs
      | `Mul xs -> List.fold_left QQXs.mul QQXs.one xs
      | `Real k -> QQXs.scalar k
      | `Unop (`Neg, x) -> QQXs.negate x
      | `Unop (`Floor, x) -> mvp_term (mk_floor srk (term_of ctx x))
      | `App (f, args) -> mvp_term (mk_app srk f args)
      | `Binop (`Div, x, y) ->
        mvp_term (mk_div srk (term_of ctx x) (term_of ctx y))
      | `Binop (`Mod, x, y) ->
        mvp_term (mk_mod srk (term_of ctx x) (term_of ctx y))
      | `Ite (cond, bthen, belse) ->
        mvp_term (mk_ite srk cond (term_of ctx bthen) (term_of ctx belse))
      | `Var (v, typ) -> mvp_term (mk_var srk v (typ :> typ_fo))
    in
    Term.eval srk alg
    
  and term_of ctx p =
    let srk = ctx.srk in
    (QQXs.enum p)
    /@ (fun (coeff, monomial) ->
        let product =
          BatEnum.fold
            (fun product (dim, power) ->
               let term = ctx.of_int dim in
               BatEnum.fold
                 (fun product _ -> term::product)
                 product
                 (1 -- power))
            []
            (Monomial.enum monomial)
        in
        mk_mul srk ((mk_real srk coeff)::product))
    |> BatList.of_enum
    |> mk_add ctx.srk
end

let simplify_term srk term =
  let ctx = TermPolynomial.mk_context srk in
  let polynomial =
    TermPolynomial.of_term ctx term
  in
  let c = TermPolynomial.QQXs.content polynomial in
  let polynomial =
    if QQ.equal c QQ.zero then
      polynomial
    else
      TermPolynomial.QQXs.scalar_mul (QQ.inverse (QQ.abs c)) polynomial
  in
  TermPolynomial.term_of ctx polynomial

let simplify_terms_rewriter srk =
  let ctx = TermPolynomial.mk_context srk in
  fun expr ->
    match destruct srk expr with
    | `Atom (op, s, t) ->
      let simplified_term =
        let polynomial =
          TermPolynomial.of_term ctx (mk_sub srk s t)
        in
        let c = TermPolynomial.QQXs.content polynomial in
        let polynomial =
          if QQ.equal c QQ.zero then
            polynomial
          else
            TermPolynomial.QQXs.scalar_mul (QQ.inverse (QQ.abs c)) polynomial
        in
        TermPolynomial.term_of ctx polynomial
      in
      let zero = mk_real srk QQ.zero in
      let result = match op with
        | `Leq -> mk_leq srk simplified_term zero
        | `Lt -> mk_lt srk simplified_term zero
        | `Eq -> mk_eq srk simplified_term zero 
      in
      (result :> ('a,typ_fo) expr)
    | _ -> expr

let simplify_terms srk expr =
  rewrite srk ~up:(simplify_terms_rewriter srk) expr

let purify_rewriter srk table =
  fun expr ->
    match destruct srk expr with
    | `Quantify (_, _, _, _) -> invalid_arg "purify: free variable"
    | `App (func, []) -> expr
    | `App (func, args) ->
      let sym =
        try
          Expr.HT.find table expr
        with Not_found ->
          let sym = mk_symbol srk ~name:"uninterp" (expr_typ srk expr) in
          Expr.HT.add table expr sym;
          sym
      in
      mk_const srk sym
    | _ -> expr

let purify srk expr =
  let table = Expr.HT.create 991 in
  let expr' = rewrite srk ~up:(purify_rewriter srk table) expr in
  let map =
    BatEnum.fold
      (fun map (term, sym) -> Symbol.Map.add sym term map)
      Symbol.Map.empty
      (Expr.HT.enum table)
  in
  (expr', map)

module SymDS = DisjointSet.Make(struct
    include Symbol
    let hash = Hashtbl.hash
    let equal = (=)
  end)
let partition_implicant implicant =
  let (zero_group, implicant) =
    List.partition (fun atom -> Symbol.Set.is_empty (symbols atom)) implicant
  in
  if implicant = [] then
    [zero_group]
  else begin
    let ds = SymDS.create 991 in
    implicant |> List.iter (fun atom ->
        let (sym, rest) = Symbol.Set.pop (symbols atom) in
        let rep = SymDS.find ds sym in
        Symbol.Set.iter (fun sym' -> ignore (SymDS.union (SymDS.find ds sym') rep)) rest);
    let rev_map =
      SymDS.reverse_map ds Symbol.Set.empty Symbol.Set.add
    in
    let find_rep symbol = Symbol.Set.choose (rev_map symbol) in
    let map =
      List.fold_left (fun map atom ->
          let equiv_class = find_rep (Symbol.Set.choose (symbols atom)) in
          if Symbol.Map.mem equiv_class map then
            Symbol.Map.add equiv_class (atom::(Symbol.Map.find equiv_class map)) map
          else
            Symbol.Map.add equiv_class [atom] map)
        Symbol.Map.empty
        implicant
    in
    let partitioned_implicant =
      BatList.of_enum (Symbol.Map.values map)
    in
    if zero_group = [] then
      partitioned_implicant
    else
      zero_group::partitioned_implicant
  end

let simplify_conjunction srk cube =
  let cube = List.map (simplify_terms srk) cube in
  let solver = SrkZ3.mk_solver srk in
  let indicator_map =
    List.fold_left (fun m prop ->
        Symbol.Map.add (mk_symbol srk `TyBool) prop m)
      Symbol.Map.empty
      cube
  in
  SrkZ3.Solver.add solver [mk_not srk (mk_and srk cube)];
  Symbol.Map.iter (fun indicator prop ->
      SrkZ3.Solver.add solver [mk_if srk (mk_const srk indicator) prop])
    indicator_map;
  let assumptions =
    Symbol.Map.fold
      (fun indicator _ xs -> (mk_const srk indicator)::xs)
      indicator_map
      []
  in
  match SrkZ3.Solver.get_unsat_core solver assumptions with
  | `Sat -> assert false
  | `Unknown -> cube
  | `Unsat core ->
    List.map (fun ind ->
        match Formula.destruct srk ind with
        | `Proposition (`App (sym, [])) -> Symbol.Map.find sym indicator_map
        | _ -> assert false)
      core

exception Nonlinear (* not exported *)
let isolate_linear srk x term =
  let rec go term =
    match Term.destruct srk term with
    | `Real k -> `Real k
    | `App (sym, []) when sym = x -> `Lin (QQ.one, [])
    | `Add xs ->
      List.fold_left (fun s t ->
          match s, (go t) with
          | (`Real k, `Real k') -> `Real (QQ.add k k')
          | (`Real k, `Lin (a, b))
          | (`Lin (a, b), `Real k) -> `Lin (a, (mk_real srk k)::b)
          | (`Lin (a, b), `Lin (a', b')) ->
            `Lin (QQ.add a a', b@b'))
        (`Real QQ.zero)
        xs
    | `Mul xs ->
      List.fold_left (fun s t ->
          match s, (go t) with
          | (`Real k, `Real k') -> `Real (QQ.mul k k')
          | (`Real k, _) when QQ.equal k QQ.zero -> `Real QQ.zero
          | (_, `Real k) when QQ.equal k QQ.zero -> `Real QQ.zero
          | (`Real k, `Lin (a, b)) | (`Lin (a, b), `Real k) ->
            `Lin (QQ.mul a k, [mk_mul srk [mk_real srk k; mk_add srk b]])
          | (`Lin (a, b), `Lin (a', b')) -> raise Nonlinear)
        (`Real QQ.one)
        xs
    | `Binop (`Div, s, t) ->
      begin match go s, go t with
        | (`Real k, `Real k') when not (QQ.equal k' QQ.zero) ->
          `Real (QQ.div k k')
        | (`Lin (a, b), `Real k) when not (QQ.equal k QQ.zero) ->
          `Lin (QQ.div a k, [mk_div srk (mk_add srk b) (mk_real srk k)])
        | _ ->
          if Symbol.Set.mem x (symbols term) then
            raise Nonlinear
          else
            `Lin (QQ.zero, [term])
      end
    | `Unop (`Neg, t) ->
      begin match go t with
        | `Real k -> `Real (QQ.negate k)
        | `Lin (a, b) -> `Lin (QQ.negate a, [mk_neg srk (mk_add srk b)])
      end
    | _ ->
      if Symbol.Set.mem x (symbols term) then
        raise Nonlinear
      else
        `Lin (QQ.zero, [term])
  in
  try
    (match go term with
     | `Lin (a, b) -> Some (a, mk_add srk b)
     | `Real k -> Some (QQ.zero, mk_real srk k))
  with Nonlinear -> None

let simplify_dda srk phi =
  let solver = Smt.mk_solver srk in
  let rec simplify_children star children =
    let changed = ref false in
    let rec go simplified = function
      | [] -> List.rev simplified
      | (phi::phis) ->
        Smt.Solver.push solver;
        Smt.Solver.add solver (List.map star simplified);
        Smt.Solver.add solver (List.map star phis);
        let simple_phi = simplify_dda_impl phi in
        Smt.Solver.pop solver 1;
        if not (Formula.equal phi simple_phi) then changed := true;
        go (simple_phi::simplified) phis
    in
    let rec fix children =
      let simplified = go [] children in
      if !changed then begin
        changed := false;
        fix simplified
      end else simplified
    in
    fix children

  and simplify_dda_impl phi =
    match Formula.destruct srk phi with
    | `Or xs -> mk_or srk (simplify_children (mk_not srk) xs)
    | `And xs -> mk_and srk (simplify_children (fun x -> x) xs)
    | _ ->
      Smt.Solver.push solver;
      Smt.Solver.add solver [phi];
      let simplified =
        match Smt.Solver.check solver [] with
        | `Unknown -> phi
        | `Unsat -> mk_false srk
        | `Sat ->
          Smt.Solver.pop solver 1;
          Smt.Solver.push solver;
          Smt.Solver.add solver [mk_not srk phi];
          match Smt.Solver.check solver [] with
          | `Unknown -> phi
          | `Unsat -> mk_true srk
          | `Sat -> phi
      in
      Smt.Solver.pop solver 1;
      simplified
  in
  simplify_dda_impl phi
