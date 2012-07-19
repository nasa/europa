# insane function call which will place targets in the correct locations

findtarget = $(if $(findstring .$(LIB_EXT),$(suffix $(1))),build/lib/$(1),$(if $(findstring .jar,$(suffix $(1))),build/lib/$(1),$(if $(findstring .o,$(suffix $(1))),build/object/$(1),$(if $(findstring .exe,$(patsubst %$(RT_SUFFIX),%.exe,$(1))),build/bin/$(1),build/$(1)))))

%_wrap.cc: %.i
	$(SWIG) -java -package $(basename $<) -o $@ -c++ -outdir build/swig/$(basename $<) $<

build/lib/$(LIB_PREFIX)%$(BUILD_SUFFIX).$(LIB_EXT):
	$(CXX) $(LDFLAGS) $(SHARED_LINK_FLAG) -o $@ $+ $(LOADLIBS) $(LDLIBS)

build/object/%$(BUILD_SUFFIX).o: %.cc
	$(CXX) -c $< $(CPPFLAGS) $(CXXFLAGS) -o $@

%.xml: %.nddl
	$(JAVA) -jar $(EUROPA_HOME)/lib/nddl.jar --NddlParser $<

%.cc %.hh: %.nddl
	$(JAVA) -jar $(EUROPA_HOME)/lib/nddl.jar --noxml $<

build/bin/%$(RT_SUFFIX):
	$(CXX) -o $@ $(LDFLAGS) $^ $(LOADLIBS) $(LDLIBS)
	chmod 711 $@

Makefile : ;
%.mk :: ;
