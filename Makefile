CXX       = g++
BINDIR    = bin
INCDIR    = include
OBJDIR    = obj
SRCDIR    = src
LIBDIR    = /home/abetts/programs/rose-0.9.5a-13059_inst/lib
BOOST_DIR = /home/abetts/programs/boost_1_45_0_inst
ROSE_DIR  = /home/abetts/programs/rose-0.9.5a-13219_inst
LDFLAGS   = -s -L$(ROSE_DIR)/lib -lrose
CXXFLAGS  = -I$(BOOST_DIR)/include -I$(ROSE_DIR)/include -I$(INCDIR) -O3 -Wall

OUT = $(BINDIR)/translator

_OBJS = CommandLine.o \
	Debug.o \
	OpParLoop.o \
	Translator.o

OBJS  = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(wildcard $(INCDIR)/*.h)
	@echo "\n===== BUILDING OBJECT FILE $@ ====="
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(OBJS) 
	@echo "\n===== LINKING $@ ====="
	$(CXX) $(LDFLAGS) $(OBJS) -o $(OUT) 

.PHONY: all clean relink
all: $(OUT) 

relink:
	rm -f $(BINDIR)/*
	@echo "\n===== LINKING $@ ====="
	$(CXX) $(LDFLAGS) $(OBJS) -o $(OUT)

clean:
	rm -f $(BINDIR)/* $(OBJDIR)/* *~

