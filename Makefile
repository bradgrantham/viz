spin: spin.cpp vectormath.cpp
	g++ spin.cpp vectormath.cpp -o spin -L/opt/local/lib -I/opt/local/include/ -lglfw -framework OpenGL -framework Cocoa -framework IOkit
