c: b
	$(CXX) main.c -O3 -O4 -o ./build/c
	./build/c

rs: b
	cd rust && cargo build --release && mv ./target/release/brc ../build/rs

main: b

b: 
	mkdir -p ./build
