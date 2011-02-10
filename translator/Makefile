CXX       = g++
BINDIR    = bin
INCDIR    = include
OBJDIR    = obj
SRCDIR    = src
BOOST_DIR = $(OP2_BOOST_INST_DIR)
ROSE_DIR  = $(OP2_ROSE_INST_DIR)
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

