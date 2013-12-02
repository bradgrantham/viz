spin: spin.cpp vectormath.cpp vectormath.h geometry.h
	g++ -Wall spin.cpp vectormath.cpp -o spin -L/opt/local/lib -I/opt/local/include/ -lglfw -framework OpenGL -framework Cocoa -framework IOkit
