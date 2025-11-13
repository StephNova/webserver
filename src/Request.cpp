
#include "../incl/Request.hpp"
#include "../incl/Utils.hpp"
#include "../incl/Cluster.hpp"

Request::Request(Server *server) : _content_length(-1), _code(200), _chunked(false), _parse_pos(0), 
	_errorPage(server->getErrorPage()), _server(server)
{
	//std::cout << "Request constructed\n";
	_cluster = _server->getCluster();
}

Request::~Request()
{
	//std::cout << "Request deconstructed\n";
}


std::ostream &operator<<(std::ostream &os, Request &request)
{
	os << "Method: " << request._method << ", URI: " << request._path << ", VERSION: " << request._version << std::endl;
	os << "Headers:\n";
	for (std::map<std::string, std::string>::const_iterator it = request._headers.begin(); it != request._headers.end(); ++it)
	{
		os << "  " << it->first << ": " << it->second << "\n";
	}
	//os << "Body:\n" << request._body << "\n";
	//os << "CODE: " << request._code << std::endl;
	return os;
}

void 	Request::setCode(int code) { _code = code; }
void	Request::setPath(std::string path) { this->_path = path; }
// void	Request::setAutoindex(bool autoindex) {_autoindex = autoindex; };
void	Request::append_body(const std::string &body_part) { _body += body_part; }

int	Request::getCode() { return _code; }
std::string Request::getMethod() { return _method; }
std::string Request::getPath() { return _path; }
std::string Request::getVersion() { return _version; }
std::string Request::getBody() { return _body; }
std::string Request::getQuery() { return _query; }
int 	Request::getContentLength() { return _content_length; }
size_t Request::getParsePos() const { return _parse_pos; }
std::string Request::getErrorPage() { return _errorPage; };
bool 	Request::getConnection()
{
	// if (_headers["Connection"] == "keep-alive")
		// return true;
	if(_headers["Connection"] == "close")
		return false;
	return (true);
}
std::string Request::getHeader(const std::string &key)
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end())
        	return it->second;
	//std::cout << "Header " << key << " does not exist\n";
	return "";
}
void	Request::setHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}

size_t Request::getBodySize() { return _body.size(); }
bool Request::isChunked() { return _chunked; }
std::string	Request::getSessionID() { return _sessionID; }
std::string	Request::getUserName() { return _cluster->getCookie(_sessionID).username; }

bool Request::getCookieStatus(std::string &id) { return _cluster->getCookie(id).logged_in; }
void Request::setCookie(std::string &session_id, bool status, std::string username) {_cluster->setCookie(session_id, status, username);}



void	Request::splitURI()
{
	size_t pos = _path.find('?');
	if (pos == std::string::npos)
		_query = "";
	else
	{
		_query = _path.substr(pos + 1);
		_path = _path.substr(0, pos);
	}
	pos = 0;
	if (_path.compare(0, 7, "http://") == 0)
		pos = 7;
	pos = _path.find("//", pos);
	if (pos != std::string::npos)
	{
		while ((pos = _path.find("//", pos)) != std::string::npos) //COMMENT FOR LATER: ADD EXCEPTION FOR HTTP:// AND MAKE REDIRECTION
		{
			_path.replace(pos, 2, "/");
		}
		//std::cout << "PATH NEW AFTER SLASHES: " << _path << std::endl;
		if (_method == "POST" || _method == "DELETE")
			_code = 307;
		else
			_code = 303;
	}
}

Request::RequestException::RequestException(std::string error)  : std::runtime_error(error) {}
