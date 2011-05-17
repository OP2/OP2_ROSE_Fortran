CXX       := g++
INCDIR		:= include
BINDIR    := bin
OBJDIR    := obj
SRCDIR    := src
LDFLAGS   := -L$(OP2_ROSE_INST_DIR)/lib -lrose
INCLUDES  := -Iinclude -I$(OP2_ROSE_INST_DIR)/include -I$(OP2_BOOST_INST_DIR)/include
CXXFLAGS  := $(INCLUDES) -Wall #-O2 commented out for debugging

SRCS  := $(wildcard $(SRCDIR)/*.cpp)
INCS  := $(wildcard $(INCDIR)/*.h)
OBJS  := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
OUT   := $(BINDIR)/translator

.PHONY: all clean

all: $(OUT)

clean:
	rm -f $(BINDIR)/* $(OBJDIR)/* *~

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp # $(INCDIR)/%.h commented out because it requires a .h for each .cpp
	@echo "\n===== BUILDING OBJECT FILE $@ ====="
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT): $(OBJS) 
	@echo "\n===== LINKING $@ ====="
	$(CXX) $(LDFLAGS) $(OBJS) -o $(OUT) 
