(**
 Collection of stemmers
*)

(**
  Stemming algorithm for the English language

  @see <http://snowball.tartarus.org/algorithms/english/stemmer.html> web
*)
module Porter2 = struct

type t

external init : unit -> t = "caml_stemmer_porter2_init"
external stem : t -> string -> string = "caml_stemmer_porter2_stem"
external close : t -> unit = "caml_stemmer_porter2_close"

end

(**
  Stemming algorithm for Russian and Ukrainian languages by Keva,
  expects CP-1251 encoded input.

  @see <http://www.keva.ru/> web
*)
module Keva = struct

type t

external init : unit -> t = "caml_stemmer_keva_init"
external stem : t -> string -> string = "caml_stemmer_keva_stem"
external close : t -> unit = "caml_stemmer_keva_close"

end
