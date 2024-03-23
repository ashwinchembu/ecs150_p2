# Target library
lib 	:= libuthread.a
objs	:= queue.o uthread.o context.o preempt.o sem.o

CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -MMD

# Print commands with "make V=1"
ifneq ($(V),1)
Q = @
V = 0
endif

# Add -g debug flag if D=1
ifneq ($(D),1)
CFLAGS	+=
else
CFLAGS	+= -g
endif

all: $(lib)

$(lib): $(objs)
	@echo "LD	$@"
	$(Q) ar rcs $@ $^

%.o: %.c %.h
	@echo "CC	$@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c private.h 
	@echo "CC	$@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "CLEAN"
	rm -f *.d *.o *.a