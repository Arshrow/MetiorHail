#include "rand.hpp"



int rand_int(const int Min, const int Max) {
	if (Min > Max) throw "M < m!";
	std::random_device r;
	std::default_random_engine e1(r());
	std::uniform_int_distribution<int> RandInterval(Min, Max);
	return RandInterval(e1);
}

