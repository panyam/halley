#!/bin/sh

PROJ_ROOT=/home/spanyam/personal/halley

g++ -g -fno-inline -DUSING_VALGRIND -Wall -I. main_eds.cpp net/*.cpp utils/*.cpp json/*.cpp net/http/*.cpp thread/*.cpp eds/*.cpp eds/http/*.cpp -lpthread 2>&1
g++ -g -Wall -I. main_eds.cpp net/*.cpp utils/*.cpp json/*.cpp net/http/*.cpp thread/*.cpp eds/*.cpp eds/http/*.cpp -lpthread 2>&1
# g++ -O2 -Wall -I. main_eds.cpp net/*.cpp utils/*.cpp json/*.cpp net/http/*.cpp thread/*.cpp eds/*.cpp eds/http/*.cpp -lpthread 2>&1

