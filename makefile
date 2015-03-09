
XRLOBJECTS:=src/xrl.o src/image.o src/tilemap_frag_asm.o

CC=gcc
AR=ar
STRIP=strip

all: test_xrl libxrl.a libxrl.so
windows: test_xrl.exe xrl.dll

test_xrl: test_xrl.o libxrl.a
	$(CC) -g -o $@ $^ -L. -std=c99 -Wall -lSDL2 -lGL -lGLEW -lpng
	$(STRIP) -s $@

test_xrl.exe: test_xrl.o xrl.dll
	$(CC) -g -o $@ $^ -L. -lxrl -std=c99 -Wall -lmingw32 -lSDL2main -lSDL2.dll -mwindows -lopengl32 -lglew32 -lpng -lz -static-libgcc -Wl,-subsystem,windows
	$(STRIP) -s $@

src/tilemap_frag_asm.o: src/tilemap.frag.asm
	echo -e " \
	const char tilemap_frag_asm[] = {\n`cat src/tilemap.frag.asm | xxd -i`, 0x00\n};\n \
	" | $(CC) -xc - -c -o $@ $(CFLAGS)

.PHONY: publish
publish: xrl.dll
	rm -rf publish/*
	mkdir --parents publish/xrl-windows/include/
	mkdir --parents publish/xrl-windows/lib/
	mkdir --parents publish/xrl-windows/bin/
	cp -r xrl/ publish/xrl-windows/include/
	cp xrl.lib publish/xrl-windows/lib/
	cp xrl.dll publish/xrl-windows/bin/
	cd publish ; zip -r xrl-windows.zip xrl-windows/

%.o: %.c
	$(CC) -g -c -o $@ $< -std=c99 -Wall -I.

libxrl.a: $(XRLOBJECTS)
	$(AR) rcs libxrl.a $^

libxrl.so: $(XRLOBJECTS)
	$(CC) -shared -o $@ $^

xrl.dll: $(XRLOBJECTS)
	$(CC) -shared -o $@ $^ -lmingw32 -lSDL2main -lSDL2.dll -mwindows -lopengl32 -lglew32 -lpng -lz -Wl,--out-implib,xrl.lib
