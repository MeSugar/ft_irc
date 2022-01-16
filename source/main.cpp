#include "../include/Server.hpp"

int main(int ac, char **av)
{
	if (ac == 3)
	{
		std::stringstream ss;
		ss << av[1];
		int num;
		ss >> num;
		Server serv(num, av[2]);
		serv.loop();
		return 0;
	}
	std::cout << "Wrong number of arguments" << std::endl;
	return -1;
}
