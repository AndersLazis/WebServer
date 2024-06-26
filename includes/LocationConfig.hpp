#ifndef LOCATION_CONFIG_HPP_INCLUDED
# define LOCATION_CONFIG_HPP_INCLUDED

class CGIConfig;

#include <string>
#include <vector>
#include <bitset>
#include <stdint.h>

class LocationConfig
{
public:

    LocationConfig();
    ~LocationConfig();

    // //location path
    // std::string path; //needed to from the key later // -> only one per block

    // //redirection 
    // //uint16_t statusCode; // 0 if not specified // -> last one in the block counts
    // int         statusCode;
    // std::string targetUrl;

    // CGIConfig*      cgiConfig;  // Include CGIConfig for CGI execution details

    // std::string rootDirectory; // if not specified best of luck finding the file //-> only one per block
    // std::string uploadDirectory; // if not specified respond with error 500 //-> only one per block // -> named "upload_store" in config
    // std::string defaultDirectoryPage; // default file to be served if no file is specified in the path
    // // -> named "index" in config, default is "index.html" // searched for in the directory requested left to right // for now limit to 1 according to subject interpretation
    // //in nginx there can only be one "index" directive per block
    // bool directoryListing; // directory listing will come into play if no default file is specified or present // only one per block
    // // Add more route-specific configurations as needed
    // std::bitset<3> allowedMethods; // 0 = GET, 1 = POST, 2 = DELETE // init to 111 -> all allowed

    // bool isMethodAllowed(int method) const;

    // void    printConfig(bool printCGI) const;
};

#endif /* LOCATION_CONFIG_HPP_INCLUDED */