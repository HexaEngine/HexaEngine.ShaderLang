#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main()
{
	ifstream file("def.txt");

	if (!file) {
		cerr << "Error opening file." << endl;
		return 1;
	}

	stringstream buffer;
	buffer << file.rdbuf();

	return 0;
}