#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main()
{
	std::ifstream file("def.txt");

	if (!file) {
		std::cerr << "Error opening file." << std::endl;
		return 1;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	return 0;
}