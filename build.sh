#!/bin/sh

if (find 'build/' -quit) then
	cd build/
	cmake ..
	cmake --build .
	cd ../
else
	mkdir build/
	cd build/
	cmake ..
	cmake --build .
	cd ../
fi
