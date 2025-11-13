
#include "incl/Server.hpp"
#include "incl/Libraries.hpp"
#include "incl/Cluster.hpp"
#include "incl/Utils.hpp"

volatile sig_atomic_t stopSignal = 0;

void signalHandler(int signum)
{
	std::cout << "\nReceived signal " << signum << ". Shutting down server.\n";
	stopSignal =  1;
}

void	createConfigList(char *av, std::vector<std::string> &configList)
{
	bool			inServerBlock = false;
	bool			inLocationBlock = false;

	std::string		line;
	std::string		serverConfig;
	std::ifstream	iss(av);
	if (!iss)
	{
		std::cerr << "Counting server failed!\n";
		return;
	}

	while (getline(iss, line))
	{
		if (line.find("#") != std::string::npos || line.empty())
			continue;
		if (!inServerBlock && line.find("server") != std::string::npos)
			inServerBlock = true;
		else if (inServerBlock && line.find("{") != std::string::npos)
			inLocationBlock = true;
		else if (inServerBlock && inLocationBlock && line.find("}") != std::string::npos)
			inLocationBlock = false;
		else if (inServerBlock && !inLocationBlock && line.find("}") != std::string::npos)
		{
			serverConfig.append(line);
			inServerBlock = false;
			configList.push_back(serverConfig);
			serverConfig.clear();
			continue;
		}
		line += '\n';
		serverConfig.append(line);
	}
}

int main(int ac, char **av)
{
	std::signal(SIGINT, signalHandler);

	std::vector<std::string>	configList;
	std::vector<Server*>		serverList;
	Cluster cluster;
	
	try
	{
		if (ac != 2)
			throw Server::ConfigException("Please provide a config file [Usage: ./webserv *.conf]");

		std::string configPath = checkFilePath(av[1]);
		if (configPath.empty())
			throw Server::ConfigException("Invalid path for config file");

		createConfigList(configPath, configList);

		if (configList.size() < 1)
			throw Server::ServerException("Loading server configuration failed");

		if (checkforSocketDuplicates(configList) == 1)
		{
			throw Server::ServerException("Duplicate Port found!\nServer creation aborted.");
		}
		
		cluster.initializeServers(configList);
		cluster.run();
	}
	catch (const Server::ConfigException &e) {
		std::cerr << "Config file error: " << e.what() << std::endl; }
	catch (const Cluster::ClusterException &e) {
		std::cerr << "Cluster error: " << e.what() << std::endl; }
	catch (const Server::ServerException &e) {
		std::cerr << "Server error: " << e.what() << std::endl; }
	for (size_t i = 0; i < serverList.size(); ++i)
		delete serverList[i]; // Calls the Server's destructor and frees memory
	return (0);
}
