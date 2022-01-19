CC=gcc
SRCS=src/funcov.c src/get_coverage.c
TARGET=funcov 

all: $(SRCS)
	$(CC) -o funcov $(SRCS)

clean:
	rm $(TARGET)