
all: generator

LIBS = -lm -lrt -lpthread -lstdc++ -ldl -lnsl

ifndef CCC
 CCC = g++
endif

LD = $(CCC)


SOURCES := Generator.cc ProGenParser.cc NDDLProGenTranslator.cc
OBJECTS := $(SOURCES:.cc=.o)

$(OBJECTS): %.o : %.cc
	$(CCC) $(CFLAGS) -c $< -o $@

generator: $(OBJECTS)
	$(CCC)  $(OBJECTS) $(LIBS) -o $@

clean:
	rm *.o generator*
