c: b
	$(CXX) c/main.c -O3 -O4 -o ./build/c -march=native -flto
	./build/c

rs: b
	cd rust && cargo build --release && mv ./target/release/brc ../build/rs

main: b

b: 
	mkdir -p ./build
