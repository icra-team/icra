SETUP = ocaml setup.ml

all: build

.PHONY: build duet srk apak patools test cca

build: setup.ml setup.data
	$(SETUP) -build

duet: setup.ml setup.data duet/config.ml
	ocamlbuild duet/duet.native

newton:
	ocamlbuild duet/duet.cmx duet/newton_interface.cmx duet/InterIR.cmx duet/duet.native
        # -verbose to the ocamlfind command for debugging
	cd _build/duet && ocamlfind ocamlopt -output-obj -g -linkpkg -package camlidl -package Z3 -package ppx_deriving -package batteries -package apron.polkaMPQ -package apron.boxMPQ -package apron.octMPQ -package ocamlgraph -package cil -package cil.default-features -package ocrs -package ntl -o libduet.so ../srk/src/srk.cmx ../apak/apak.cmx core.cmx afg.cmx ast.cmx hlir.cmx report.cmx cfgIr.cmx cmdLine.cmx pointerAnalysis.cmx call.cmx inline.cmx config.cmx datalog.cmx bddpa.cmx ai.cmx solve.cmx interproc.cmx cra.cmx translateCil.cmx cbpAst.cmx cbpLex.cmx cbpParse.cmx translateCbp.cmx InterIR.cmx conversion.cmx newtonDomain.cmx newton_interface.cmx safety.cmx duet.cmx || exit 1

srk: setup.ml setup.data
	ocamlbuild srk/src/test_srk.native -tag debug

apak: setup.ml setup.data
	ocamlbuild apak/test_apak.native -tag debug

patools: setup.ml setup.data
	ocamlbuild patools/patools.native -tag debug

setup.ml: _oasis
	oasis setup

setup.data: setup.ml
	$(SETUP) -configure

install:
	$(SETUP) -install

clean:
	$(SETUP) -clean
	@rm -f setup.data

test:
	$(SETUP) -test

uninstall:
	$(SETUP) -uninstall

reinstall:
	$(SETUP) -reinstall

doc: setup.ml setup.data
	$(SETUP) -doc
