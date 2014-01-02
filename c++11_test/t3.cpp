#include <iostream>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>

int main(int argc, char **argv) {
	auto t1 = boost::chrono::high_resolution_clock::now();
	boost::this_thread::sleep_for( boost::chrono::nanoseconds(10000) );
	auto t2 = boost::chrono::high_resolution_clock::now();

	auto time_span = boost::chrono::duration_cast<boost::chrono::duration<double>>(t2-t1);

	std::cout << time_span.count()*1000000000 << " ns" << std::endl;
}