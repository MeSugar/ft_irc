#include "../include/Server.hpp"

// void	testPASScmd(Server &serv)
// {
// 	Client client;
// 	std::vector<std::string> vec1;
// 	std::vector<std::string> vec2;
// 	vec1.push_back("bbb");
// 	vec1.push_back("ccc");
// 	Message msg1("aaa", "PASS", vec1);
// 	Message msg2(std::string(), "PASS", vec1);
// 	serv.commandPASS(client, msg1);
// 	serv.commandPASS(client, msg2);
	
// }

int main(int ac, char **av)
{
	if (ac == 3)
	{
		std::stringstream ss;
		ss << av[1];
		int num;
		ss >> num;
		Server serv(num, av[2]);
		// testPASScmd(serv);
		return 0;
	}
	std::cout << "Wrong number of arguments" << std::endl;
	return -1;
}
