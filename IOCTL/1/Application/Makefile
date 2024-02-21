CC = gcc

CFLAGS = -Wall -Wextra

TARGET = spockv2p_ioctl_app

$(TARGET): spockv2p_ioctl_app.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(TARGET)