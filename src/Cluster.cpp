#include "../incl/Libraries.hpp"
#include "../incl/Cluster.hpp"

/*Every server has:
std::vector<int>_serverSocket which is an array of FD of each server socket
std::vector<struct pollfd> _socketArray; which is an array of pollfd of each server socket

So when a server is initialized, it creates a socket for each port in the config file, binds it to the port and the fd is pushed to the _serverSocket, then after is listens on the socket and the fd is pushed to the _socketArray.
*/

Cluster::Cluster() : _session_counter(0) {}

Cluster::ClusterException::ClusterException(const std::string &error) : std::runtime_error(error) {}

t_cookie Cluster::getCookie(std::string &id) { return _sessionData[id]; }

void	Cluster::setCookie(std::string &session_id, bool status, std::string username)
{
	_sessionData[session_id].logged_in = status;
	if (!username.empty())
		_sessionData[session_id].username = username;
	//populate other future elements of struct here
}

std::string	Cluster::makeSessionID()
{ 
	if (_session_counter == UINT_MAX - 1)
		_session_counter = 0;
	return intToString(++_session_counter);
}
bool Cluster::hasSessionID(const std::string& id)
{
	return _sessionData.find(id) != _sessionData.end();
}

void Cluster::initializeServers(std::vector<std::string> configList)
{
	if (configList.empty())
		throw ClusterException("No server configurations provided.");
	for (size_t i = 0; i < configList.size(); ++i)
	{
		Server* newServer = new Server(configList[i], this); //Create a new Server instance, all its sockets according to the ports are initialized in the constructor and they bind and listen to the ports
		_servers.push_back(newServer);
	}
	setupPollFds();
}

void Cluster::setupPollFds()
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		const std::vector<int>& serverSockets = _servers[i]->getServerSockets();
		for (size_t j = 0; j < serverSockets.size(); ++j)
		{
			pollfd pfd;
			pfd.fd = serverSockets[j];
			pfd.events = POLLIN; // Monitor for incoming data
			_pollfds.push_back(pfd);
			_fdToServerMap[serverSockets[j]] = _servers[i]; // Map the fd to its server
		}
	}
}

void Cluster::run()
{
	if (_servers.empty())
		throw ClusterException("No servers initialized. Cannot run cluster.");
	while (!stopSignal)
	{
		int ret = poll(_pollfds.data(), _pollfds.size(), POLL_TIME_OUT);
		if (ret < 0)
		{
			if (errno == EINTR)
				continue;
			perror("poll");
			break;
		}
		time_t now = time(NULL);
		for (int i = _pollfds.size() - 1; i >= 0; --i)
		{
			int fd = _pollfds[i].fd;
			short revents = _pollfds[i].revents;
			
			Server* server = _fdToServerMap[fd];
			if (!server)
			{
				std::cerr << "Unknown server for fd " << fd << "\n";
				continue;
			}
			//Check client activity timeout
			if(!server->isServerSocket(fd) && now - _lastActive[fd] > CLIENT_TIMEOUT)
			{
				std::cout << "[TIMEOUT] Closing inactive fd " << fd << "\n";
				removeConnection(fd);
				continue;
			}
			if (revents == 0)
			 	continue;
			//Handle new connections
			if (server->isServerSocket(fd) && (revents & POLLIN))
				handleNewConnection(fd, server);
			else if (revents & POLLIN)
			{
					if (!server->readFromConnection(_responseCollector, fd, _keepAlive, _pollfds))
						removeConnection(fd);
					else
						_lastActive[fd] = now; // Update last active time for the client
			}
			else if (revents & POLLOUT)
			{
				int sendResult = server->write_to_connection(_responseCollector, fd, _pollfds);
				if (sendResult == SEND_ERROR || (sendResult == SEND_COMPLETE && _keepAlive[fd] == false))
					removeConnection(fd);
				else
					_lastActive[fd] = now;
			}
			else if (revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				std::cout << "[FD ERROR] Closing fd " << fd << "\n";
				removeConnection(fd);
			}
		}
	}
}

void Cluster::handleNewConnection(int serverSocketFd, Server* server)
{
	time_t now = time(NULL);
	std::vector<int> newClients = server->makeNewConnections(serverSocketFd);

	for (size_t i = 0; i < newClients.size(); ++i)
	{
		pollfd pfd;
		pfd.fd = newClients[i];
		pfd.events = POLLIN; // Monitor for incoming data
		pfd.revents = 0; // Initialize revents to 0, poll will update it
		_pollfds.push_back(pfd); // Add the new client fd to the poll array
		_fdToServerMap[newClients[i]] = server; // Map the new client fd to its server
		_responseCollector[newClients[i]] = ""; // Initialize response collector for the new client
		_keepAlive[newClients[i]] = true; // Default keep-alive status
		_lastActive[newClients[i]] = now; // Set last active time
	}
}

void Cluster::removeConnection(int fd)
{
	close(fd);
	std::cout << "Closing fd: " << fd << '\n';
	if (_fdToServerMap.count(fd))
		_fdToServerMap[fd]->close_erase(fd); // Delete form _socketBuffers and _requestCollector

	_fdToServerMap.erase(fd);
	_responseCollector.erase(fd);
	_keepAlive.erase(fd);
	_lastActive.erase(fd);

	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			_pollfds.erase(_pollfds.begin() + i);
			//std::cout << "Removed fd " << fd << " from poll array." << std::endl;
			return;
		}
	}
}

void Cluster::removePollFd(int fd)
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			_pollfds.erase(_pollfds.begin() + i);
			_fdToServerMap.erase(fd);
			std::cout << "Removed fd " << fd << " from poll array." << std::endl;
			return;
		}
	}
	std::cerr << "FD " << fd << " not found in poll array." << std::endl;
}

Cluster::~Cluster() {
	if (_servers.empty()) {
		//std::cout << "No servers to clean up." << std::endl;
		return;
	}
	for (size_t i = 0; i < _servers.size(); ++i) {
		delete _servers[i]; // Calls the Server's destructor and frees memory
	}
}
