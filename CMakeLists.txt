cmake_minimum_required(VERSION 3.24)

PROJECT(Centaur VERSION 0.1
        DESCRIPTION "Centaur. Trading Application"
        LANGUAGES C CXX)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_COLOR_DIAGNOSTICS ON)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

OPTION(CNT_USE_PCH "Use precompiled headers" ON)
OPTION(CNT_USE_TEST_LOGIN "Do not prompt the login dialog at startup (only for testing purposes)" ON)
OPTION(CNT_AVOID_PLUGIN_CHECK "Do not check the checksum of a plugin at startup (only for testing purposes)" ON)


IF (DEFINED ENV{QT6_INSTALLATION})
    SET(CENTAUR_ENV_DETECTED $ENV{QT6_INSTALLATION})
    MESSAGE(STATUS " QT6 FOUND in environment variable ${CENTAUR_ENV_DETECTED} ")
ELSE ()
    MESSAGE(STATUS " QT6 NOT FOUND in environment variable ")
ENDIF ()


ADD_COMPILE_DEFINITIONS($<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)
ADD_COMPILE_DEFINITIONS($<$<CONFIG:Debug>:DEBUG>)
if (CNT_USE_TEST_LOGIN)
    ADD_COMPILE_DEFINITIONS($<$<CONFIG:Debug>:TEST_LOGIN_MODE>)
ENDIF ()
IF (CNT_AVOID_PLUGIN_CHECK)
    ADD_COMPILE_DEFINITIONS(NO_PLUGIN_CHECKSUM_CHECK)
ENDIF ()


INCLUDE_DIRECTORIES(Plugins/include)

# Main Application
ADD_SUBDIRECTORY(Centaur)

#Clients

#Libraries
ADD_SUBDIRECTORY(Library/uuid)
ADD_SUBDIRECTORY(Library/cui)
ADD_SUBDIRECTORY(Library/Protocol)

#Libraries for exchanges
ADD_SUBDIRECTORY(Library/Exchanges/BinanceAPI)

#Plugins
ADD_SUBDIRECTORY(Plugins/ExchangeRate)
ADD_SUBDIRECTORY(Plugins/BinanceSpot)


#tests
ADD_SUBDIRECTORY(tests/general)
ADD_SUBDIRECTORY(tests/CandleWidgetTest)

#Tools
ADD_SUBDIRECTORY(tools/btrader)
ADD_SUBDIRECTORY(tools/plpack)