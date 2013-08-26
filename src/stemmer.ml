
module Porter2 = struct

type t

external create : unit -> t = "caml_porter2_create"
external stem : t -> string -> string = "caml_porter2_stem"
external finish : t -> unit = "caml_porter2_finish"

end
