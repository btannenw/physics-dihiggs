# Makefile to compile histMaker code
# G. Facini
# Wed Jul 18 11:21:03 CEST 2012
#
# S.Hageboeck May 03 2016:
# Add automatic dependency tracking for all object files, simplify
#

SDIR=src
ODIR=obj
LDIR=lib
$(shell mkdir -p $(ODIR) >/dev/null)
$(shell mkdir -p $(LDIR) >/dev/null)

SOLIBNAME  := PlotUtils
SOLIB = $(LDIR)/lib$(SOLIBNAME).so

cpputilsdir = /afs/cern.ch/work/n/nmorange/public/cpp-root-utils/
curdir = $(shell pwd -P)
LINKFLAGS = $(shell root-config --libs) -L. -Wl,-R -Wl,. -Wl,-R -Wl,$(curdir) -L$(cpputilsdir)/lib -lUtils -Wl,-R -Wl,$(cpputilsdir)/lib

INCLUDE = -I./include/ -I./transform/
CXX = $(shell root-config --cxx)
CXXFLAGS  = $(shell root-config --cflags) $(INCLUDE) -fPIC -std=c++11
CXXFLAGS += -Wall -Wextra -Wno-unused-parameter
CXXFLAGS += -O2 -ggdb

LD  = $(shell root-config --ld)
LDFLAGS = -shared -Wl,-soname,lib$(SOLIBNAME).so,--no-undefined $(shell root-config --ldflags) $(LIBS)
LIBS=-L./$(LDIR) $(shell root-config --libs) -lEG -lMinuit -lRooFit -lRooFitCore -lRooStats -lHistFactory

#Auto-dependencies:
# Include $(DEPFLAGS) in all commands that compile something
# This both compiles and makes dependencies at the same time
# Execute $(DEPPOST) after the compile command to replace the old dependency file with the new one.
# This prevents broken depfiles in case compilation fails.
DEPDIR = .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d.tmp
DEPPOST = @mv -f $(DEPDIR)/$*.d.tmp $(DEPDIR)/$*.d
$(shell mkdir -p $(DEPDIR) >/dev/null)

LINK_DEF  = $(SDIR)/Linkdef.h
DICT_BASE = $(ODIR)/PlotUtils
DICT_CINT = $(DICT_BASE).cpp

m_sOBJ = splitter.o splitter_Run2.o SplitInputs.o
sOBJ = $(patsubst %,$(ODIR)/%,$(m_sOBJ))
m_wOBJ = MakeWorkspace.o TMsgLogger.o engine.o sample.o samplehandler.o category.o categoryhandler.o sampleincategory.o
m_wOBJ += configuration.o inputshandler.o inputshandler_paper.o properties.o binning.o HistoTransform.o systematic.o systematichandler.o
m_wOBJ += finder.o outputhandler.o plotting.o analysis.o analysishandler.o systematiclistsbuilder.o systematiclistsbuilder_vhbb.o
m_wOBJ += regiontracker.o regiontracker_vhbb.o regiontracker_azh.o regiontracker_vhbbrun2.o regionnamesparser.o regionnamesparser_vhbb.o samplesbuilder.o samplesbuilder_vhbb.o
m_wOBJ += binning_vhbb.o regionnamesparser_run2.o inputshandler_run2.o samplesbuilder_vhres.o systematiclistsbuilder_vhres.o
m_wOBJ += samplesbuilder_htt.o systematiclistsbuilder_htt.o binning_htt.o
m_wOBJ += binning_vhres.o samplesbuilder_monoh.o systematiclistsbuilder_monoh.o binning_monoh.o regiontracker_monoh.o
m_wOBJ += samplesbuilder_hh.o systematiclistsbuilder_hh.o binning_hh.o regionnamesparser_vhres.o
m_wOBJ += samplesbuilder_hhwwbb.o systematiclistsbuilder_hhwwbb.o binning_hhwwbb.o 
m_wOBJ += samplesbuilder_hvt.o systematiclistsbuilder_hvt.o binning_hvt.o samplesbuilder_azh.o systematiclistsbuilder_azh.o binning_azh.o
m_wOBJ += samplesbuilder_vhbbrun2.o systematiclistsbuilder_vhbbrun2.o binning_vhbbrun2.o
m_wOBJ += regiontracker_vbfgamma.o samplesbuilder_vbfgamma.o binning_vbfgamma.o systematiclistsbuilder_vbfgamma.o regionnamesparser_vbfgamma.o inputshandler_vbfgamma.o
wOBJ = $(patsubst %,$(ODIR)/%,$(m_wOBJ))

# header files for which we want to create a dictionnary
m_nHEADER = plotUtils.hpp roofitUtils.hpp RooExpandedFitResult.h utility.hpp
m_nOBJtmp = $(patsubst %.hpp,%.o,$(m_nHEADER))
m_nOBJ = $(patsubst %.h,%.o,$(m_nOBJtmp))
nOBJ = $(patsubst %,$(ODIR)/%,$(m_nOBJ)) $(DICT_BASE).o

all : SplitInputs MakeWorkspace addData $(SOLIB) scripts

############################################
#The executables with explicit dependencies#
############################################
SplitInputs: $(sOBJ) $(SOLIB)
	$(CXX) -o $@ $(sOBJ) $(LIBS) -l$(SOLIBNAME) $(CXXFLAGS)

MakeWorkspace: $(wOBJ) $(SOLIB)
	$(CXX) -o $@ $(wOBJ) $(LIBS) -l$(SOLIBNAME) $(CXXFLAGS)

addData: addData.cxx
	$(CXX) $(CXXFLAGS) -I$(cpputilsdir)/include  $(LINKFLAGS) -o $@ $<

############################################
# Object files                             #
############################################
$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPDIR)/%.d
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<
	$(DEPPOST)
#For dicts
$(ODIR)/%.o: $(ODIR)/%.cpp $(DEPDIR)/%.d
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<
	$(DEPPOST)
#For executables
$(ODIR)/%.o: %.C $(DEPDIR)/%.d
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<
	$(DEPPOST)
#For the transform package
$(ODIR)/%.o: transform/%.C $(DEPDIR)/%.d
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<
	$(DEPPOST)


scripts: FitCrossCheckForLimits.C runAsymptoticsCLs.C newGetMuHat.C runSig.C
	root -b -q -l compile.C

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(ODIR)/*.d $(ODIR)/*Dict* core* SplitInputs MakeWorkspace $(LDIR)/* *_C.so addData $(DEPDIR)/*

$(SOLIB): $(nOBJ)
	$(LD) $^ -o $@ $(LDFLAGS)
	cp $(SOLIBNAME)*.pcm lib/

$(DICT_CINT) : $(LINK_DEF) $(DEPDIR)/Linkdef.d
	@echo Making $@
	$(ROOTSYS)/bin/rootcint -f $(DICT_CINT) -rml $(SOLIB) -s $(SOLIBNAME) $(INCLUDE) $(m_nHEADER) $(LINK_DEF)

$(DEPDIR)/Linkdef.d : $(LINK_DEF)
	set -e; rm -f $@; mkdir -p $(DEPDIR) ; \
	$(CXX) $(CXXFLAGS) -MM $< > $@.$$$$ ; \
	sed 's,Linkdef\.o[ :]*,$(DEPDIR)/Linkdef.d $(DICT_CINT) : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$



#Some make magic for the auto dependencies:
# - Prevent make from failing if the dependency file doesn't exist yet
$(DEPDIR)/%.d: ;
# - Make make keep depfiles
.PRECIOUS: $(DEPDIR)/%.d
# - Include all dependencies that are already known
-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(m_wOBJ)))
-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(m_nOBJ)))
-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(m_sOBJ)))
-include $(DEPDIR)/Linkdef.d
# - Prevent make from trying a predefined rule
%.o: %.cpp
