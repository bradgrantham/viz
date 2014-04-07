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

spin: spin.cpp vectormath.cpp vectormath.h geometry.h manipulator.h manipulator.cpp model.cpp
	g++ -Wall spin.cpp vectormath.cpp manipulator.cpp model.cpp -o spin -L/opt/local/lib -I/opt/local/include/ -lglfw -framework OpenGL -framework Cocoa -framework IOkit
