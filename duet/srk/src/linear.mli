(** Various operations for the vector space [int -> QQ] *)

open Syntax

(** Raised for unsolvable systems of linear equations *)
exception No_solution

exception Nonlinear

(** Sparse vector with integer entries. *)
module ZZVector : sig
  include Ring.Vector with type dim = int and type scalar = ZZ.t
  val compare : t -> t -> int
  val pp : Format.formatter -> t -> unit
  val pp_term : (Format.formatter -> int -> unit) -> Format.formatter -> t -> unit
  val show : t -> string
  val hash : t -> int
end

(** Sparse vector with rational entries. *)
module QQVector : sig
  include Ring.Vector with type dim = int and type scalar = QQ.t
  val compare : t -> t -> int
  val pp : Format.formatter -> t -> unit
  val pp_term : (Format.formatter -> int -> unit) -> Format.formatter -> t -> unit
  val show : t -> string
  val hash : t -> int
end

(** Sparse matrix with rational entries. *)
module QQMatrix : sig
  include Ring.Matrix with type scalar = QQ.t
                       and type vector = QQVector.t

  val pp : Format.formatter -> t -> unit
  val show : t -> string

  (** Matrix exponentiation by a positive integer power. *)
  val exp : t -> int -> t

  (** Compute a list rational eigenvalues of the square sub-matrix
     over the given dimensions, along with their algebraic
     multiplicities. *)
  val rational_eigenvalues : t -> int list -> (QQ.t * int) list
end

(** [nullspace mat dimensions] computes a basis for the vector space [{ x :
    mat*x = 0}], projected on to the set of dimensions [dimensions].  (Note
    that the nullspace is not finitely generated in [int -> QQ], hence the
    projection). *)
val nullspace : QQMatrix.t -> int list -> QQVector.t list

(** [solve_exn mat b] computes a rational vector [x] such that [mat*x =
    b]. Raises [No_solution] if there is no solution. *)
val solve_exn : QQMatrix.t -> QQVector.t -> QQVector.t

(** [solve mat b] computes a rational vector [x] such that [mat*x = b], if
    such a vector exists.  Otherwise, return [None]. *)
val solve : QQMatrix.t -> QQVector.t -> QQVector.t option

(** Given a predicate on dimensions and a list of terms (all implicitly equal
    to zero), orient the equations as rewrite rules that eliminate dimensions
    that don't satisfy the predicate. *)
val orient : (int -> bool) -> QQVector.t list -> (int * QQVector.t) list

(** [vector_right_mul m v] computes [m*v] *)
val vector_right_mul : QQMatrix.t -> QQVector.t -> QQVector.t

(** [vector_left_mul v m] computes [(v^t)*m] *)
val vector_left_mul : QQVector.t -> QQMatrix.t -> QQVector.t

(** Given two matrices [A] and [B], compute matrices [C] and [D] such that [CA
    = DB] is a basis for the intersection of the rowspaces of [A] and [B]. *)
val intersect_rowspace : QQMatrix.t -> QQMatrix.t -> (QQMatrix.t * QQMatrix.t)

(** Given two matrices [A] and [B], compute matrices [C] and [D] such
   that [CA = DB], and for any [E] and [F] such that [EA = FB], there
   exists a (unique) [U] such that [UCA = EA (= UDB = FB)] *)
val pushout : QQMatrix.t -> QQMatrix.t -> (QQMatrix.t * QQMatrix.t)

(** Given two matrices A and B, compute a matrix C such that CB = A (if one
    exists).  C exists when the rowspace of B is contained in the rowspace of
    A.  If A and B are invertible, then C is exactly AB{^-1}. *)
val divide_right : QQMatrix.t -> QQMatrix.t -> QQMatrix.t option

(** Given two matrices A and B, compute a matrix C such that BC = A (if one
    exists).  C exists when the columnspace of B is contained in the columnspace of
    A.  If A and B are invertible, then C is exactly B{^-1}A. *)
val divide_left : QQMatrix.t -> QQMatrix.t -> QQMatrix.t option

(** Given matrices [A] and [B] representing a system of equations [Ax' = Bx],
    find a matrix [T] and a square matrix [M] such that [y' = My] is the
    greatest linear dynamical system that approximates [Ax' = Bx], and [T] is
    the linear transformation into the linear dynamical system.  That is, [TB
    = MTA], and the rowspace of [TA] is maximal. *)
val max_lds : QQMatrix.t -> QQMatrix.t -> QQMatrix.t * QQMatrix.t

(** Given a matrix [A], find a pair of matrices [(M,T)] such that [MA = TM],
    [T] is lower-triangular, and the rowspace of [MA] is maximal. *)
val rational_triangulation : QQMatrix.t -> (QQMatrix.t * QQMatrix.t)

(** Compute a list of (rational eigenvalue, generalized eigenvector)
   pairs of a finite dimensional square submatrix of a given
   matrix. The submatrix is determined by the list of dimensions given
   as the second parameter. *)
val rational_spectral_decomposition : QQMatrix.t -> int list -> (QQ.t * QQVector.t) list

(** Compute a list of (period, rational eigenvalue, generalized
   eigenvector) triples of a finite dimensional square submatrix of a
   given matrix. The submatrix is determined by the list of dimensions
   given as the second parameter. *)
val periodic_rational_spectral_decomposition : QQMatrix.t ->
  int list ->
  (int * QQ.t * QQVector.t) list

(** [jordan_chain mA lambda v] computes the (left) Jordan chain generated by
   [v] corresponding to the eigenvalue [lambda] of [mA].  This is a
   list of vectors v{_ 0},...,v{_ n} such that
   {ul {- v{_ 0} = v }
       {- for each i < n we have v{_ i}(mA-lambda*I) = v{_ {i+1}} }
       {- v{_ n}(mA-lambda*I) = 0 } }
 *)
val jordan_chain : QQMatrix.t -> QQ.t -> QQVector.t -> QQVector.t list

(** {2 Rational vector spaces} *)
module QQVectorSpace : sig
  (** Vector spaces are represented by a list of basis vectors *)
  type t = QQVector.t list

  val equal : t -> t -> bool

  (** [subspace a b] checks whether [a] is a subspace of [b] *)
  val subspace : t -> t -> bool

  (** Empty vector space *)
  val empty : t

  (** Check whether a given vector space is empty. *)
  val is_empty : t -> bool

  (** Create a matrix whose rows form a basis for the space *)
  val matrix_of : t -> QQMatrix.t

  (** Given a matrix *with linearly independent rows*, get basis for its rowspace *)
  val of_matrix : QQMatrix.t -> t

  (** Given a vector space V and a vector v, does v belong to V *)
  val mem : t -> QQVector.t -> bool

  (** Intersect two vector spaces *)
  val intersect : t -> t -> t

  (** Given vector spaces U and V, compute a basis for the direct sum
      [{ u+v : u in U, v in V }] *)
  val sum : t -> t -> t

  (** Given vector spaces U and V, compute a basis for a vector space W such that
      [sum (diff U V) W = U].  NOTE: this vector space is not unique. *)
  val diff : t -> t -> t

  (** [standard_basis n] returns standard basis for n-dimensional space *)
  val standard_basis : int -> t

  (** Compute a basis for the vector space spanned by a given set of vectors *)
  val basis : QQVector.t list -> t

  (** Simplify a basis using Gauss-Jordan elimination *)
  val simplify : t -> t

  (** Find the dimension of the given vector space *)
  val dimension : t -> int
end

(** {2 Partial linear maps} *)
module PartialLinearMap : sig

  (** A partial linear map is a partial function that is linear and
     whose domain is a vector space *)
  type t

  val equal : t -> t -> bool

  (** [identity n] is the (total) identity map on the coordinates [0
     .. n-1]; all other coordinates are sent to 0. *)
  val identity : int -> t


  (** [make A G] creates the partial map whose action is given by the
     matrix A, and whose domain of definition is the set of all [v]
     such that [g.v = 0] for all [g in G].  (I.e., [G] is a set of
     equality constraints that must on the input for [make A G] to be
     defined; the domain is the orthogonal complement of G). *)
  val make : QQMatrix.t -> QQVector.t list -> t

  val pp : Format.formatter -> t -> unit

  (** (Partial) map composition *)
  val compose : t -> t -> t

  (** Repeatly compose a partial map with itself until the domain
     stabilizes; return both iteration sequence and the invariant guard
     of the function.

     We have guard(f) <= guard(f o f) <= guard(f o f o f) <= ...
     and there is some (least) n such that guard(f^n) = guard(f^m)
     for all m >= n.  The pair returned by [iteration_sequence f] is
     [([f; f o f; ...; f^n], dom(f^n))].
  *)
  val iteration_sequence : t -> (t list) * (QQVector.t list)

  (** Access the underlying (total) map of a partial map. *)
  val map : t -> QQMatrix.t

  (** Access the guard of a partial map.  The domain of the map is its
     orthogonal complement *)
  val guard : t -> QQVectorSpace.t

  (** Given matrices [A] and [B] representing a system of equations
     [Ax' = Bx], find a matrix [S] and a deterministic linear transition
     system (DLTS) [f] such that:
      1. [Ax' = Bx] implies [Sx' = f(Sx)]
      2. if [g] is a DLTS and [T] is a matrix such that [Ax' = Bx]
     implies [Tx' = g(Tx)], then there is some [U] such that [T = US]
     and [y' = f(y)] implies [Uy' = g(Uy)].  *)
  val max_dlts : QQMatrix.t -> QQMatrix.t -> QQMatrix.t * t
end

(** {2 Affine terms} *)

(** Various operations for manipulating affine terms over symbols, represented
    as rational vectors *)

(** Map a symbol to a dimension.  The following equations hold:
    - [sym_of_dim (dim_of_sym sym) = Some sym]
    - [sym_of_dim const_dim = None] *)
val sym_of_dim : int -> symbol option

(** Map a dimension to symbol.  The following equations hold:
    - [sym_of_dim (dim_of_sym sym) = Some sym]
    - [sym_of_dim const_dim = None] *)
val dim_of_sym : symbol -> int

(** Dimension for representing the coefficient of the constant 1. *)
val const_dim : int

(** Representation of a rational number as an affine term.  The equation
    [const_of_linterm (const_linterm qq) = Some qq] must hold. *)
val const_linterm : QQ.t -> QQVector.t

(** Convert an affine term to a rational number, if possible.  The equation
    [const_of_linterm (const_linterm qq) = Some qq] must hold. *)
val const_of_linterm : QQVector.t -> QQ.t option

(** Convert a rational vector representing an affine term.  Raises [Nonlinear]
    if the input term is non-linear. *)
val linterm_of : 'a context -> 'a term -> QQVector.t

(** Convert a rational vector to an affine term.  The equation [of_linterm srk
    (linterm_of srk t) = t] must hold. *)
val of_linterm : 'a context -> QQVector.t -> 'a term

(** Pretty-print an affine term *)
val pp_linterm : 'a context -> Format.formatter -> QQVector.t -> unit

(** [evaluate_linterm env t] evaluates the affine term t in the environment
    [env] *)
val evaluate_linterm : (symbol -> QQ.t) -> QQVector.t -> QQ.t

(** Count the number of dimensions with non-zero coefficients *)
val linterm_size : QQVector.t -> int

(** [term_of_vec srk t vec] creates a term representation of [vec] by
   interpreting each dimension i as the term [t i]. *)
val term_of_vec : ('a context) -> (int -> 'a term) -> QQVector.t -> 'a term

(** Evaluate a vector by interpreting each coordinate according to the
   given interpretation (the interpretation of [const_dim] is fixed to
   be 1).  *)
val evaluate_affine : (int -> QQ.t) -> QQVector.t -> QQ.t
