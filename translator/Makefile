CXX       := g++
INCDIR	  := include
BINDIR    := bin
OBJDIR    := obj
SRCDIR    := src
LDFLAGS   := -L/usr/lib/jvm/java-6-sun/jre/lib/i386/server -ljvm -L$(BOOST_HOME)/lib -lboost_date_time -lboost_filesystem -lboost_regex -lboost_system -lboost_thread -lboost_wave -lboost_program_options  -L$(ROSE_HOME)/lib -lrose
INCLUDES  := -Iinclude -I$(ROSE_HOME)/include -I$(BOOST_HOME)/include
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

