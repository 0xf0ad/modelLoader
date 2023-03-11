#pragma once

#include <chrono>
#include <memory>

inline auto startTime(){
	return std::chrono::high_resolution_clock::now();
}

inline long long timerStop(std::chrono::time_point<std::chrono::high_resolution_clock>& startTime){
	auto endTime = std::chrono::high_resolution_clock::now();
	auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
	auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

	return end - start;
}
