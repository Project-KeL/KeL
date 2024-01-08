OBJDIR = obj

CPPFLAGS = -std=c2x
SRCS = $(wildcard src/*.c) $(wildcard *.c)
OBJS = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

exl : $(OBJS)
	gcc $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o : %.c
	gcc $(CPPFLAGS) -o $@ -c $<

.PHONY : clean

clean :
	rm -r $(OBJDIR)/*
