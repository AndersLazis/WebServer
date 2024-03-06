# Variables
NAME						:= webserv
VALGRIND_NAME   := webserv_valgrind
CXX							:= c++
RM							:= rm -f

# Directories
VALGRIND_OBJDIR     := build_valgrind
OBJDIR							:= build
SRCDIR							:= sources
INCLUDES_DIR				:= includes

# Include paths
INCLUDES		:= -I ${INCLUDES_DIR}

# Source files for the logger
SRC_LOGGER := $(addprefix logger/, \
		Logger.cpp \
		FileDestination.cpp \
		ConsoleDestination.cpp \
		LogLevel.cpp \
)

# Source files for the config parser
CONFIG_PARSER := $(addprefix configParser/, \
    checkChar.cpp \
    ConfigParser.cpp \
    parsingStateHandlers.cpp \
    processKeyValuePair.cpp \
    validateValues.cpp \
)


# Source files for request parser
REQUEST := $(addprefix request/, \
    parserUtils.cpp \
    parsingStateHandlers.cpp \
    RequestParser.cpp \
		UriParser.cpp \
)

# All source files
SRCS := \
    main.cpp \
		${SRC_LOGGER} \
		${CONFIG_PARSER} \
		${REQUEST} \
		Address.cpp \
		StrContainer.cpp \
		CGI.cpp \
		CGIHandler.cpp \
		CGIReceiver.cpp \
		CGISender.cpp \
		Channel.cpp \
		Connection.cpp \
		Data.cpp \
		ErrorHandler.cpp \
		HttpCodes.cpp \
		Method.cpp \
		FileTypes.cpp \
		RedirectHandler.cpp \
		Request.cpp \
		RequestReceiver.cpp \
		ResponseSender.cpp \
		Route.cpp \
		Server.cpp \
		StaticHandler.cpp \
		URL.cpp \
		Manager.cpp \
		Manager_epoll.cpp \


# Object files corresponding to each source file
OBJS := $(patsubst %.cpp, ${OBJDIR}/%.o, ${SRCS})
VALGRIND_OBJS := $(patsubst %.cpp, ${VALGRIND_OBJDIR}/%.o, ${SRCS})

# ---Compiler Flags---
# Production flags
CXXFLAGS := -Wall -Wextra -Werror
CXXFLAGS += -std=c++98 #-pedantic
# CXXFLAGS += -O2


# Generating dependencies for user headers automaticly
CXXFLAGS += -MMD 


#-------Valgrind---------#

# Valgrind flags
VALGRIND_CXXFLAGS := -Wall -Wextra -Werror
VALGRIND_CXXFLAGS += -std=c++98 #-pedantic
VALGRIND_CXXFLAGS += -g -O0

# Generating dependencies for user headers automaticly
VALGRIND_CXXFLAGS += -MMD 

# Rules
all: ${NAME}

${NAME}: ${OBJS}
	${CXX} ${CXXFLAGS} ${INCLUDES} ${OBJS} -o ${NAME}

${OBJDIR}/%.o: ${SRCDIR}/%.cpp | ${OBJDIR}
	mkdir -p $(dir $@)
	${CXX} ${CXXFLAGS} ${INCLUDES} -c $< -o $@

${OBJDIR}:
	mkdir -p ${OBJDIR}

clean:
	${RM} -r ${OBJDIR}

fclean: clean
	${RM} ${NAME}

re: fclean all

# adding the dependcies as rules and not complainging (-) if file not existent
-include ${OBJS:.o=.d}

${VALGRIND_NAME}: ${VALGRIND_OBJS}
	${CXX} ${VALGRIND_CXXFLAGS} ${INCLUDES} ${VALGRIND_OBJS} -o ${VALGRIND_NAME}

${VALGRIND_OBJDIR}/%.o: ${SRCDIR}/%.cpp | ${VALGRIND_OBJDIR}
	mkdir -p $(dir $@)
	${CXX} ${VALGRIND_CXXFLAGS} ${INCLUDES} -c $< -o $@

${VALGRIND_OBJDIR}:
	mkdir -p ${VALGRIND_OBJDIR}

valgrind: ${VALGRIND_NAME}
	@echo "Running Valgrind. Use CONFIG_PATH=/path/to/config to specify a config path."
	valgrind --leak-check=full --track-origins=yes ./${VALGRIND_NAME} ${CONFIG_PATH}

clean_valgrind:
	${RM} -r ${VALGRIND_OBJDIR}

fclean_valgrind: clean_valgrind
	${RM} ${VALGRIND_NAME}

re_valgrind: fclean_valgrind ${VALGRIND_NAME}

# adding the dependcies as rules and not complainging (-) if file not existent
-include ${VALGRIND_OBJS:.o=.d}

.PHONY: all clean fclean re valgrind clean_valgrind fclean_valgrind re_valgrind
.NOTPARALLEL: