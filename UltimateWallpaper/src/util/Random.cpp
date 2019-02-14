#include <random>

float rnd(const float min, const float max) {
	return min + (max - min) * rand() / RAND_MAX;
}