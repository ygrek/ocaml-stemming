open Ocamlbuild_plugin

;;

dispatch begin function
| After_rules ->

     ocaml_lib "stemming";
     flag ["link"; "ocaml"; "library"] (S[A"-cclib"; A "-lstemming_stubs"]);
     flag ["link"; "ocaml"; "library"; "byte"] (S[A"-dllib"; A"-lstemming_stubs"]);
     dep  ["link"; "ocaml"; "library"] ["libstemming_stubs." ^ !Options.ext_lib];
     dep  ["c"; "compile" ] ["porter2_c.h"];

| _ -> ()
end

