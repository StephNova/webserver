#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Libraries.hpp"

class Router
{
public:
	Router(Server *server, Request *request);
	~Router();

	void	extractPath();
	void	extractFile();
	void	findDirConfig();
	void	checkForDirRequest();
	void	handleFavicon();
	void	resolvePath();
	void	assignFileWithExtension(std::string &type);
	void	assignFileWithoutExtension();
	void	checkDirPermission();
	void	checkMethods();
	void	checkScriptTypes();

	class RouterException : public std::runtime_error {
	public:
		RouterException(const std::string &error);
	};

private:
	Server 														*_server;
	Request 													*_request;
	std::string													_serverName;
	std::string													_requestedPath;
	std::string													_requestedFile;
	std::string													_extractedPath;
	std::string													_mimeType;
	std::string													_locationBlockIndex;
	std::string													_locationBlockRoot;
	std::string													_location;
	std::map<std::string, std::string>							_dirConfig;
	std::map<std::string, std::map<std::string, std::string> >	*_locationBlocks;

	Router(Router &other);
	Router& operator=(Router &other);
};

#endif	//ROUTER_HPP
