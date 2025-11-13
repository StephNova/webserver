
#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <limits.h>
#include "Request.hpp"
#include "Utils.hpp"

class Response {
	public:
		Response(Request *request, Server *server, std::string &hostName);
		~Response();

		void 		setCode(int code);
		int 		getCode();
		std::string process_request();
		std::string responseBuilder();
		std::string	headersBuilder();
		void		bodyBuilder();
		void		POSTBodyBuilder();

		void		handleERROR(int statusCode);
		void		handleGET();
		void		handlePOST();
		void		handleDELETE();
		void		handleCGI(std::string &uri);
		void 		assign_status_phrase();

		void 		sendResponse(int client_fd);
		void 		cgiExecuter(std::string path, const std::string &query);
		void		redirect(std::string path);
		void 		parseCGIOutput(const std::string &output);
		std::string getMimeType(const std::string &path);
		bool 		isCGIdir(const std::string &path);
		bool		isUploadsDir(const std::string &path);

		bool		isAutoindex(const std::string &path);
		void		autoindexBuilder(const std::string &path, const std::vector<FileEntry>& entries);
		std::vector<FileEntry> getDirectoryEntries(const std::string& path);

	private:
		Request *_request;
		Server 	*_server;
		int _code;
		std::map<std::string, std::string> _status;
		std::map<std::string, std::string> _headers;
		std::string _body;

		Response(Response &other);
		Response& operator=(Response &other);
};

#endif //RESPONSE_HPP
