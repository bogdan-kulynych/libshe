SHELL              := /bin/bash

BOOSTDIR           := /usr/local/lib
PREFIX             := /usr/local

CXXFLAGS           := -Wall -fPIC -std=c++11 -pedantic

INCDIR             := include
SRCDIR             := src
BUILDDIR           := build
TESTDIR            := tests
BENCHDIR           := benchmarks

LIBS               := -L$(BOOSTDIR) -lboost_serialization -lstdc++ -lgmp
TESTLIBS           := -L$(BOOSTDIR) -lboost_unit_test_framework
INC                := -I$(INCDIR)

LIBSOURCES         := $(wildcard $(SRCDIR)/*.cpp)
TESTSOURCES        := $(wildcard $(TESTDIR)/*.cpp)
BENCHSOURCES       := $(wildcard $(BENCHDIR)/*.cpp)
LIBOBJECTS         := $(patsubst %.cpp,$(BUILDDIR)/%.o, $(LIBSOURCES))
TESTOBJECTS        := $(patsubst %.cpp,$(BUILDDIR)/%.o, $(TESTSOURCES))
BENCHOBJECTS       := $(patsubst %.cpp,$(BUILDDIR)/%.o, $(BENCHSOURCES))
TESTTARGETS        := $(patsubst %.cpp,$(BUILDDIR)/%, $(TESTSOURCES))
BENCHTARGETS       := $(patsubst %.cpp,$(BUILDDIR)/%, $(BENCHSOURCES))

LIBTARGET          := $(BUILDDIR)/libshe.so

TESTOPTS           := --log_level=test_suite
COVERAGEFILE       := $(BUILDDIR)/coverage.info
GCOV               := gcov


define \n


endef

# Messages

LINK     := > Link
COMPILE  := > Cmpl
EXEC     := > Exec

.PHONY: all
all: library

# Library

.PHONY: library
library: CXXFLAGS += -O3
library: $(LIBTARGET)

$(LIBTARGET): $(LIBOBJECTS)
	@echo "$(LINK): $^"
	@$(CXX) $(LDFLAGS) -shared $^ $(LIBS) -o $@

$(LIBOBJECTS): $(BUILDDIR)/%.o: %.cpp
	@echo "$(COMPILE): $^"
	@mkdir -p $(BUILDDIR)/$(SRCDIR)
	@$(CXX) $(CXXFLAGS) $(INC) -c $^ -o $@

# Tests

.PHONY: tests
tests: CXXFLAGS += -O0 -DDEBUG -g --coverage
tests: LDFLAGS = --coverage
tests: $(LIBOBJECTS)
tests: $(TESTTARGETS)

$(TESTTARGETS): $(LIBOBJECTS)
$(TESTTARGETS): $(BUILDDIR)/%: $(BUILDDIR)/%.o
	@echo "$(LINK): $^"
	@$(CXX) $(LDFLAGS) $^ $(TESTLIBS) $(LIBS) -o $@
	$(foreach exe,$@,@echo "$(EXEC): $(exe)" && ./$(exe) $(TESTOPTS)${\n})

$(TESTOBJECTS): $(BUILDDIR)/%.o: %.cpp
	@echo "$(COMPILE): $^"
	@mkdir -p $(BUILDDIR)/$(TESTDIR)
	@$(CXX) $(CXXFLAGS) $(INC) -c $^ -o $@

# Coverage report

.PHONY: coverage
coverage:
	@lcov --gcov-tool $(GCOV) --directory . --capture --output-file $(COVERAGEFILE)
	@lcov --gcov-tool $(GCOV) --remove $(COVERAGEFILE) 'tests/*' '/usr/*' --output-file $(COVERAGEFILE) 2> /dev/null
	@lcov --gcov-tool $(GCOV) --list $(COVERAGEFILE)
	@genhtml $(COVERAGEFILE) --output-directory $(BUILDDIR)/coverage

# Benchmarks

.PHONY: benchmarks
benchmarks: $(LIBOBJECTS)
benchmarks: $(BENCHTARGETS)
	$(foreach exe,$<,@echo "$(EXEC): $(exe)" && ./$(exe) $(TESTOPTS)${\n})

$(BENCHTARGETS): $(LIBOBJECTS)
$(BENCHTARGETS): $(BUILDDIR)/%: $(BUILDDIR)/%.o
	@echo "$(LINK): $^"
	@$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

$(BENCHOBJECTS): $(BUILDDIR)/%.o: %.cpp
	@echo "$(COMPILE): $^"
	@mkdir -p $(BUILDDIR)/$(BENCHDIR)
	@$(CXX) $(CXXFLAGS) $(INC) -c $^ -o $@

# Installation

.PHONY: install
install: $(BUILDDIR)/$(LIBTARGET)
	@mkdir -p $(PREFIX)/include
	@mkdir -p $(PREFIX)/lib
	cp -rf $(INCDIR) -t $(PREFIX)
	cp -f $(BUILDDIR)/$(LIBTARGET) -t $(PREFIX)/lib/
	ldconfig

.PHONY: uninstall
uninstall:
	rm $(PREFIX)/lib/$(LIBTARGET)
	$(foreach header,$(shell ls -1 $(INCDIR)),rm -r $(PREFIX)/include/$(header)${\n})
	ldconfig

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
