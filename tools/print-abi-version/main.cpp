#include <iostream>

int main(void) {
	std::cout << "GCC version: " << __GNUC__ << std::endl;
	std::cout << "ABI version: " << (__GXX_ABI_VERSION - 1000) << std::endl;
	std::cout << "C++ standard version: " << __cplusplus << std::endl;
}
