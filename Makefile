.PHONY: all clean
TARGET=lifegame

all: ${TARGET}

clean:
	rm -f ${TARGET}
	rm -f *.o

${TARGET}: ${TARGET}.o
