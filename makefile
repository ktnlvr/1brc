c: b
	$(CXX) main.c -O3 -O4 -o ./build/c
	./build/c

rust: b
	rustc ./main.rs -C opt-level=3 -o ./build/rs

main: b

b: 
	mkdir -p ./build
