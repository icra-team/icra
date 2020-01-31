BOOST_PATH = /unsup/boost-1.55.0/amd64_rhel6
DUET_ROOT = duet
WALI_ROOT = WALi-OpenNWA
DOMAINS = $(WALI_ROOT)/AddOns/Domains/Source/wali/domains
DOMAINS_FILES = $(DOMAINS)/duet/DuetRel.cpp $(DOMAINS)/duet/DuetRel.hpp $(DOMAINS)/nparel/NPARel.cpp $(DOMAINS)/nparel/NPARel.hpp
DOMAINS_SO = $(WALI_ROOT)/lib64/libwalidomains.so
WALI_SO = $(WALI_ROOT)/lib64/libwali.so
MAKE_WALI = cd $(WALI_ROOT) && scons strong_warnings=0 addons -Q
BUILD = _build
SOURCE = src
DUET_SO = $(DUET_ROOT)/_build/duet/libduet.native.so
CXXFLAGS = -c -O0 -Wall -g -Wextra -Wformat=2 -Winit-self -Wfloat-equal -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion -Woverloaded-virtual -fdiagnostics-show-option -DBOOST_NO_DEFAULTED_FUNCTIONS=1 -DCHECKED_LEVEL=2 -DEXPORT_GTR_SYMBOLS=0 -DPRATHMEHS_NWA_DETENSOR=0 -DREGEXP_TEST=1 -DUSE_DUET=1 -D_GLIBCXX_DEBUG=1 -I$(WALI_ROOT)/Source -I$(BOOST_PATH)/include -I$(SOURCE) -I$(WALI_ROOT)/AddOns/Domains/Source -I$(WALI_ROOT)/AddOns/Domains/ThirdParty/include -I"`ocamlc -where`"
ICRA_BINARY = icra

.PHONY: wali duet clean veryclean
.DEFAULT_GOAL := icra

$(ICRA_BINARY): $(DOMAINS_SO) $(WALI_SO) $(DUET_SO) $(BUILD)/icra.o $(BUILD)/icra_callbacks.o $(BUILD)/ire.o $(BUILD)/ire_callbacks.o 
	g++ -g -o icra -Wl,-rpath=\$$ORIGIN/$(BOOST_PATH)/lib -Wl,-rpath=\$$ORIGIN/$(DUET_ROOT)/_build/duet -Wl,-rpath=\$$ORIGIN/$(DUET_ROOT)/_build/src/duet -Wl,-rpath=\$$ORIGIN/$(WALI_ROOT) -Wl,-rpath=\$$ORIGIN/$(WALI_ROOT)/lib64 -Wl,--start-group $(BUILD)/icra.o $(BUILD)/ire_callbacks.o $(BUILD)/ire.o $(BUILD)/icra_callbacks.o -L$(BOOST_PATH)/lib -L$(WALI_ROOT)/lib64 -L$(BUILD) -L"`ocamlc -where`" -L$(DUET_ROOT)/_build/duet -L$(WALI_ROOT) -lrt -lduet.native -lwali -lwalidomains -lglog -Wl,--end-group
	@echo " **** ICRA build completed successfully **** "

$(shell mkdir -p $(BUILD))

clean:
	-rm $(ICRA_BINARY)
	-rm -rf $(BUILD)
	-rm -rf $(DUET_ROOT)/_build
	-cd $(DUET_ROOT) && ocaml setup.ml -clean
	-rm -rf $(WALI_ROOT)/_build
	-rm -rf $(WALI_ROOT)/lib64

veryclean:
	-rm $(ICRA_BINARY)
	-rm -rf $(BUILD)
	-rm -rf $(DUET_ROOT)/_build
	-cd $(DUET_ROOT) && ocaml setup.ml -clean
	-cd $(WALI_ROOT) && scons -c

$(BUILD)/icra.o: $(SOURCE)/icra.cpp $(SOURCE)/icra.hpp $(DOMAINS_FILES)
	g++ -o $(BUILD)/icra.o $(CXXFLAGS) src/icra.cpp

$(BUILD)/icra_callbacks.o: $(SOURCE)/icra_callbacks.cpp $(SOURCE)/icra_callbacks.hpp
	g++ -o $(BUILD)/icra_callbacks.o $(CXXFLAGS) $(SOURCE)/icra_callbacks.cpp

$(BUILD)/ire.o: $(SOURCE)/ire.cpp $(SOURCE)/ire.hpp $(DOMAINS_FILES)
	g++ -o $(BUILD)/ire.o $(CXXFLAGS) src/ire.cpp

$(BUILD)/ire_callbacks.o: $(SOURCE)/ire_callbacks.cpp $(SOURCE)/ire_callbacks.hpp $(DOMAINS_FILES)
	g++ -o $(BUILD)/ire_callbacks.o $(CXXFLAGS) src/ire_callbacks.cpp

all: duet wali icra

$(DOMAINS_SO): $(DOMAINS_FILES)
	$(MAKE_WALI)

$(WALI_SO):
	$(MAKE_WALI)

wali:
	$(MAKE_WALI)

$(DUET_SO):
	cd $(DUET_ROOT) && $(MAKE) newton

duet:
	-rm $(DUET_SO)
	$(MAKE) $(DUET_SO)
