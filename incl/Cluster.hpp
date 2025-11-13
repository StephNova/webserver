#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include "Libraries.hpp"
#include "Server.hpp"

typedef struct s_cookie
{
	bool logged_in;
	std::string username;
}	t_cookie;

class Cluster {
	public:
		Cluster(); // Default constructor, initializes an empty cluster
		~Cluster();

		void initializeServers(std::vector<std::string> configList); // Initializes servers based on ConfigList
		void run(); // Main event loop using poll
		void setupPollFds();
		void handleNewConnection(int serverSocketFd, Server* server); // Handles new client connections
		void removeConnection(int fd); // Removes fd from _pollfds, _responseCollector, _keepAlive, _lastActive and _fdToServerMap
		void removePollFd(int fd); // Removes fd from _pollfds and _fdToServerMap
		t_cookie							getCookie(std::string &id);
		void								setCookie(std::string &session_id, bool status, std::string username);
		std::string	makeSessionID();
		bool hasSessionID(const std::string& id);

		class ClusterException : public std::runtime_error {
			public:
				ClusterException(const std::string &error);
		};

	private:
		std::vector<Server*>			_servers; // Manages individual Server objects
		std::vector<pollfd>					_pollfds; // Stores all sockets to be polled
		std::map<int, Server*>			 _fdToServerMap; // Map FDs to their managing Server for convenience
		std::map<int, std::string>		 _responseCollector; // Collects responses from clients
		std::map<int, bool>					_keepAlive; // Tracks keep-alive status for clients
		std::map<int, time_t>				_lastActive; // Tracks last active time for each client socket
		//t_cookie	_cookie;
		std::map<std::string, t_cookie>	_sessionData;
		unsigned int	_session_counter;
		

		Cluster(Cluster &other);
		Cluster& operator=(Cluster &other);

};

#endif // CLUSTER_HPP
