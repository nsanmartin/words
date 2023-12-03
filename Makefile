DEBUG?= -g -ggdb
CFLAGS?= -Wall -W -std=c99
LDFLAGS= -lm

INCLUDE:=rax
BUILD:=build
RAXDIR:=rax

words: words.c $(RAXDIR)/rax.o
	$(CC) -I$(INCLUDE) -o $(BUILD)/$@ $^ $(DEBUG) $(CFLAGS) $(LDFLAGS)


$(RAXDIR)/rax.o: $(RAXDIR)/rax.h
	$(MAKE) -C $(RAXDIR) rax.o

tags:
	ctags -R .

clean:
	$(MAKE) -C $(RAXDIR) clean
	find $(BUILD) -type f -delete

