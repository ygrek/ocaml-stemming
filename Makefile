
.PHONY: build clean install remove

build:
	ocamlbuild stemming.cma stemming.cmxa

install: build
	ocamlfind install stemming META $(wildcard _build/stemming.cma _build/stemming.cmxa _build/porter2.cmi _build/porter2.cmx _build/*.a _build/*.so _build/*.lib _build/*.dll)

uninstall:
	ocamlfind remove stemming

clean:
	ocamlbuild -clean
