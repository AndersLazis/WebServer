#ifndef CONFIG_PARSER_HPP_INCLUDED
# define CONFIG_PARSER_HPP_INCLUDED

class WebServerConfig;
class LocationConfig;
class ServerConfig;
class Server;
class Route;
class Manager;

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <iostream>


class ConfigParser
{
    public:
        //ConfigParser(Manager & manager);
        ConfigParser();
        // ConfigParser(const ConfigParser& src);
        // ConfigParser&           operator=(const ConfigParser& rhs);
        ~ConfigParser();

        // const WebServerConfig&  getConfig(void) const;
        void                    parseConfig(const std::string& filePath);
        void           _printconfig();
         std::vector<Server>	getServers() const;
           void				setEnv(char **env);
           char** getEnv() const;

    private:
        enum States
        {
            STATE_WS, // white space -> ' ' or '\t' or '\n' or '\r'
            STATE_KEY,
            STATE_OWS, // optional white space -> ' ' or '\t'
            STATE_VALUE,
            STATE_LOCATION,
            STATE_COMMENT,
            STATE_COUNT
        };

        static const int BUFFER_SIZE = 4000;
        static const int MAX_KEY_LENGTH = 100;
        static const int MAX_VALUE_LENGTH = 1000;

        typedef void (ConfigParser::*_stateHandler)(char);
        _stateHandler parsingStateHandlers[6];

        typedef void (ConfigParser::*_processValueFunctions)(void);
        std::vector<Server>	servers;
         //Manager&            _config;                 // not owned by this class
         Server*               _currentServerConfig;     // NULL at start
        Route*               _currentLocationConfig;   // NULL at start
        int                         _parsingState;
        int                         _oldParsingState;
        std::string                 _key;
        std::string                 _value;
        std::vector<std::string>    _mulitValues;             // holds multiple values for one key
        size_t                      _paramterLength;
        char                        _lastChar;                // '\0' at start
        bool                        _isQuoteMode;             // false at start
        size_t                      _lineCount;               // for error messages
        size_t                      _charCount;               // for error messages


        char				**env;
        // //allowed keys, key counter and key value processing functions
         std::map<std::string,
                  std::pair<int, _processValueFunctions> > _httpKeys;
         std::map<std::string,
                  std::pair<int, _processValueFunctions> > _serverKeys;
        std::map<std::string,
                  std::pair<int, _processValueFunctions> > _locationKeys;


        void            _throwConfigError(const std::string& message,
                                          char offendingChar,
                                          const std::string& offendingString,
                                          bool givePosition);

        // //parsing
        bool            _isFileNameValid(const std::string& filePath);
        void            _parseOneChar(char c);

        // //parsing state handlers
         void            _changeState(int current, int next);
         void	        _handleStateWs(char c);
         void            _handleStateKey(char c);
         void            _handleStateOws(char c);
        void	        _handleStateValue(char c);
        void            _handleStateComment(char c);
        void            _handleStateLocation(char c);

        // //storing key and value in buffer
        void            _addCharToKey(char c);
        void            _addCharToValue(char c);

        // //check chars for parsing
        bool            _isAllowedWs(char c);
        bool            _isAllowedOws(char c);
        bool            _isCommentStart(char c);
        bool            _isQuoteStart(char c);
        bool            _isAllowedKeyChar(char c);
        bool            _isAllowedValueChar(char c);
        bool            _toggleQuoteMode(char c);
        bool            _isUnescapedChar(char expected, char actual);

        // //key value pair processing
        void	        _processKeyValuePair(void);
        void            _validateAndHandleKey(void);
        void            _validateKeyAndCallHandler(std::map<std::string,
                                                   std::pair<int, _processValueFunctions> >& keys);
        void            _resetKeyCounts(std::map<std::string,
                                        std::pair<int, _processValueFunctions> >& keys);

        // //validating and storing values
        std::string&    _extractSingleValueFromValueVector(const bool isRequired);
        void	        _processClientMaxBodySize();
        void	        _processListen();
        void	        _processServerName();
        void            _processErrorPage();
         void            _processDefaultErrorPage();
         void            _processRoot();
         void            _processLocationPath();
         void            _processIndex();
         void            _processCgiExtension();
         void            _processUploadStore();
         void            _processReturn();
         void            _processMethods();
         void            _processAutoindex();

        // //validating and storing values helpers
        // uint32_t        _ipStringToNumber(const std::string& ip);
        // uint16_t        _ip_port_to_uint16(const std::string& ip_port) ;
        // uint16_t        _stringToUint16(const std::string& str);

        // uint16_t        extractPort(std::istringstream& iss);
        // uint32_t        extractIp(std::istringstream& iss);
        // //validating config blocks
        // // bool        isServerBlockValid();
        // // bool        isLocationBlockValid();
        void            _validateLocationConfig(Route* currentLocationConfig);
        void            _validateServerConfig(Server* currentServerConfig);
        void            _addServerConfig(Server* currentServerConfig);
        void            _addLocationConfig(Route* currentLocationConfig);

      
       

        
};

#endif /* CONFIG_PARSER_HPP_INCLUDED */
