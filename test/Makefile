.SUFFIXES:	# Disable implicit rules

CC = gcc

MYHOME=/work/gg10/e29005
CPPFLAGS = -I$(MYHOME)/project/uti/install/include
CFLAGS = -g -O2
LDFLAGS = -L$(MYHOME)/project/uti/install/lib -luti -Wl,-rpath -Wl,$(MYHOME)/project/uti/install/lib -lpthread
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
