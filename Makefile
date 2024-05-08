OBJDIR = obj

CPPFLAGS = -std=c2x
SRCS = $(wildcard ./*.c) $(wildcard ./src/*.c) $(wildcard ./src/*/*.c)
OBJS = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

kel : $(OBJS)
	gcc $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o : %.c
	gcc $(CPPFLAGS) -o $@ -c -I./headers $<

.PHONY : clean

clean :
	rm -r $(OBJDIR)/*
