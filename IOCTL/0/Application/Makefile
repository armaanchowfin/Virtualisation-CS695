CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall -std=c99 -O0 -g -Wconversion -Wextra

all: clean cret0_ioctl_app

cret0_ioctl_app: cret0_ioctl_app.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o cret0_ioctl_app