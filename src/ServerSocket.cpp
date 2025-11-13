#include	"../incl/Server.hpp"
#include	"../incl/Request.hpp"
#include	"../incl/Response.hpp"

void	Server::printPorts()
{
	std::vector<int>::iterator it = this->_ports.begin();

	while (it != this->_ports.end())
	{
		std::cout << "Port: " << *it << '\n';
		++it;
	}
}

void	Server::checkPortDuplicates(int &port)
{
	std::vector<int>::iterator it = this->_ports.begin();

	while (it != this->_ports.end())
	{
		if (port == *it)
			throw ConfigException("Port duplicate found!");

		++it;
	}
}

// void	Server::extractPorts(std::map<std::string, std::string>::iterator &it)
// {
// 	int			port = 0;
// 	int			portCounter = 0;
// 	std::string item;
// 	std::string	trimmedItem;

// 	std::istringstream iss(it->second);
// 	while (getline(iss, item, ','))
// 	{
// 		trimmedItem = trim(item);
// 		if (!safeAtoi(trimmedItem, port) || port < 1024 || port > 65535) //below 1024 only with sudo rights
// 			throw ServerException("Ports need to be between 1024 and 65535");
// 		checkPortDuplicates(port);
// 		this->_ports.push_back(port);
// 		++portCounter;
// 	}
// 	this->_numPorts = portCounter;
// }

void	Server::storeServerConfig()
{
	std::map<std::string, std::string> *config = getConfigMap("serverConfig");

	if (!config)
		throw ConfigException("Extracting serverConfig map failed!");

	std::map<std::string, std::string>::iterator it = config->begin();
	while (it != config->end())
	{
		// if (it->first.find("listen") != std::string::npos)
		// 	extractPorts(it);

		// if (it->first.find("name") != std::string::npos)
		// 	this->_name = it->second;

		if (it->first.find("maxbodysize") != std::string::npos)
		{
			int size;
			if (!safeAtoi(it->second, size) || size < 0)
				throw ConfigException("Max body size needs to be between 0 and INT MAX");
			_maxBodySize = static_cast<size_t>(size);
		}

		++it;
	}
	printPorts();
}

int	Server::createServerSocket(int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		throw ServerException("Creating server socket failed!");

	//prepare server adress structure
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;				//IPv4
	serverAddr.sin_port = htons(port);				//port in network byte order
	if (this->_IPHost == "0.0.0.0") // Dynamic IP address
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all available interfaces
	else
	{
		if (inet_aton(this->_IPHost.c_str(), &serverAddr.sin_addr) == 0)
		{
			std::cerr << "Error: Invalid IP address " << this->_IPHost << " for " << this->_name << std::endl;
			close(sock);
			return -1;
		}
	}

	int yes = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));	// reuse port
	fcntl(sock, F_SETFL, O_NONBLOCK);	// set socket to non-blocking mode
	if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		 std::cerr << "Error binding socket to port " << ntohs(serverAddr.sin_port)
						<< ": " << strerror(errno) << " (errno: " << errno << ")" << std::endl;
		throw ServerException("Bind failed!");
		// std::cerr << "Error binding server socket!\n";
		// exit(1);
	}
	return (sock);
}
