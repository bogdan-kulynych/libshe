SHELL              := /bin/bash

CXX                := clang++
CFLAGS             := -g -Wall -fPIC --std=c++11 -O3

BOOSTDIR           := /usr/local/lib

LIBS               := -lgmp -L$(BOOSTDIR) -lboost_serialization
TESTLIBS           := -lboost_unit_test_framework
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


.PHONY: all
all: library

.PHONY: library
library: $(LIBTARGET)

compile_objects = $(CXX) $(CFLAGS) $(INC) -c $^ -o $@

$(LIBTARGET): $(LIBOBJECTS)
	@echo "Link: $^"
	@$(CXX) $(LIBS) -shared $^ -o $@

$(LIBOBJECTS): $(BUILDDIR)/%.o: %.cpp
	@echo "Compile: $^"
	@mkdir -p $(BUILDDIR)/$(SRCDIR)
	@$(compile_objects)

.PHONY: test
test: $(TESTTARGET)
	@$(TESTTARGET) $(TESTOPTS)

$(TESTTARGET): $(TESTOBJECTS) $(LIBOBJECTS)
	@echo "Link: $^"
	@$(CXX) $^ $(LIBS) $(TESTLIBS) -o $@

$(TESTOBJECTS): $(BUILDDIR)/%.o: %.cpp
	@echo "Compile: $^"
	@mkdir -p $(BUILDDIR)/$(TESTDIR)
	@$(compile_objects)

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)
