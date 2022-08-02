all: webass

# -Wl,--allow-undefined 

webass: fizzicks_webass.cpp
	clang --target=wasm32 -mbulk-memory -Wl,--allow-undefined -nostdlib -Wl,--no-entry -Wl,--export-all -o fizzicks.wasm fizzicks_webass.cpp
