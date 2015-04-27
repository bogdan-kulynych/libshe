SHELL              := /bin/bash

BOOSTDIR           := /usr/local/lib
PREFIX             := /usr/local

CXXFLAGS           := -Wall -fPIC -std=c++11 -pedantic

INCDIR             := include
SRCDIR             := src
BUILDDIR           := build
TESTDIR            := tests

LIBS               := -L$(BOOSTDIR) -lboost_serialization -lstdc++ -lgmp
TESTLIBS           := -L$(BOOSTDIR) -lboost_unit_test_framework
INC                := -I$(INCDIR)

LIBTARGET          := libshe.so
TESTTARGET         := runtests

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
library: $(BUILDDIR)/$(LIBTARGET)

$(BUILDDIR)/$(LIBTARGET): $(LIBOBJECTS)
	@echo "Link: $^"
	@$(CXX) $(LDFLAGS) -shared $^ $(LIBS) -o $@

$(LIBOBJECTS): $(BUILDDIR)/%.o: %.cpp
	@echo "Compile: $^"
	@mkdir -p $(BUILDDIR)/$(SRCDIR)
	@$(CXX) $(CXXFLAGS) $(INC) -c $^ -o $@

.PHONY: tests
tests: CXXFLAGS += -O0 -DDEBUG -g --coverage
tests: LDFLAGS = --coverage
tests: $(BUILDDIR)/$(TESTTARGET)
	@$(BUILDDIR)/$(TESTTARGET) $(TESTOPTS)

$(BUILDDIR)/$(TESTTARGET): $(TESTOBJECTS) $(LIBOBJECTS)
	@echo "Link: $^"
	@$(CXX) $(LDFLAGS) $^ $(TESTLIBS) $(LIBS) -o $@

$(TESTOBJECTS): $(BUILDDIR)/%.o: %.cpp
	@echo "Compile: $^"
	@mkdir -p $(BUILDDIR)/$(TESTDIR)
	@$(CXX) $(CXXFLAGS) $(INC) -c $^ -o $@

.PHONY: coverage
coverage:
	@lcov --gcov-tool $(GCOV) --directory . --capture --output-file $(COVERAGEFILE)
	@lcov --gcov-tool $(GCOV) --remove $(COVERAGEFILE) 'tests/*' '/usr/*' --output-file $(COVERAGEFILE) 2> /dev/null
	@lcov --gcov-tool $(GCOV) --list $(COVERAGEFILE)
	@genhtml $(COVERAGEFILE) --output-directory $(BUILDDIR)/coverage

.PHONY: install
install: $(BUILDDIR)/$(LIBTARGET)
	@echo "Install:"
	@mkdir -p $(PREFIX)/include
	@mkdir -p $(PREFIX)/lib
	cp -rf $(INCDIR) -t $(PREFIX)
	cp -f $(BUILDDIR)/$(LIBTARGET) -t $(PREFIX)/lib/
	ldconfig

define \n


endef

.PHONY: uninstall
uninstall:
	@echo "Uninstall:"
	rm $(PREFIX)/lib/$(LIBTARGET)
	$(foreach header,$(shell ls -1 $(INCDIR)),rm -r $(PREFIX)/include/$(header)${\n})
	ldconfig

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
