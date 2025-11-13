#ifndef LIBRARIES_HPP
#define LIBRARIES_HPP

// Standard C++ Libraries
#include <iostream>     // For input/output operations (e.g., std::cout, std::cin)
#include <fstream>      // For file stream operations (e.g., std::ifstream, std::ofstream)
#include <map>          // For associative containers (e.g., std::map)
#include <exception>    // For exception handling
#include <cstdlib>      // For general utilities (e.g., exit, atoi)
#include <string>       // For string manipulation (e.g., std::string)
#include <vector>       // For dynamic arrays (e.g., std::vector)
#include <sstream>      // For string stream operations (e.g., std::stringstream)
#include <csignal>      // For signal handling (e.g., signal, SIGINT)
#include <cstring>      // For C-style string manipulation (e.g., memset)
#include <cctype>  		// for isxdigit
#include <cstdio>  		// for sscanf
#include <cerrno>
#include <climits>

// System-Specific Libraries
#include <sys/stat.h>   // For file status and permissions (e.g., stat, mkdir)
#include <sys/types.h>  // For system data types (e.g., mode_t, pid_t)
#include <sys/socket.h> // For socket programming (e.g., socket, bind, listen, accept)
#include <netinet/in.h> // For internet address family (e.g., sockaddr_in)
#include <unistd.h>     // For POSIX operating system API (e.g., close, read, write)
#include <poll.h>       // For monitoring multiple file descriptors (e.g., pollfd)
#include <fcntl.h>      // For file control options (e.g., fcntl, O_NONBLOCK)
#include <arpa/inet.h>  // For IP address manipulation (e.g., inet_aton, inet_ntoa)
#include <dirent.h>       // For opendir, eaddir and closedir.
#include <ctime>

// Custom Header
#include "Utils.hpp"    // Your custom utility header file

#define POLL_TIME_OUT 100		// milliseconds
#define CLIENT_TIMEOUT 90
#define BUFFER_SIZE 1024

enum RequestState {
		REQUEST_COMPLETE,           // Request fully received and parsed
		REQUEST_INCOMPLETE,         // Waiting for more data (e.g., full body/chunks)
		REQUEST_ERROR_RESPOND,      // Request had an error (e.g., 400, 413) but needs a response
		REQUEST_ERROR_CLOSE_CONN    // Fatal error requiring immediate connection close
	};

	struct FileEntry {
		std::string name;
		bool is_directory;
		std::string last_modified;
		std::string size_str;
	};
#endif
