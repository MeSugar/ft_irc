#include "../include/Server.hpp"

/*void	testPASScmd(Server &serv)
{
	Client client;
	std::vector<std::string> vec1;
	std::vector<std::string> vec2;
	vec1.push_back("bbb");
	vec1.push_back("ccc");
	Message msg1;
	msg1.prefix = "aaa";
	msg1.command = "PASS";
	msg1.params = vec1;
	Message msg2;
	msg2.prefix = std::string();
	msg2.command = "PASS";
	msg2.params = vec1;
	serv.commandPASS(client, msg1);
	serv.commandPASS(client, msg2);
}*/

int main(int ac, char **av)
{
	if (ac == 3)
	{
		std::stringstream ss;
		ss << av[1];
		int num;
		ss >> num;
		Server serv(num, av[2]);

		serv.server_test_client();
		//testPASScmd(serv);
		return 0;
	}
	std::cout << "Wrong number of arguments" << std::endl;
	return -1;
}
