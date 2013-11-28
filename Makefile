spin: spin.cpp
	g++ spin.cpp -o spin -L/opt/local/lib -I/opt/local/include/ -lglfw -framework OpenGL -framework Cocoa -framework IOkit
