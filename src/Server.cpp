
#include	"../incl/Server.hpp"
#include	"../incl/Request.hpp"
#include	"../incl/Response.hpp"
#include	"../incl/Cluster.hpp"

Server::Server(std::string &serverConfig, Cluster *cluster) : _cluster(cluster)
{
	//std::cout << "Server created\n";
	this->createConfig(serverConfig);
	this->extractVariables();
	this->storeServerConfig();
	this->assignUploadDir();
	this->assignCGIDir();
	this->checkScriptsExecutable();

	//set default port if none in config file
	if (this->_numPorts == 0)
	{
		std::cout << "No port in config file, default set to 8080!\n";
		this->_ports.push_back(8080);
	}

	std::vector<int>::iterator it = this->_ports.begin();

	while (it != this->_ports.end())	// maybe change to for with index
	{
		int sock = this->createServerSocket(*it); //create a server socket and bind it to the port
		this->_serverSockets.push_back(sock);
		startListen(sock); //start listening on the socket and push it to the pollfd array _socketArray
		std::cout << "Server socket fd: " << sock << " created and bound\n";
		++it;
	}
}

Server::~Server()
{
	for (size_t i = 0; i < this->_serverSockets.size(); ++i)
	{
		close(this->_serverSockets[i]);
		std::cout << "Server socket fd: " << this->_serverSockets[i] << " closed\n";
	}
}

void	Server::startListen(int socket)
{
	//start listening for incoming connections
	if (listen(socket, 20) < 0)	// was 1, 10 is to test / amount of connections
		throw ServerException("Listen failed!");

	//std::cout << "Server starts listening for incomming connections on FD " << socket <<  "\n";

}

void Server::closeServer()
{
	for (size_t i = 0; i < _pollFdArray.size(); ++i) {
		std::cout << "Closing socket fd " << _pollFdArray[i].fd <<std::endl;
		close(_pollFdArray[i].fd);
	}
}

const std::vector<struct pollfd>& Server::getpollFdArray() const { return this->_pollFdArray; }

const std::vector<int>& Server::getServerSockets() const { return this->_serverSockets; }

std::string	Server::getName() { return _name; }

std::string Server::getRoot() { return _serverRoot;}

Server::ServerException::ServerException(const std::string &error) : std::runtime_error(error) {}

Server::ConfigException::ConfigException(const std::string &error) : std::runtime_error(error) {}

size_t	Server::getMaxBodySize() { return _maxBodySize; }
std::string Server::getIndex() {return _index;}

std::map<std::string, std::string> Server::getUploadDir() { return _uploadDir; }

std::string Server::getErrorPage() { return _errorPage; };

std::map<std::string, std::map<std::string, std::string> >*	Server::getLocationBlocks() { return &_locationBlocks;}

std::map<std::string, std::string>	Server::getCGIDir() { return _cgiDir; }

std::vector<std::string>	Server::getAllowedScripts() { return _allowedScripts; }
Cluster*	Server::getCluster() { return _cluster; }
