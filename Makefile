SHELL              := /bin/bash

BOOSTDIR           := /usr/local/lib

CXXFLAGS           := -Wall -fPIC -std=c++11 -pedantic

LIBS               := -L$(BOOSTDIR) -lboost_serialization -lstdc++ -lgmp
TESTLIBS           := -L$(BOOSTDIR) -lboost_unit_test_framework
INC                := -Iinclude

SRCDIR             := src
BUILDDIR           := build
TESTDIR            := test

LIBTARGET          := $(BUILDDIR)/libshe.so
TESTTARGET         := $(BUILDDIR)/runtests

LIBSOURCES         := $(wildcard $(SRCDIR)/*.cpp)
TESTSOURCES        := $(wildcard $(TESTDIR)/*.cpp)
LIBOBJECTS         := $(patsubst %.cpp,$(BUILDDIR)/%.o, $(LIBSOURCES))
TESTOBJECTS        := $(patsubst %.cpp,$(BUILDDIR)/%.o, $(TESTSOURCES))

TESTOPTS           := --log_level=test_suite
COVERAGEFILE       := $(BUILDDIR)/coverage.info
GCOV               := gcov


.PHONY: all
all: library

.PHONY: library
library: CXXFLAGS += -O3
library: $(LIBTARGET)

$(LIBTARGET): $(LIBOBJECTS)
	@echo "Link: $^"
	@$(CXX) $(LDFLAGS) -shared $^ $(LIBS) -o $@

$(LIBOBJECTS): $(BUILDDIR)/%.o: %.cpp
	@echo "Compile: $^"
	@mkdir -p $(BUILDDIR)/$(SRCDIR)
	@$(CXX) $(CXXFLAGS) $(INC) -c $^ -o $@

.PHONY: test
test: CXXFLAGS += -O0 -DDEBUG -g --coverage
test: LDFLAGS = --coverage
test: $(TESTTARGET)
	@$(TESTTARGET) $(TESTOPTS)

.PHONY: coverage
coverage:
	@lcov --gcov-tool $(GCOV) --directory . --capture --output-file $(COVERAGEFILE)
	@lcov --gcov-tool $(GCOV) --remove $(COVERAGEFILE) 'test/*' '/usr/*' --output-file $(COVERAGEFILE) 2> /dev/null
	@lcov --gcov-tool $(GCOV) --list $(COVERAGEFILE)

$(TESTTARGET): $(TESTOBJECTS) $(LIBOBJECTS)
	@echo "Link: $^"
	@$(CXX) $(LDFLAGS) $^ $(TESTLIBS) $(LIBS) -o $@

$(TESTOBJECTS): $(BUILDDIR)/%.o: %.cpp
	@echo "Compile: $^"
	@mkdir -p $(BUILDDIR)/$(TESTDIR)
	@$(CXX) $(CXXFLAGS) $(INC) -c $^ -o $@

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
