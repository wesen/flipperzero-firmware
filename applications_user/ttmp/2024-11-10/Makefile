CC=gcc
CFLAGS=-Wall -Wextra -g
SOURCES=interpreter_test.c \
        agitation_interpreter.c \
        agitation_process_interpreter.c
HEADERS=agitation_interpreter.h \
        agitation_sequence.h \
        agitation_processes.h \
        agitation_process_interpreter.h

interpreter_test: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SOURCES)

clean:
	rm -f interpreter_test 