.PHONY: all build run clean

all: build

build:
	mkdir -p tmp
	TMP=tmp TEMP=tmp TMPDIR=tmp gcc -pipe -Wall -std=c99 src/*.c \
		-I"C:/SDL2/x86_64-w64-mingw32/include" \
		-L"C:/SDL2/x86_64-w64-mingw32/lib" -lmingw32 -lSDL2main -lSDL2 -lm \
		-o renderer

run:
	./renderer

clean:
	rm -f renderer
	rm -rf tmp