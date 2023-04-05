#!/bin/sh -x

g++ -I../Forwards/include -I../Backwards/include -I../../libdecmath -I../OddsAndEnds -O0 -g -Wall -Wextra -Wpedantic -o DeciCalc main.cpp Screen.cpp ../OddsAndEnds/*.cpp ./lib/*.a -lncurses
