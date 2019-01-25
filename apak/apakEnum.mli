val pp_print_enum : ?indent:int ->
  ?pp_sep:(Format.formatter -> unit -> unit) ->
  (Format.formatter -> 'a -> unit) ->
  Format.formatter ->
  'a BatEnum.t ->
  unit
val pp_print_enum_nobox : ?pp_sep:(Format.formatter -> unit -> unit) ->
  (Format.formatter -> 'a -> unit) ->
  Format.formatter ->
  'a BatEnum.t ->
  unit
val adjacent_pairs : 'a BatEnum.t -> ('a * 'a) BatEnum.t
val cartesian_product : 'a BatEnum.t -> 'b BatEnum.t -> ('a * 'b) BatEnum.t
val tuples : ('a BatEnum.t) list -> ('a list) BatEnum.t

(** Given an enumerateion a0, a1, a2, a3,  ..., enumerate the pairs
    (a0, a1), (a0, a2), ..., (a1, a2), (a1, a3), ... *)
val distinct_pairs : 'a BatEnum.t -> ('a * 'a) BatEnum.t
