.PHONY: all build rebuild rel_build rel_rebuild clean update_pch build_actions clean_actions
all: build

build_type = debug

CXX = g++
CXXFLAGS += -MMD
CXXFLAGS += -Wall
CXXFLAGS += $(foreach i,$(macro_defs),-D $(i))
CXXFLAGS += $(foreach i,$(include_dirs),-I '$(i)')
CXXFLAGS += $(foreach i,$(lib_dirs),-L '$(i)')
CXXFLAGS += $(foreach i,$(lib_files),-l '$(i)')

CXXFLAGS += -std=c++0x

ifeq ($(build_type),debug)
CXXFLAGS += -g
else
CXXFLAGS += -O3 -DNDEBUG
endif

pch_file =pch.h

build: build_actions update_pch main
rebuild: clean
	$(MAKE) build
rel_build: 
	$(MAKE) build -e build_type=release
rel_rebuild: clean
	$(MAKE) rel_build
clean: clean_actions
	rm -f main $(objs) $(all_deps) $(pch_file).gch

clean_actions:
	rm -f parser/*.tokens parser/*.hpp parser/JSMinusLexer.* parser/JSMinusParser.*

update_pch: $(pch_file).gch

srcs = $(wildcard *.cpp runtime/*.cpp bytecode/*.cpp parser/*.cpp)
objs = $(srcs:.cpp=.o)
all_deps = $(srcs:.cpp=.d)
exist_deps = $(wildcard *.d)
not_exist_deps = $(filter-out $(exist_deps), $(all_deps))

main: $(objs)
	$(CXX) $(CXXFLAGS) -o $@ $(objs)

$(pch_file).gch: $(pch_file)
	$(CXX) $(filter-out -MMD,$(CXXFLAGS)) $<

ifneq ($(exist_deps),)
include $(exist_deps)
endif
ifneq ($(not_exist_deps),)
$(not_exist_deps:.d=.o):%.o:%.cpp
endif
