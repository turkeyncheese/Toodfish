CC := gcc
CFLAGS := -Ofast -fomit-frame-pointer
LDFLAGS := -lwsock32
NNUE_DIR := ./nnue

all: toodfish toodfish.exe

toodfish: toodfish.c nnue_eval.c $(NNUE_DIR)/nnue.cpp $(NNUE_DIR)/misc.cpp
	$(CC) $(CFLAGS) -DUSE_SSE41 -msse4.1 -DUSE_SSSE3 -mssse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse $^ -o $@ $(LDFLAGS)

toodfish.exe: toodfish.c nnue_eval.c $(NNUE_DIR)/nnue.cpp $(NNUE_DIR)/misc.cpp
	x86_64-w64-mingw32-$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

debug: toodfish_debug toodfish_debug.exe

toodfish_debug: toodfish.c nnue_eval.c $(NNUE_DIR)/nnue.cpp $(NNUE_DIR)/misc.cpp
	$(CC) $^ -o $@ $(LDFLAGS)

toodfish_debug.exe: toodfish.c nnue_eval.c $(NNUE_DIR)/nnue.cpp $(NNUE_DIR)/misc.cpp
	x86_64-w64-mingw32-$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f toodfish toodfish.exe toodfish_debug toodfish_debug.exe