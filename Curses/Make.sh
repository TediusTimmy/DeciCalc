#!/bin/sh -x

g++ -I../Forwards/include -I../Backwards/include -I../../libdecmath -I../OddsAndEnds -s -O3 -Wall -Wextra -Wpedantic -o DeciCalc main.cpp Screen.cpp ../OddsAndEnds/*.cpp ./lib/*.a -lncurses
