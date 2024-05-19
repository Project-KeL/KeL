OBJDIR = ./obj

wildcard_recursive = $(foreach D, $(wildcard $(1:=/*)), $(call wildcard_recursive, $D, $(2)) $(filter $(subst *, %, $2), $D))

CPPFLAGS = -std=c2x -O0
SRCS = $(call wildcard_recursive, ., *.c)
VPATH = $(dir $(SRCS))
OBJS = $(patsubst %.c, $(OBJDIR)/%.o, $(notdir $(SRCS)))

kel : $(OBJS)
	gcc $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.c
	gcc $(CPPFLAGS) -g -c $< -o $@ -I./headers -I./binary/headers -I./linker/headers

.PHONY: clean

clean:
	rm -r $(OBJDIR)/*
