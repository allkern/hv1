bin/hyrisc-vm main.cpp:
	mkdir -p bin

	c++ main.cpp -o bin/hyrisc-vm -std=c++17

clean:
	rm -rf "bin/hyrisc-vm"

install:
	sudo cp -rf bin/hyrisc-vm /usr/bin/