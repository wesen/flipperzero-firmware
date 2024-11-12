CC=gcc
CFLAGS=-Wall -Wextra -g
DEBUG=1

SOURCES_DIR=../../gpio_loop
SOURCES=$(SOURCES_DIR)/agitation_interpreter.c \
        $(SOURCES_DIR)/agitation_process_interpreter.c
HEADERS=$(SOURCES_DIR)/agitation_interpreter.h \
        $(SOURCES_DIR)/agitation_sequence.h \
        $(SOURCES_DIR)/agitation_processes.h \
        $(SOURCES_DIR)/agitation_process_interpreter.h

interpreter_test: interpreter_test.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -DDEBUG=$(DEBUG) -DHOST -I$(SOURCES_DIR) -o $@ interpreter_test.c $(SOURCES)

clean:
	rm -f interpreter_test 