#include "../incl/Router.hpp"
#include "../incl/Request.hpp"
#include "../incl/Utils.hpp"
#include "../incl/Cluster.hpp"

void	Request::check_headers(const std::string &headers_raw)
{
	std::istringstream rstream(headers_raw); //turn string into stream so it can be read line by line with getline
	std::string line;
	//std::cout << "HEADERS RAW: " << headers_raw << "UNTIL HERE" << std::endl;
	if (std::getline(rstream, line))
	{
		std::istringstream lstream(line); //splits with space as delimiter
		std::string extra;
		if (!(lstream >> _method >> _path >> _version) || lstream >> extra) // less than or more than 3 parts
		{
			this->_code = 400;
			return;
		}
	}

	// make extra check for header too long for buffer --> code 431
	// URI to long
	_path = urlDecode(_path);
	replaceAll(_path, "{{UPLOAD_BLOCK}}", _server->getUploadDir()["location"].substr(1));
	replaceAll(_path, "{{CGI_BIN}}", _server->getCGIDir()["location"].substr(1));
	if (_path.empty())
	{
		this->_code = 400;
		return;
	}
	splitURI();
	//std::cout << "PATH: " << _path << ", QUERY: " << _query << std::endl;

	if (_code == 200)
	{
		if (split_headers(rstream) == 1 || checkURILength() == 1 || checkPathChars() == 1)
		{
			// std::cout << "Returning in check headers\n";
			// std::cout << "Code: " << _code << std::endl;
			// std::cout << "splitheaders returned" << split_headers(rstream) << std::endl;
			// std::cout << "checkURILength returned" << checkURILength() << std::endl;
			// std::cout << "checkPathChars returned" << checkPathChars()	<< std::endl;
			return ;
		}
		Router Router(this->_server, this);
	}
}

int Request::split_headers(std::istringstream &rstream)
{
	std::string line;
	bool blank = false;
	bool cookie_found = false;
	// _content_length = -1;
	// _chunked = false;
	while (std::getline(rstream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r') //getline removes \n but not \r
			line.erase(line.size() - 1); //remove last char (trailing carriage return '\r')
		if (line.empty())	//if there is no body, then there is maybe no empty line
		{
			blank = true;
			break;
		}
		size_t pos = line.find(": ");
		if (pos == std::string::npos || line[pos - 1] == ' ') //if no colon or extra whitespace
			return 0;
		
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 2);
		
		trim(key);
		trim(value);
		
		if (key.empty() || value.empty())
			return 0;
		
		_headers[key] = value;
		
		if (key == "Content-Length")
		{
			if (!safeAtoi(value, _content_length) || _content_length < 0 || _content_length > INT_MAX)
				return (std::cout << "Content length should be between 0 and INT MAX bytes\n", 0); // COMMENT FOR LATER: ADD EXCEPTION SO PROGRAM QUITS HERE
			std::cout << "Content-Length: " << _content_length << "*****" << std::endl;
		}
		else if (key == "Transfer-Encoding" && value == "chunked")
			_chunked = true;
		else if (key == "Cookie")
			checkCookie(value, cookie_found);
		
	}
	if (_query == "login=false")
	{
		_cluster->setCookie(_sessionID, false, "");
		_code = 303;
		_path = this->_server->getIndex();
		return (1);
	}
	if (!blank)
	{
		this->_code = 400;
		return (1); //no empty line after header
	}
	if (_content_length == -1 && _method != "POST") //check if only in GET and DELETE it's ok not to have content_length set
		_content_length = 0;
	if (cookie_found == false)
		makeNewCookie();
		
	return 0;
}

void Request::checkCookie(std::string &value, bool &cookie_found)
{
	size_t pos = 0;
	size_t pos_sid = 0;
	pos = value.find(";");
	pos_sid = value.find("sid=");
	if (pos_sid == std::string::npos)
		makeNewCookie();
	else
	{
		size_t start = pos_sid + 4;
		std::string id = value.substr(start, pos - start);
		//std::cout << "STRING ID: " << id << std::endl;
		if (!_cluster->hasSessionID(id))
			makeNewCookie();
		else
		{
			_sessionID = id;
			if (value.find("logged_in=") == std::string::npos)
				setCookie(_sessionID, false, "");
		}
	}
	cookie_found = true;
}

void	Request::makeNewCookie()
{
	std::string session_id = _cluster->makeSessionID();
	_cluster->setCookie(session_id, false, "");
	_sessionID = session_id;
}
