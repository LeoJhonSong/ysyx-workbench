.PHONY: all run gdb clean latest $(ALL)

RESULT = .result
$(shell > $(RESULT))

COLOR_RED   = \033[1;31m
COLOR_GREEN = \033[1;32m
COLOR_CYAN = \033[1;36m
COLOR_NONE  = \033[0m

ALL = $(basename $(notdir $(shell find tests/. -name "*.c")))

all: $(addprefix Makefile., $(ALL))

$(ALL): %: Makefile.%

Makefile.%: tests/%.c latest
	@printf "$(COLOR_CYAN) $*$(COLOR_NONE)\n"
	@/bin/echo -e "NAME = $*\nSRCS = $<\nLIBS += klib\ninclude $${AM_HOME}/Makefile" > $@
	@if [[ "$(MAKECMDGOALS)" == "test" ]]; then \
		make -s -f $@ ARCH=$(ARCH) NEMUFLAGS=--batch run 2>&1 > /dev/null; # redirect strerr of command to stdout and drop stdout of command \
		if [[ "$$(tail -n 1 build/nemu-log.txt)" == "[exit code] 0" ]]; then \
			printf "$(COLOR_GREEN) $*$(COLOR_NONE)\n" >> $(RESULT); \
		else \
			printf "$(COLOR_RED) $*$(COLOR_NONE)\n" >> $(RESULT); \
		fi \
	else \
		make -s -f $@ ARCH=$(ARCH) $(MAKECMDGOALS); \
	fi
	-@rm -f Makefile.$*

run: all
	@rm $(RESULT)

test: all
	@echo ────────────────────────────────────Summary─────────────────────────────────────
	@cat $(RESULT)
	@rm $(RESULT)

gdb: all

clean:
	rm -rf Makefile.* build/

latest:
