CC=gcc
SRCS=src/funcov.c src/shm_coverage.c src/get_coverage.h
TARGET=funcov 

all: $(SRCS)
	$(CC) -o funcov $(SRCS)

clean:
	rm $(TARGET)