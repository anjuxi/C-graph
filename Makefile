CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2

ifeq ($(OS),Windows_NT)
EXE := .exe
RM := del /Q
RUN :=
else
EXE :=
RM := rm -f
RUN := ./
endif

all: graph_demo$(EXE)

graph_demo$(EXE): main.o graph.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c graph.h
	$(CC) $(CFLAGS) -c main.c -o $@

graph.o: graph.c graph.h
	$(CC) $(CFLAGS) -c graph.c -o $@

test_graph$(EXE): tests/test_graph.c graph.c graph.h
	$(CC) $(CFLAGS) -I. -o $@ tests/test_graph.c graph.c

test: test_graph$(EXE)
	$(RUN)test_graph$(EXE)

clean:
	-$(RM) main.o graph.o graph_demo$(EXE) test_graph$(EXE)

.PHONY: all test clean
