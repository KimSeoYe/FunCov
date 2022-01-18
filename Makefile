CC=gcc
SRCS=src/funcov.c 
TARGET=funcov 

all: $(SRCS)
	$(CC) -o funcov src/funcov.c 

clean:
	rm $(TARGET)