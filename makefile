cc := clang gcc tcc
cflags := -O3
output := lockshell
sources := src/lockshell.c src/sha256.c

# Windows stuff
ifeq ($(OS),Windows_NT)
	extension :=.exe
	which := where
else
	extension :=
	which := which
endif

output := bin/$(output)$(extension)

# Find working c compiler
pathsearch = $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(PATH)))))
cc := $(firstword $(foreach compiler,$(cc),$(call pathsearch,$(compiler))))

.PHONY: all clean dir

all: dir $(output)

clean:
	-@rm -rf bin

dir: 
	-@mkdir -p bin

$(output): $(sources)
	$(cc) $(cflags) $(sources) -Isrc -o $@

