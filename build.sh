#!/bin/sh

if (find 'build/' -quit) then
	cd build/
	cmake $1 ..
	cmake --build .
	cd ../
else
	mkdir build/
	cd build/
	cmake $1 ..
	cmake --build .
	cd ../
fi
