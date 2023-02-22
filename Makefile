.PHONY: all clean

all:
	cmake -S . -B BUILD
	MAKEFLAGS=--silent cmake --build BUILD

clean:
	rm -rf BUILD
