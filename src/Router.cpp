#include "../incl/Router.hpp"
#include "../incl/Server.hpp"
#include "../incl/Libraries.hpp"
#include "../incl/Utils.hpp"


Router::Router(Server *server, Request *request) : _server(server), _request(request)
{
	//std::cout << "Router created\n";

	this->_requestedPath = this->_request->getPath();
	this->_serverName = this->_server->getName();
	this->_locationBlocks = this->_server->getLocationBlocks();

	try
	{
		extractPath();
		extractFile();
		findDirConfig();
		checkMethods();
		checkForDirRequest();
		resolvePath();
		handleFavicon();
		//std::cout << "FULL PATH from routing ==> " << this->_request->getPath() << std::endl;
		//std::cout << "CODE from routing ==> " << this->_request->getCode() << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << "Router exception: " << e.what() << std::endl;
	}
}

Router::Router(Router &other)
{
	this->_server = other._server;
	this->_request = other._request;
	this->_requestedPath = other._requestedPath;
	this->_requestedFile = other._requestedFile;
	this->_serverName = other._serverName;
	this->_locationBlocks = other._locationBlocks;
	this->_dirConfig = other._dirConfig;
	this->_extractedPath = other._extractedPath;

	std::cout << "Router copied\n";
}

Router& Router::operator=(Router &other)
{
	if (this == &other)
		return (*this);

	this->_server = other._server;
	this->_request = other._request;
	this->_requestedPath = other._requestedPath;
	this->_requestedFile = other._requestedFile;
	this->_serverName = other._serverName;
	this->_locationBlocks = other._locationBlocks;
	this->_dirConfig = other._dirConfig;
	this->_extractedPath = other._extractedPath;

	std::cout << "Router assigned\n";

	return (*this);
}

Router::~Router()
{
	//std::cout << "Router deconstructed\n";
}

Router::RouterException::RouterException(const std::string &error) : std::runtime_error(error) {}

void	Router::extractPath()
{
	//std::cout << "Requested Path: " << this->_requestedPath << '\n';

	int 		lastSlashPos = -1;

	lastSlashPos = this->_requestedPath.find_last_of('/');
	if (lastSlashPos == -1)
	{
		this->_request->setCode(404);
		throw RouterException("Invalid Request (no slash)!");
	}

	this->_extractedPath = this->_requestedPath.substr(0, lastSlashPos + 1);
}

void	Router::extractFile()
{
	int 		dotPos 		= -1;
	int 		lastSlashPos = -1;

	dotPos = this->_requestedPath.find_last_of('.');
	lastSlashPos = this->_requestedPath.find_last_of('/');

	//check if directory was set to requested file
	if (dotPos == -1)
	{
		//std::cout << "No dot in the requested path" << std::endl;
		this->_requestedFile = "none";
		return;
	}

	this->_requestedFile = this->_requestedPath.substr(lastSlashPos + 1);
	//std::cout << "Requested file found = " << this->_requestedFile << std::endl;
}

void Router::findDirConfig()
{
	std::map<std::string, std::map<std::string, std::string> >::iterator it = this->_locationBlocks->begin();
	std::map<std::string, std::map<std::string, std::string> >::iterator bestIt = this->_locationBlocks->end();
	size_t longestMatch = 0;
	std::string bestMatch;
	size_t length = 0;
	std::string usePath;

	if (_requestedFile == "none")
		usePath = _requestedPath;
	else
		usePath = _extractedPath;

	while (it != this->_locationBlocks->end())
	{
		std::string loc_path = it->first;

		if (loc_path.length() > 1)
			loc_path = loc_path.substr(0, loc_path.length() - 1);
		length = loc_path.length();

		if (usePath.compare(0, length, loc_path) == 0)
		{
			if (loc_path.length() > longestMatch)
			{
				longestMatch = loc_path.length();
				bestMatch = it->first;
				bestIt = it;
			}
		}
		++it;
	}
	if (bestIt == this->_locationBlocks->end())
	{
		this->_request->setPath("www/error/status_page.html");
		this->_request->setCode(404);
		throw RouterException("Router exception: No Locationblock for routing found!");
	}

	this->_location = bestMatch;
	this->_dirConfig.clear();
	this->_dirConfig = bestIt->second;

	this->_locationBlockIndex = this->_dirConfig["index"];
	if (this->_locationBlockIndex.empty())
		this->_locationBlockIndex = "none";

	this->_locationBlockRoot = this->_dirConfig["root"];
	if (this->_locationBlockRoot.empty())
		this->_locationBlockRoot = this->_server->getRoot();
	DIR* dir = opendir((_locationBlockRoot + _location).c_str());
	if (dir == NULL)
	{
		this->_request->setCode(404);
		throw RouterException("Router exception: Directory " + _location + " doesn't exist!");
	}
	closedir(dir);
		//std::cout << "Location Block ===> " << _location << std::endl;
	if (this->_requestedFile == "none")
	{
		if (this->_requestedPath.find_last_of("/") != this->_requestedPath.size() - 1)
		{
			std::cout << "Setting path to _location from findDirConfig" << std::endl;
			this->_requestedPath += '/';
			this->_request->setPath(_requestedPath);
			if (this->_request->getMethod() == "POST" || this->_request->getMethod() == "DELETE")
				this->_request->setCode(307);
			else
				this->_request->setCode(301);
			throw RouterException("Redirect");
		}
	}
}

void	Router::checkForDirRequest()
{
	if (_request->getMethod() == "GET" && ((!this->_requestedPath.empty() && *(this->_requestedPath.end() - 1) == '/') || this->_requestedFile == "none"))
	{
		std::cout << "Detected a directory request or missing file.\n";

		if (this->_locationBlockIndex != "none")
		{
			std::cout << "Index found for directory → serving to corresponding index of the location block "  << "\n";
			this->_requestedFile = this->_locationBlockIndex;
			this->_requestedPath +=_requestedFile;
			return;
		}
		if (_dirConfig["autoindex"] == "on")
		{
			std::cout << "Autoindex is on → updating path to autoindex\n";
			//this->_request->setAutoindex(true);
			//std::cout << "PATHHHh: " << this->_request->getPath() << "\n";
			this->_requestedFile = "__AUTO_INDEX__";
			this->_requestedPath +=_requestedFile;
		}
		else
		{
			this->_request->setCode(403);
			throw RouterException("No index file defined in location block and autooindex off, returning 403");
		}
	}
}

void	Router::resolvePath()
{
	std::string	fullPath;
	std::string root = this->_server->getRoot();
	fullPath = checkCwd(root, false); // TODO check the root maybe it can be locationBlockRoot instead?

	if (_request->getMethod() == "DELETE")
	{
		std::cout << "Setting path for DELETE" << std::endl;
		this->_request->setPath(this->_locationBlockRoot + this->_requestedPath); //TODO
		std::cout << "DELETE request, returning full path: " << fullPath << '\n';
		return;
	}

	//std::cout << "Location block Root = " << this->_locationBlockRoot << std::endl;
	//std::cout << "Requested Path = " << this->_requestedPath << std::endl;
	this->_request->setPath(this->_locationBlockRoot + this->_requestedPath);
	//std::cout << "FROM setDirForType Setting Path to: " << this->_request->getPath() << std::endl;
}

void	Router::handleFavicon()
{
	std::string	serverRoot = this->_server->getRoot();
	std::string newPath = checkCwd(serverRoot, false);

	if (this->_requestedFile == "favicon.ico")
	{
		newPath += "/files/favicon.ico";
		this->_request->setPath(newPath);
	}
}

void	Router::checkMethods()
{
	std::string	method = this->_request->getMethod();
	std::map<std::string, std::string>::iterator it = this->_dirConfig.begin();

	while (it != this->_dirConfig.end())
	{
		if (it->first == "methods")
		{
			if (it->second.find(method) != std::string::npos)
			{
				//std::cout << "Request Method is allowed: "<< method << '\n';
				return ;
			}
		}
		++it;
	}
	// Method not found
	this->_request->setPath("www/error/status_page.html");
	this->_request->setCode(405);
	throw RouterException("Method is not allowed!");
}
