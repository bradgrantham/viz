spin: spin.cpp vectormath.cpp vectormath.h geometry.h manipulator.h manipulator.cpp
	g++ -Wall spin.cpp vectormath.cpp manipulator.cpp -o spin -L/opt/local/lib -I/opt/local/include/ -lglfw -framework OpenGL -framework Cocoa -framework IOkit
