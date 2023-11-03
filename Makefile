# TYPE (executable / staticlib)
TYPE = executable

# INCLUDE FLAGS
INCFLAGS   = -iquotesrc

# FRAMEWORKS
FRAMEWORKS =

# COMPILER FLAGS
CC		 = emcc
CXX      = em++
CCFLAGS  = -O3 -g -Wall -Wextra -Wpedantic -Wno-c99-extensions -Wno-unused-parameter
CCFLAGS += $(INCFLAGS)
CXXFLAGS = $(CCFLAGS) -std=c++20 

# LINKER FLAGS
LDFLAGS  = -lm -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sNO_EXIT_RUNTIME=1 -O3 -sFETCH -sASSERTIONS -sALLOW_MEMORY_GROWTH
LDFLAGS += -lstdc++
LDFLAGS += $(INCFLAGS)
LDFLAGS += $(FRAMEWORKS)

# DIRECTORIES
SRCDIR = Source
BINDIR = bin
INTDIR = obj

# OUTPUT EXEC NAME
BIN = anders.js

# ------------------------------ #
# 		   DO NOT TOUCH			 #
# ------------------------------ #

# DEBUG
print-%  : ; @echo $* = $($*)

SRC = $(shell find $(SRCDIR) -name "*.cpp" -o -name "*.c")
OBJ := $(patsubst $(SRCDIR)/%.cpp, $(INTDIR)/%.o, $(SRC))
OBJ := $(patsubst $(SRCDIR)/%.c, $(INTDIR)/%.o, $(OBJ))
DEP = $(patsubst %.o, %.d, $(OBJ))
UNAME_S = $(shell uname -s)

.PHONY: all build dirs run executable staticlib clean

all: build

build: $(TYPE)

dirs:
	mkdir -p ./$(BINDIR)

run: executable
	$(BINDIR)/$(BIN)

executable: dirs $(BINDIR)/$(BIN)

staticlib: dirs $(BINDIR)/$(BIN).a

$(BINDIR)/$(BIN).a: $(OBJ)
	ar rcs $(BINDIR)/$(BIN).a $(OBJ)

$(BINDIR)/$(BIN): $(OBJ)
	$(CXX) -o $(BINDIR)/$(BIN) $(filter %.o,$^) $(LDFLAGS)

-include $(DEP)

$(INTDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) -o $@ -c $< $(CXXFLAGS) -MMD -MP

$(INTDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	emcc -o $@ -c $< $(CCFLAGS) -MMD -MP

clean:
	rm -rf $(BINDIR) $(INTDIR)