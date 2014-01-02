#include <iostream>
#include <vector>
#include <boost/shared_ptr.hpp>

struct Person {
	Person(std::string _name, int _age) {
		name = _name;
		age = _age;
	}
	~Person() {
		std::cout << "Destructing person " << name << " aged " << age << std::endl;
	}
	std::string name;
	int age;
};

typedef boost::shared_ptr<Person> ptr_Person;

int main(int argc, char **argv) {
	std::vector<ptr_Person> people;

	ptr_Person michal(new Person("Michal", 17));
	ptr_Person tomas(new Person("Tomas", 17));
	ptr_Person peter(new Person("Peter", 17));

	people.push_back(michal);
	people.push_back(tomas);
	people.push_back(peter);
}