// pipe.cpp : Defines the entry point for the application.
//

#include "pipe.h"
#include <iostream>
#include <string>

using namespace std;

int main()
{
	string line;
	while (getline(cin, line))
	{
		cout << line << endl;
	}
	return 0;
}