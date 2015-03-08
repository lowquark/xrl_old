
XRLOBJECTS:=src/xrl.o src/image.o src/tilemap_frag_asm.o

all: test_xrl libxrl.a libxrl.so

test_xrl: test_xrl.o libxrl.a
	clang -g -o $@ $^ -L. -std=c99 -Wall -lSDL2 -lGL -lGLEW -lpng
	strip -s $@

src/tilemap_frag_asm.o: src/tilemap.frag.asm
	echo -e " \
	const char tilemap_frag_asm[] = {\n`cat src/tilemap.frag.asm | xxd -i`, 0x00\n};\n \
	" | clang -xc - -c -o $@ $(CFLAGS)

%.o: %.c
	clang -g -c -o $@ $< -std=c99 -Wall -I.

libxrl.a: $(XRLOBJECTS)
	ar rcs libxrl.a $^

libxrl.so: $(XRLOBJECTS)
	clang -shared -o $@ $^
