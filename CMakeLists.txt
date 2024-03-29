cmake_minimum_required(VERSION 3.24)

PROJECT(Centaur VERSION 0.1
        DESCRIPTION "Centaur. Trading Application"
        LANGUAGES C CXX)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_COLOR_DIAGNOSTICS ON)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

SET(CENT_GLOBAL_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include")
SET(CENT_CUI_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/Library/cui/include")
SET(CENT_PROTOCOL_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/Library/Protocol/include")
SET(CENT_UUID_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/Library/uuid/include")

IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    ADD_COMPILE_DEFINITIONS(C_C_CLANG)
ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    ADD_COMPILE_DEFINITIONS(C_C_GNU)
ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    ADD_COMPILE_DEFINITIONS(C_C_MSVC)
ELSE ()
    ADD_COMPILE_DEFINITIONS(C_C_UNKNOWN)
ENDIF ()


OPTION(CNT_USE_PCH "Use precompiled headers" ON)
OPTION(CNT_USE_TEST_LOGIN "Do not prompt the login dialog at startup (only for testing purposes)" ON)
OPTION(CNT_AVOID_PLUGIN_CHECK "Do not check the checksum of a plugin at startup (only for testing purposes)" ON)
OPTION(CNT_GENERATE_BUILTIN_PLPACK "Generate plpack for builtin plugins including themes" ON)
OPTION(CNT_COPY_BUILTIN_PLUGIN " For testing purposes this will run scripts/builtin.py" ON)
OPTION(CNT_COPY_THEME_PLUGIN "For testing purposes this will run scripts/theme.py" ON)
OPTION(CNT_THEME_TESTING "Include theme testing" ON)


IF (DEFINED ENV{QT6_INSTALLATION})
    SET(CENTAUR_ENV_DETECTED $ENV{QT6_INSTALLATION})
    MESSAGE(STATUS "QT6 FOUND in environment variable ${CENTAUR_ENV_DETECTED} ")
ELSE ()
    MESSAGE(STATUS "QT6 NOT FOUND in environment variable ")
ENDIF ()


ADD_COMPILE_DEFINITIONS($<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)
ADD_COMPILE_DEFINITIONS($<$<CONFIG:Debug>:DEBUG>)
if (CNT_USE_TEST_LOGIN)
    ADD_COMPILE_DEFINITIONS($<$<CONFIG:Debug>:TEST_LOGIN_MODE>)
ENDIF ()
IF (CNT_AVOID_PLUGIN_CHECK)
    ADD_COMPILE_DEFINITIONS(NO_PLUGIN_CHECKSUM_CHECK)
ENDIF ()


# Naming configuration in order that ctpath and centaur does not mismatch names
# So, the internal testing installation can find the necessary files
SET(NAMING_ORGANIZATION_NAME "CentaurProject")
SET(NAMING_ORGANIZATION_DOMAIN "centaur.com")
SET(NAMING_APPLICATION_NAME "Centaur")
CONFIGURE_FILE(cmake/core-naming.hpp.in config/core-naming.hpp @ONLY)

IF (CNT_GENERATE_BUILTIN_PLPACK OR CNT_COPY_BUILTIN_PLUGIN OR CNT_COPY_THEME_PLUGIN)
    SET(CENT_LOCAL_PATH "${PROJECT_SOURCE_DIR}/local")
    IF (CNT_GENERATE_BUILTIN_PLPACK)
        SET(PLPACK_OUTDIR "${CMAKE_BINARY_DIR}/plpack")
        MESSAGE(STATUS "Creating: ${PLPACK_OUTDIR} ")
        FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/plpack)
    ENDIF ()
    ADD_SUBDIRECTORY(tools/ctpath)
ENDIF ()


SET(CENT_THEME_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/Themes/MainTheme/include")


INCLUDE_DIRECTORIES(Plugins/include)

# Main Application
ADD_SUBDIRECTORY(Centaur)

# Clients
ADD_SUBDIRECTORY(tools/btrader)

# Libraries
ADD_SUBDIRECTORY(Library/uuid)
ADD_SUBDIRECTORY(Library/cui)
ADD_SUBDIRECTORY(Library/Protocol)

# Tools
ADD_SUBDIRECTORY(tools/plpack)

# Libraries for exchanges
ADD_SUBDIRECTORY(Library/Exchanges/BinanceAPI)

# Plugins
ADD_SUBDIRECTORY(Plugins/ExchangeRate)
ADD_SUBDIRECTORY(Plugins/BinanceSpot)

# Themes
ADD_SUBDIRECTORY(Themes/MainTheme)

# tests
ADD_SUBDIRECTORY(tests/general)
ADD_SUBDIRECTORY(tests/CandleWidgetTest)
ADD_SUBDIRECTORY(tests/ThemeTests)



