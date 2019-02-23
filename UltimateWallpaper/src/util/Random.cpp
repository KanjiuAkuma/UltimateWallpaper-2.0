#include <random>

float rnd(const float min, const float max) {
	return min + (max - min) * rand() / RAND_MAX;
}

float* rndArray(const int size, const float min, const float max) {
	auto* arr = new float[size];
	for (int i = 0; i < size; i++) {
		arr[i] = rnd(min, max);
	}
	return arr;
}