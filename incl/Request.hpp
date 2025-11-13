
#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <vector>
#include <cctype>	//needed for std::isalnum
#include "Server.hpp"

class Request
{
	public:
		Request(Server *server);
		~Request();

		// int	parse_request(const std::string &request_raw);
		void								check_headers(const std::string &headers_raw);
		void								splitURI();
		int									split_headers(std::istringstream &rstream);
		bool								parse_chunks(std::string &data, size_t start);
		int									checkURILength();
		int									checkPathChars();
		int									checkRequestedPath();
		int									checkRequestedFiletype();

		void								setCode(int code);
		// void	setAutoindex(bool autoindex);
		void								setPath(std::string path);
		void								append_body(const std::string &body_part);
		int									getCode();
		std::string 						getMethod();
		std::string 						getPath();
		std::string 						getVersion();
		std::string 						getBody();
		std::string 						getQuery();
		bool 								getConnection();
		std::string 						getHeader(const std::string &key);
		void								setHeader(const std::string &key, const std::string &value);
		int 								getContentLength();
		std::string 						getErrorPage();
		size_t								getBodySize();
		size_t								getParsePos() const;
		bool								isChunked();
		std::string	getSessionID();
		std::string	getUserName();
		bool	getCookieStatus(std::string &id);
		void	setCookie(std::string &session_id, bool status, std::string username);

		void	checkCookie(std::string &value, bool &cookie_found);
		void	makeNewCookie();

		friend std::ostream &operator<<(std::ostream &os, Request &request); //double check that we're allowed to use friend keyword

		class RequestException : public std::runtime_error
		{
			RequestException(std::string error);
		};

	private:
		std::string 						_method;
		std::string 						_path;
		std::string 						_query;
		std::string 						_version;
		// bool 									_autoindex;
		std::map<std::string, std::string>	_headers;
		int 								_content_length;
		std::string 						_body;
		int 								_code;
		bool 								_chunked;
		size_t 								_parse_pos;
		//std::map<std::string, std::string>	_uploadDir;
		std::string	_errorPage;
		std::string	_sessionID;
		Server 								*_server;
		Cluster	*_cluster;

		Request(Request &other);
		Request& operator=(Request &other);
};

#endif
