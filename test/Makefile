.SUFFIXES:	# Disable implicit rules

CC = gcc

CPPFLAGS = -I$(HOME)/project/uti/install/include
CFLAGS = -g -O2
LDFLAGS = -L$(HOME)/project/uti/install/lib -luti -Wl,-rpath -Wl,$(HOME)/project/os/install/lib -lpthread
SRCS = $(shell ls *.c)
EXES = $(SRCS:.c=)
OBJS = $(SRCS:.c=.o)

all: $(EXES)

% : %.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o :: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -f core $(EXES) $(OBJS)
