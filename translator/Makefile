CXX       = g++
BINDIR    = bin
OBJDIR    = obj
SRCDIR    = src
LDFLAGS   = -L$(OP2_ROSE_INST_DIR)/lib -lrose -s
INCLUDES  = -Iinclude -I$(OP2_ROSE_INST_DIR)/include -I$(OP2_BOOST_INST_DIR)/include
CXXFLAGS  = $(INCLUDES) -O3 -Wall

SRCS  = $(wildcard $(SRCDIR)/*.cpp)
OBJS  = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
OUT   = $(BINDIR)/translator

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "\n===== BUILDING OBJECT FILE $@ ====="
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(OBJS) 
	@echo "\n===== LINKING $@ ====="
	$(CXX) $(LDFLAGS) $(OBJS) -o $(OUT) 

.PHONY: all depend clean
all: $(OUT) 

depend: $(SRCS)
	makedepend $(INCLUDES) $^

clean:
	rm -f $(BINDIR)/* $(OBJDIR)/* *~

