#include <iostream>
#include <string>
#include <boost/foreach.hpp>

int main(int argc, char **argv) {
	std::string hello = "Hello World!";

	BOOST_FOREACH(char c, hello) {
		std::cout << c << std::endl;
	}
}