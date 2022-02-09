CC=gcc
SRCS=src/funcov.c src/shm_coverage.c src/get_coverage.c src/translate_addr.c
TARGET=funcov 

all: $(SRCS)
	$(CC) -o funcov $(SRCS)

clean:
	rm $(TARGET)