#include <vector>
#include <memory>

typedef unsigned short u16;

struct symbol {
	symbol(std::string _name, u16 _addr, u16  _value): 
		name(_name),
		addr(_addr),
		value(_value) {}

	~symbol() {

	}

	std::string name;
	u16 addr;
	u16 value;
};

typedef std::shared_ptr<symbol> symbol_ptr;

int main(int argc, char **argv) {
	std::vector<symbol_ptr> symbols;

	symbol_ptr n(new symbol("data", 3, 3));
	symbols.push_back(n);
}