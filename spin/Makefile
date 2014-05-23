#
# Copyright 2013-2014, Bradley A. Grantham
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 

default: spin

CXXFLAGS=-Wall -I/opt/local/include --std=c++11
LDFLAGS=-L/opt/local/lib -lglfw -framework OpenGL -framework Cocoa -framework IOkit

spin.o: builtin_loader.h drawable.h geometry.h manipulator.h phongshader.h vectormath.h
vectormath.o: vectormath.h
manipulator.o: geometry.h manipulator.h vectormath.h
drawable.o: drawable.h
phongshader.o: drawable.h geometry.h phongshader.h vectormath.h
builtin_loader.o: builtin_loader.h drawable.h geometry.h phongshader.h vectormath.h

CXXSOURCES      = spin.cpp vectormath.cpp manipulator.cpp drawable.cpp phongshader.cpp builtin_loader.cpp
OBJECTS         = $(CXXSOURCES:.cpp=.o)

spin: $(OBJECTS)
	g++ $^ -o $@ -L/opt/local/lib $(LDFLAGS)
