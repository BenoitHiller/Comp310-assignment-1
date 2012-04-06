OBJS = shesells
CC = gcc
CFLAGS = -std=gnu99 -Wall
withprompt:
	$(CC) $(CFLAGS) -o shesells -DPROMPT simple.c
shesells:
	$(CC) $(CFLAGS) -o shesells simple.c
install: shesells
	cp shesells /usr/bin/ || (mkdir ~/bin; cp shesells ~/bin/)
