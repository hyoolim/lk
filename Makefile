PRESET ?= dev

.PHONY: all build test install stdlib clean

all: build

build:
	cmake --preset $(PRESET)
	cmake --build --preset $(PRESET)

test:
	ctest --preset $(PRESET)

stdlib:
	cmake --build --preset $(PRESET) -t stdlib

install:
	cmake --install build

clean:
	rm -rf build build-release
	rm -rf lib/*/*.dll lib/*/*.dylib lib/*/*.so lib/*/*.o
