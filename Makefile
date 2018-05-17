BOOST_PATH = /unsup/boost-1.55.0/amd64_rhel6
DUET_ROOT = duet
WALI_ROOT = WALi-OpenNWA
DOMAINS = $(WALI_ROOT)/AddOns/Domains/Source/wali/domains
DOMAINS_FILES = $(DOMAINS)/duet/DuetRel.cpp $(DOMAINS)/duet/DuetRel.hpp $(DOMAINS)/nparel/NPARel.cpp $(DOMAINS)/nparel/NPARel.hpp
DOMAINS_SO = $(WALI_ROOT)/lib64/libwalidomains.so
WALI_SO = $(WALI_ROOT)/lib64/libwali.so
MAKE_WALI = cd $(WALI_ROOT) && scons addons -Q
BUILD = _build
SOURCE = src
DUET_SO = $(DUET_ROOT)/_build/duet/libduet.so

.PHONY: wali duet
.DEFAULT_GOAL := icra


icra: $(DOMAINS_SO) $(WALI_SO) $(DUET_SO) $(BUILD)/icra.o $(BUILD)/libocamlinterface.so $(BUILD)/IRE_callbacks.o $(BUILD)/IRE.o
	g++ -g -o icra -Wl,-rpath=$(BOOST_PATH)/lib -Wl,-rpath=$(DUET_ROOT)/_build/duet -Wl,-rpath=$(DUET_ROOT)/_build/src/duet -Wl,-rpath=$(WALI_ROOT) -Wl,-rpath=$(WALI_ROOT)/lib64 -Wl,--start-group $(BUILD)/icra.o $(BUILD)/IRE_callbacks.o $(BUILD)/IRE.o $(BUILD)/newton_interface.o -L$(BOOST_PATH)/lib -L$(WALI_ROOT)/lib64 -L$(BUILD) -L"`ocamlc -where`" -L$(DUET_ROOT)/_build/duet -L$(WALI_ROOT) -lrt -lduet -lwali -lwalidomains -lglog -Wl,--end-group
#	g++ -g -o icra -Wl,-rpath=$(BOOST_PATH)/lib -Wl,-rpath=$(DUET_ROOT)/_build/duet -Wl,-rpath=$(DUET_ROOT)/_build/src/duet -Wl,-rpath=$(WALI_ROOT) -Wl,-rpath=$(WALI_ROOT)/lib64 -Wl,--start-group $(BUILD)/icra.o $(BUILD)/IRE_callbacks.o $(BUILD)/IRE.o -L$(BOOST_PATH)/lib -L$(WALI_ROOT)/lib64 -L$(BUILD) -L"`ocamlc -where`" -L$(DUET_ROOT)/_build/duet -L$(WALI_ROOT) -lrt -lduet -locamlinterface -lwali -lwalidomains -lglog -Wl,--end-group
#	g++ -g -o icra -Wl,-rpath=$(BOOST_PATH)/lib -Wl,-rpath=$(DUET_ROOT)/_build/duet -Wl,-rpath=$(DUET_ROOT)/_build/src/duet -Wl,-rpath=$(WALI_ROOT) -Wl,-rpath=$(WALI_ROOT)/lib64 -Wl,--start-group $(BUILD)/icra.o -L$(BOOST_PATH)/lib -L$(WALI_ROOT)/lib64 -L$(BUILD) -L"`ocamlc -where`" -L$(DUET_ROOT)/_build/duet -L$(WALI_ROOT) -lboost_filesystem -lboost_system -lboost_serialization -lrt -lduet -locamlinterface -lwali -lbpparser -lwalidomains -lglog -lbdd -Wl,--end-group
# some defunct options are: -Wl,-rpath=$TSLRE_LIB_PATH -L$TSLRE_LIB_PATH  -ltslre 

$(shell mkdir -p $(BUILD))

$(BUILD)/newton_interface.o: $(SOURCE)/NewtonOcamlInterface.cpp $(SOURCE)/NewtonOcamlInterface.hpp
	g++ -o $(BUILD)/newton_interface.o -c -Wall -g -O0 -Wextra -fdiagnostics-show-option -fPIC -DBOOST_NO_DEFAULTED_FUNCTIONS=1 -DCHECKED_LEVEL=1 -DEXPORT_GTR_SYMBOLS=0 -DPRATHMEHS_NWA_DETENSOR=0 -DREGEXP_TEST=1 -DUSE_DUET=1 -I$(WALI_ROOT)/Source -I$(BOOST_PATH)/include -I$(WALI_ROOT)/AddOns/Domains/ThirdParty/include -I$(WALI_ROOT)/AddOns/Domains/Source -IThirdParty/include -I"`ocamlc -where`" $(SOURCE)/NewtonOcamlInterface.cpp
# some defunct options are: -DTSL_DETENSOR=1 -D__TSL_REGEXP=1 

$(BUILD)/libocamlinterface.so: $(BUILD)/newton_interface.o
	g++ -o $(BUILD)/libocamlinterface.so -rdynamic -shared -Wl,-rpath=$(BOOST_PATH)/lib $(BUILD)/newton_interface.o -L$(BOOST_PATH)/lib -lrt
#	g++ -o $(BUILD)/libocamlinterface.so -rdynamic -shared -Wl,-rpath=$(BOOST_PATH)/lib $(BUILD)/newton_interface.o -L$(BOOST_PATH)/lib -lboost_filesystem -lboost_system -lboost_serialization -lrt
# some defunct options are: -Wl,-rpath=/home/turetsky/OCamlTest -L/home/turetsky/OCamlTest 

$(BUILD)/icra.o: $(SOURCE)/icra.cpp $(SOURCE)/icra.hpp $(DOMAINS_FILES)
	g++ -o $(BUILD)/icra.o -c -O0 -Wall -g -Wextra -Wformat=2 -Winit-self -Wfloat-equal -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion -Woverloaded-virtual -fdiagnostics-show-option -DBOOST_NO_DEFAULTED_FUNCTIONS=1 -DCHECKED_LEVEL=2 -DEXPORT_GTR_SYMBOLS=0 -DPRATHMEHS_NWA_DETENSOR=0 -DREGEXP_TEST=1 -DUSE_DUET=1 -D_GLIBCXX_DEBUG=1 -I$(WALI_ROOT)/Source -I$(BOOST_PATH)/include -I$(SOURCE) -I$(WALI_ROOT)/AddOns/Domains/Source -I$(WALI_ROOT)/AddOns/Domains/ThirdParty/include -I"`ocamlc -where`" src/icra.cpp
#some defunct options are: -DTSL_DETENSOR=1 -D__TSL_REGEXP=1 -I$TSLRE_INCLUDE_PATH 

$(BUILD)/IRE_callbacks.o: $(SOURCE)/IRE_callbacks.cpp $(SOURCE)/IRE_callbacks.hpp $(DOMAINS_FILES)
	g++ -o $(BUILD)/IRE_callbacks.o -c -O0 -Wall -g -Wextra -Wformat=2 -Winit-self -Wfloat-equal -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion -Woverloaded-virtual -fdiagnostics-show-option -DBOOST_NO_DEFAULTED_FUNCTIONS=1 -DCHECKED_LEVEL=2 -DEXPORT_GTR_SYMBOLS=0 -DPRATHMEHS_NWA_DETENSOR=0 -DREGEXP_TEST=1 -DUSE_DUET=1 -D_GLIBCXX_DEBUG=1 -I$(WALI_ROOT)/Source -I$(BOOST_PATH)/include -I$(SOURCE) -I$(WALI_ROOT)/AddOns/Domains/Source -I$(WALI_ROOT)/AddOns/Domains/ThirdParty/include -I"`ocamlc -where`" src/IRE_callbacks.cpp
# some defunct options are: -DTSL_DETENSOR=1 -D__TSL_REGEXP=1 -I$TSLRE_INCLUDE_PATH 

$(BUILD)/IRE.o: $(SOURCE)/IRE.cpp $(SOURCE)/IRE.hpp $(DOMAINS_FILES)
	g++ -o $(BUILD)/IRE.o -c -O0 -Wall -g -Wextra -Wformat=2 -Winit-self -Wfloat-equal -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion -Woverloaded-virtual -fdiagnostics-show-option -DBOOST_NO_DEFAULTED_FUNCTIONS=1 -DCHECKED_LEVEL=2 -DEXPORT_GTR_SYMBOLS=0 -DPRATHMEHS_NWA_DETENSOR=0 -DREGEXP_TEST=1 -DUSE_DUET=1 -D_GLIBCXX_DEBUG=1 -I$(WALI_ROOT)/Source -I$(BOOST_PATH)/include -I$(SOURCE) -I$(WALI_ROOT)/AddOns/Domains/Source -I$(WALI_ROOT)/AddOns/Domains/ThirdParty/include -I"`ocamlc -where`" src/IRE.cpp
# some defunct options are: -DTSL_DETENSOR=1 -D__TSL_REGEXP=1 -I$TSLRE_INCLUDE_PATH 

#all: wali icra

$(DOMAINS_SO): $(DOMAINS_FILES)
	$(MAKE_WALI)

$(WALI_SO):
	$(MAKE_WALI)

wali:
	$(MAKE_WALI)

$(SOURCE)/icraRegexp.cmx: $(SOURCE)/icraRegexp.ml
	cd $(SOURCE) && ocamlopt icraRegexp.ml -c

$(DUET_SO): $(SOURCE)/icraRegexp.cmx
	cd duet && ./configure
	cd duet && ocamlbuild duet/duet.cmx duet/newton_interface.cmx duet/duet.native
	cd duet/_build/duet && ocamlfind ocamlopt -output-obj -g -linkpkg -package camlidl -package Z3 -package mathsat -package ppx_deriving -package batteries -package apron.polkaMPQ -package apron.boxMPQ -package apron.octMPQ -package ocamlgraph -package cil -package cil.default-features -package ocrs -package ntl -o libduet.so ../srk/src/srk.cmx ../apak/apak.cmx core.cmx afg.cmx ast.cmx hlir.cmx report.cmx cfgIr.cmx cmdLine.cmx pointerAnalysis.cmx call.cmx solve.cmx ai.cmx config.cmx datalog.cmx inline.cmx bddpa.cmx interproc.cmx cra.cmx translateCil.cmx cbpAst.cmx cbpLex.cmx cbpParse.cmx translateCbp.cmx conversion.cmx newtonDomain.cmx newton_interface.cmx safety.cmx duet.cmx ../../../$(SOURCE)/icraRegexp.cmx

duet: $(DUET_SO)
