open Syntax
open BatPervasives

include Log.Make(struct let name = "srk.linear" end)

module IntMap = struct
  include SrkUtil.Int.Map
  let hash ring_hash vec =
    BatEnum.fold
      (fun hash (k, v) -> hash + (Hashtbl.hash (k, ring_hash v)))
      0
      (enum vec)
end
module IntSet = SrkUtil.Int.Set

module ZZVector = struct
  include Ring.RingMap(IntMap)(ZZ)

  let pp formatter vec =
    let pp_elt formatter (k, v) = Format.fprintf formatter "%d:%a" k ZZ.pp v in
    IntMap.enum vec
    |> Format.fprintf formatter "[@[%a@]]" (SrkUtil.pp_print_enum pp_elt)

  let pp_term pp_dim formatter vec =
    let pp_elt formatter (k, v) = Format.fprintf formatter "%a * %a" ZZ.pp v pp_dim k in
    if IntMap.is_empty vec then
      Format.pp_print_string formatter "0"
    else
      IntMap.enum vec
      |> Format.fprintf formatter "[@[%a@]]" (SrkUtil.pp_print_enum pp_elt)

  let show = SrkUtil.mk_show pp
  let compare = IntMap.compare ZZ.compare
  let hash = IntMap.hash ZZ.hash
end

module QQVector = struct
  include Ring.RingMap(IntMap)(QQ)

  let pp formatter vec =
    let pp_elt formatter (k, v) = Format.fprintf formatter "%d:%a" k QQ.pp v in
    IntMap.enum vec
    |> Format.fprintf formatter "[@[%a@]]" (SrkUtil.pp_print_enum pp_elt)

  let pp_term pp_dim formatter vec =
    let pp_elt formatter (k, v) = Format.fprintf formatter "%a * %a" QQ.pp v pp_dim k in
    if IntMap.is_empty vec then
      Format.pp_print_string formatter "0"
    else
      IntMap.enum vec
      |> Format.fprintf formatter "[@[%a@]]" (SrkUtil.pp_print_enum pp_elt)

  let show = SrkUtil.mk_show pp
  let compare = IntMap.compare QQ.compare
  let hash = IntMap.hash QQ.hash
end

module QQMatrix = struct
  include Ring.MakeMatrix(QQ)
  let pp = pp QQ.pp
  let show = SrkUtil.mk_show pp

  let exp m p =
    let one =
      identity (SrkUtil.Int.Set.elements (SrkUtil.Int.Set.union (row_set m) (column_set m)))
    in
    SrkUtil.exp mul one m p

  let rational_eigenvalues m dims =
    let denominator =
      BatEnum.fold (fun d (_, _, entry) ->
          ZZ.lcm d (QQ.denominator entry))
        ZZ.one
        (entries m)
    in
    let dim_array = Array.of_list dims in
    let m =
      Array.map (fun i ->
          Array.map (fun j ->
              let (num, den) = QQ.to_zzfrac (entry i j m) in
              Ntl.ZZ.of_mpz (ZZ.div (ZZ.mul num denominator) den))
            dim_array)
        dim_array
    in
    let charpoly = Ntl.ZZMatrix.charpoly m in
    let (_, factors) = Ntl.ZZX.factor charpoly in
    factors |> BatList.filter_map (fun (p, m) ->
        if Ntl.ZZX.degree p == 1 then
          (* p = ax + b *)
          let a = Ntl.ZZ.mpz_of (Ntl.ZZX.get_coeff p 1) in
          let b = Ntl.ZZ.mpz_of (Ntl.ZZX.get_coeff p 0) in
          let eigenvalue =
            QQ.negate (QQ.of_zzfrac b (ZZ.mul a denominator))
          in
          Some (eigenvalue, m)
        else
          None)
end

exception No_solution

let row_eschelon_form mat b_column =
  let rec reduce finished mat =
    if QQMatrix.equal mat QQMatrix.zero then
      finished
    else
      let (row_num, _) = QQMatrix.min_row mat in
      let (next_row, mat') = QQMatrix.pivot row_num mat in
      let column =
        try BatEnum.find (fun (_, i) -> i != b_column) (QQVector.enum next_row) |> snd
        with Not_found -> raise No_solution
      in
      let (cell, next_row') = QQVector.pivot column next_row in
      let next_row' =
        QQVector.scalar_mul (QQ.negate (QQ.inverse cell)) next_row'
      in
      let f row =
        let (coeff, row') = QQVector.pivot column row in
        QQVector.add
          row'
          (QQVector.scalar_mul coeff next_row')
      in
      reduce ((column,next_row')::finished) (QQMatrix.map_rows f mat')
  in
  reduce [] mat

let nullspace mat dimensions =
  let open QQMatrix in
  let columns = column_set mat in
  let b_column =
    1 + (IntSet.fold max columns (List.fold_left max 0 dimensions))
  in
  let rr = row_eschelon_form mat b_column in
  let rec backprop soln = function
    | [] -> soln
    | ((lhs, rhs)::rest) ->
      backprop (QQVector.add_term (QQVector.dot soln rhs) lhs soln) rest
  in
  let free_dimensions =
    List.filter (fun x ->
        not (List.exists (fun (y, _) -> x = y) rr))
      dimensions
  in
  free_dimensions |> List.map (fun d ->
      let start = QQVector.of_term QQ.one d in
      backprop start rr)

let solve_exn mat b =
  let open QQMatrix in
  logf "Solving system:@\nM:  %a@\nb:  %a" pp mat QQVector.pp b;
  let columns = column_set mat in
  let b_column = 1 + (IntSet.fold max columns 0) in
  let mat = add_column b_column b mat in
  let rr = row_eschelon_form mat b_column in
  let rec backprop soln = function
    | [] -> soln
    | ((lhs, rhs)::rest) ->
      backprop (QQVector.add_term (QQVector.dot soln rhs) lhs soln) rest
  in
  let res =
    backprop (QQVector.of_term QQ.one b_column) rr
    |> QQVector.pivot b_column
    |> snd
    |> QQVector.negate
  in
  logf "Solution: %a" QQVector.pp res;
  res

let solve mat b =
  try Some (solve_exn mat b)
  with No_solution -> None

let orient p system =
  let module V = QQVector in
  let rec reduce fin sys =
    match sys with
    | [] -> fin
    | (eq::rest) ->
      if V.equal eq V.zero then
        reduce fin rest
      else
        try
          let (coeff, dim) =
            BatEnum.find (fun (_, dim) -> not (p dim)) (V.enum eq)
          in
          let coeff_inv = QQ.inverse coeff in
          let sub eq' =
            try
              let coeff' = V.coeff dim eq' in
              let k = QQ.negate (QQ.mul coeff_inv coeff') in
              V.add (V.scalar_mul k eq) eq'
            with Not_found -> eq'
          in
          let rhs =
            V.scalar_mul (QQ.negate coeff_inv) (snd (V.pivot dim eq))
          in
          reduce
            ((dim,rhs)::(List.map (fun (dim, rhs) -> (dim, sub rhs)) fin))
            (List.map sub rest)
        with Not_found -> reduce fin rest (* No variable to eliminate *)
  in
  reduce [] system

let vector_right_mul = QQMatrix.vector_right_mul
let vector_left_mul = QQMatrix.vector_left_mul

let intersect_rowspace a b =
  (* Create a system lambda_1*A - lambda_2*B = 0.  lambda_1's occupy even
     columns and lambda_2's occupy odd. *)
  let mat_a =
    BatEnum.fold
      (fun mat (i, j, k) -> QQMatrix.add_entry j (2*i) k mat)
      QQMatrix.zero
      (QQMatrix.entries a)
  in
  let mat =
    ref (BatEnum.fold
           (fun mat (i, j, k) -> QQMatrix.add_entry j (2*i + 1) (QQ.negate k) mat)
           mat_a
           (QQMatrix.entries b))
  in
  let c = ref QQMatrix.zero in
  let d = ref QQMatrix.zero in
  let c_rows = ref 0 in
  let d_rows = ref 0 in
  let mat_rows =
    ref (BatEnum.fold (fun m (i, _) -> max m i) 0 (QQMatrix.rowsi (!mat)) + 1)
  in

  (* Loop through the columns col of A/B, trying to find a vector in the
     intersection of the row spaces of A and B and which has 1 in col's entry.
     If yes, add it the linear combinations to C/D, and add a constraint to
     mat that (in all future rows of CA), col's entry is 0.  This ensures that
     the rows of CA are linearly independent. *)
  (* to do: repeatedly solving super systems of the same system of equations
       -- can be made more efficient *)
  (QQMatrix.rowsi (!mat))
  |> (BatEnum.iter (fun (col, _) ->
      let mat' =
        QQMatrix.add_row
          (!mat_rows)
          (QQMatrix.row col mat_a)
          (!mat)
      in
      match solve mat' (QQVector.of_term QQ.one (!mat_rows)) with
      | Some solution ->
        let (c_row, d_row) =
          BatEnum.fold (fun (c_row, d_row) (entry, i) ->
              if i mod 2 = 0 then
                (QQVector.add_term entry (i/2) c_row, d_row)
              else
                (c_row, QQVector.add_term entry (i/2) d_row))
            (QQVector.zero, QQVector.zero)
            (QQVector.enum solution)
        in
        c := QQMatrix.add_row (!c_rows) c_row (!c);
        d := QQMatrix.add_row (!d_rows) d_row (!d);
        mat := mat';
        incr c_rows; incr d_rows; incr mat_rows
      | None -> ()));
  (!c, !d)

let divide_right a b =
  try
    let b_tr = QQMatrix.transpose b in
    let div =
      BatEnum.fold (fun div (i, row) ->
          QQMatrix.add_row i (solve_exn b_tr row) div)
        QQMatrix.zero
        (QQMatrix.rowsi a)
    in
    Some div
  with No_solution -> None

(* Given matrices A and B, find a matrix C whose rows constitute a basis for
   the vector space { v : exists u. uA = vB } *)
let max_rowspace_projection a b =
  (* Create a system u*A - v*B = 0.  u's occupy even columns and v's occupy
     odd. *)
  let mat_a =
    BatEnum.fold
      (fun mat (i, j, k) -> QQMatrix.add_entry j (2*i) k mat)
      QQMatrix.zero
      (QQMatrix.entries a)
  in
  let mat =
    ref (BatEnum.fold
           (fun mat (i, j, k) -> QQMatrix.add_entry j (2*i + 1) (QQ.negate k) mat)
           mat_a
           (QQMatrix.entries b))
  in
  let c = ref QQMatrix.zero in
  let c_rows = ref 0 in
  let mat_rows =
    ref (BatEnum.fold (fun m (i, _) -> max m i) 0 (QQMatrix.rowsi (!mat)) + 1)
  in

  (* Loop through the columns col of A/B, trying to find a vector u and v such
     that uA = vB and v has 1 in col's entry.  If yes, add v to C, and add a
     constraint to mat that (in all future rows of C), col's entry is 0.  This
     ensures that the rows of C are linearly independent. *)
  (* to do: repeatedly solving super systems of the same system of equations
       -- can be made more efficient *)
  (QQMatrix.rowsi b)
  |> (BatEnum.iter (fun (r, _) ->
      let col = 2*r + 1 in
      let mat' =
        QQMatrix.add_row
          (!mat_rows)
          (QQVector.of_term QQ.one col)
          (!mat)
      in
      match solve mat' (QQVector.of_term QQ.one (!mat_rows)) with
      | Some solution ->
        let c_row =
          BatEnum.fold (fun c_row (entry, i) ->
              if i mod 2 = 1 then
                QQVector.add_term entry (i/2) c_row
              else
                c_row)
            QQVector.zero
            (QQVector.enum solution)
        in
        assert (not (QQVector.equal c_row QQVector.zero));
        c := QQMatrix.add_row (!c_rows) c_row (!c);
        mat := mat';
        incr c_rows; incr mat_rows
      | None -> ()));
  !c

let max_lds mA mB =
  (* We have a system of the form Ax' = Bx, we need one of the form Ax' =
     B'Ax.  If we can factor B = B'A, we're done.  Otherwise, we compute an
     m-by-n matrix T' with m < n, and continue iterating with the system T'Ax'
     = T'Bx. *)
  let rec fix mA mB mT =
    let mS = max_rowspace_projection mA mB in
    (* Since matrices are sparse, need to account for 0-rows of B -- they
       should always be in the max rowspace projection *)
    let mT' =
      SrkUtil.Int.Set.fold
        (fun i (mT', nb_rows) ->
           if QQVector.is_zero (QQMatrix.row i mB) then
             let mT' =
               QQMatrix.add_row nb_rows (QQVector.of_term QQ.one i) mT'
             in
             (mT', nb_rows + 1)
           else
             (mT', nb_rows))
        (QQMatrix.row_set mA)
        (mS, QQMatrix.nb_rows mB)
      |> fst
    in
    if QQMatrix.nb_rows mB = QQMatrix.nb_rows mS then
      match divide_right mB mA with
      | Some mM ->
        assert (QQMatrix.equal (QQMatrix.mul mM mA) mB);

        (mT, mM)
      | None ->
        (* mS's rows are linearly independent -- if it has as many rows as B,
           then the rowspace of B is contained inside the rowspace of A, and
           B/A is defined. *)
        assert false
    else
      fix (QQMatrix.mul mT' mA) (QQMatrix.mul mT' mB) (QQMatrix.mul mT' mT)

  in
  let dims =
    SrkUtil.Int.Set.elements
      (SrkUtil.Int.Set.union (QQMatrix.row_set mA) (QQMatrix.row_set mB))
  in
  let (mT, mM) = fix mA mB (QQMatrix.identity dims) in
  (* Remove coordinates corresponding to zero rows of T*A *)
  let mTA = QQMatrix.mul mT mA in
  let mTA_rows = QQMatrix.row_set mTA in
  BatEnum.foldi (fun i row (mT', mM') ->
      let mT' =
        QQMatrix.add_row i (QQMatrix.row row mT) mT'
      in
      let mM' =
        let mM_row = QQMatrix.row row mM in
        let rowi =
          BatEnum.foldi (fun j col v ->
              QQVector.add_term (QQVector.coeff col mM_row) j v)
            QQVector.zero
            (SrkUtil.Int.Set.enum mTA_rows)
        in
        QQMatrix.add_row i rowi mM'
      in
      (mT', mM'))
    (QQMatrix.zero, QQMatrix.zero)
    (SrkUtil.Int.Set.enum mTA_rows)

let rational_spectral_decomposition mA dims =
  let mAt = QQMatrix.transpose mA in
  let identity = QQMatrix.identity dims in
  List.fold_left (fun rsd (lambda, _) ->
      let mE = (* A^t - lambda*I *)
        QQMatrix.add mAt (QQMatrix.scalar_mul (QQ.negate lambda) identity)
      in
      let rec add_jordan_chain_rec rsd v =
        match solve mE v with
        | Some u -> add_jordan_chain_rec ((lambda,u)::rsd) u
        | None -> rsd
      in
      let add_jordan_chain rsd v =
        add_jordan_chain_rec ((lambda,v)::rsd) v
      in
      List.fold_left add_jordan_chain rsd (nullspace mE dims))
    []
    (QQMatrix.rational_eigenvalues mA dims)

let mem_vector_space basis v =
  let mA =
    BatList.fold_lefti
      (fun m i v -> QQMatrix.add_row i v m)
      QQMatrix.zero
      basis
  in
  match solve (QQMatrix.transpose mA) v with
  | Some _ -> true
  | None -> false

let periodic_rational_spectral_decomposition mA dims =
  let nb_dims = List.length dims in
  let max_pow = nb_dims*nb_dims*nb_dims in
  let rec go prsd i mA_pow =
    if i > max_pow || List.length prsd == nb_dims then
      prsd
    else
      let prsd =
        List.fold_left (fun prsd (lambda, v) ->
            if mem_vector_space (List.map (fun (_,_,v) -> v) prsd) v then
              prsd
            else
              (i, lambda, v)::prsd)
          prsd
          (rational_spectral_decomposition mA_pow dims)
      in
      go prsd (i+1) (QQMatrix.mul mA mA_pow)
  in
  go [] 1 mA

let rational_triangulation mA =
  let mAt = QQMatrix.transpose mA in
  let next_row =
    let r = ref 0 in
    fun () ->
      let nr = !r in
      incr r;
      nr
  in
  let dims = SrkUtil.Int.Set.elements (QQMatrix.row_set mAt) in
  let identity = QQMatrix.identity dims in
  List.fold_left (fun (mM, mT) (lambda, _) ->
      let mE = (* A^t - lambda*I *)
        QQMatrix.add mAt (QQMatrix.scalar_mul (QQ.negate lambda) identity)
      in
      (* Assuming that that the last row of M is v, add the Jordan chain of
         lambda/v to M, and the corresponding Jordan block to T. *)
      let rec add_jordan_chain_rec (mM, mT) v =
        match solve mE v with
        | Some u ->
          let row = next_row () in
          let mM = QQMatrix.add_row row u mM in
          let t_row =
            QQVector.of_list [(QQ.one, row-1); (lambda, row)]
          in
          let mT = QQMatrix.add_row row t_row mT in
          add_jordan_chain_rec (mM, mT) u
        | None -> (mM, mT)
      in
      let add_jordan_chain (mM, mT) v =
        let row = next_row () in
        let mM = QQMatrix.add_row row v mM in
        let t_row = QQVector.of_term lambda row in
        let mT = QQMatrix.add_row row t_row mT in
        add_jordan_chain_rec (mM, mT) v
      in
      List.fold_left add_jordan_chain (mM, mT) (nullspace mE dims)
    )
    (QQMatrix.zero, QQMatrix.zero)
    (QQMatrix.rational_eigenvalues mA dims)

let rec jordan_chain mA lambda v =
  let residual = (* v*mA = lambda*v + residual *)
    QQVector.sub
      (QQMatrix.vector_left_mul v mA)
      (QQVector.scalar_mul lambda v)
  in
  if QQVector.is_zero residual then
    [v]
  else
    v::(jordan_chain mA lambda residual)

(* Affine expressions over constant symbols.  dim_of_sym, const_dim, and
   sym_of_dim are used to translate between symbols and the dimensions of the
   coordinate space. *)

exception Nonlinear

let const_dim = -1

let sym_of_dim dim =
  if dim >= 0 then Some (symbol_of_int dim)
  else None

let dim_of_sym k = int_of_symbol k

let const_linterm k = QQVector.of_term k const_dim

let linterm_size linterm = BatEnum.count (QQVector.enum linterm)

let const_of_linterm v =
  let (k, rest) = QQVector.pivot const_dim v in
  if QQVector.equal rest QQVector.zero then Some k
  else None

let linterm_of srk term =
  let open QQVector in
  let real qq = of_term qq const_dim in
  let pivot_const = pivot const_dim in
  let qq_of term =
    let (k, rest) = pivot_const term in
    if equal rest zero then k
    else raise Nonlinear
  in
  let nonzero_qq_of term =
    let qq = qq_of term in
    if QQ.equal qq QQ.zero then raise Nonlinear else qq
  in
  let mul x y =
    try scalar_mul (qq_of x) y
    with Nonlinear -> scalar_mul (qq_of y) x
  in
  let alg = function
    | `Real qq -> real qq
    | `App (k, []) -> of_term QQ.one (dim_of_sym k)
    | `Var (_, _) | `App (_, _) -> raise Nonlinear
    | `Add sum -> List.fold_left add zero sum
    | `Mul sum -> List.fold_left mul (real QQ.one) sum
    | `Binop (`Div, x, y) -> scalar_mul (QQ.inverse (nonzero_qq_of y)) x
    | `Binop (`Mod, x, y) -> real (QQ.modulo (qq_of x) (nonzero_qq_of y))
    | `Unop (`Floor, x) -> real (QQ.of_zz (QQ.floor (qq_of x)))
    | `Unop (`Neg, x) -> negate x
    | `Ite (_, _, _) -> raise Nonlinear
  in
  Term.eval srk alg term

let of_linterm srk linterm =
  let open QQVector in
  enum linterm
  /@ (fun (coeff, dim) ->
      match sym_of_dim dim with
      | Some k ->
        if QQ.equal coeff QQ.one then mk_const srk k
        else mk_mul srk [mk_real srk coeff; mk_const srk k]
      | None -> mk_real srk coeff)
  |> BatList.of_enum
  |> mk_add srk

let pp_linterm srk formatter linterm =
  Term.pp srk formatter (of_linterm srk linterm)

let evaluate_linterm interp term =
  (QQVector.enum term)
  /@ (fun (coeff, dim) ->
      match sym_of_dim dim with
      | Some const -> QQ.mul (interp const) coeff
      | None -> coeff)
  |> BatEnum.fold QQ.add QQ.zero

let evaluate_affine m term =
  (QQVector.enum term)
  /@ (fun (coeff, dim) ->
      if dim == const_dim then
        coeff
      else
        QQ.mul (m dim) coeff)
  |> BatEnum.fold QQ.add QQ.zero

