type t = Mpzf.t

let pp = Mpzf.print
let show = SrkUtil.mk_show pp

let hash x = Hashtbl.hash (Mpzf.to_string x)

let compare = Mpzf.cmp
let equal x y = compare x y = 0
let leq x y = compare x y <= 0
let lt x y = compare x y < 0

let add = Mpzf.add
let sub = Mpzf.sub
let mul = Mpzf.mul
let negate = Mpzf.neg
let one = Mpzf.of_int 1
let zero = Mpzf.of_int 0

let modulo = Mpzf.fdiv_r
let div = Mpzf.fdiv_q

let gcd = Mpzf.gcd
let lcm = Mpzf.lcm

let of_int = Mpzf.of_int
let of_string = Mpzf.of_string

let to_int x =
  if Mpz.fits_int_p x then Some (Mpz.get_int x) else None

let min x y = if leq x y then x else y
let max x y = if leq y x then x else y
let abs x =
  if lt x zero then negate x
  else x
