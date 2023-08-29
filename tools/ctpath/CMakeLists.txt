CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_COLOR_DIAGNOSTICS ON)


SET(Qt6_Components Core)

IF (DEFINED CENTAUR_ENV_DETECTED)
    MESSAGE(STATUS "ctpack: Qt6 package in environment variable")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED
            PATHS "${CENTAUR_ENV_DETECTED}" NO_DEFAULT_PATH)
ELSE ()
    MESSAGE(STATUS "ctpack: Qt6 package in default paths")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED)
ENDIF ()


IF (UNIX AND NOT APPLE)
    MESSAGE(STATUS "ctpath: Linux Detected")
ELSEIF (WIN32)
    MESSAGE(STATUS "ctpath: Windows Detected")
    SET(CENTAUR_EXECUTABLE WIN32)
ELSEIF (UNIX AND APPLE)
    MESSAGE(STATUS "ctpath: Apple Detected")
    SET(CENTAUR_EXECUTABLE MACOSX_BUNDLE)
    SET(APPLE_PLATFORM TRUE)
    SET(CMAKE_MACOSX_RPATH 1)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_BINARY_DIR}/lib")
ELSE ()
    MESSAGE(FATAL_ERROR "ctpath: Unknown target platform")
ENDIF ()


ADD_EXECUTABLE(ctpath main.cpp)

TARGET_INCLUDE_DIRECTORIES(ctpath PRIVATE "${CMAKE_BINARY_DIR}/config")


IF (CNT_USE_PCH)
    TARGET_PRECOMPILE_HEADERS(ctpath
            PRIVATE
            <QCoreApplication> <QStandardPaths>)
ENDIF ()


TARGET_LINK_LIBRARIES(ctpath PRIVATE Qt6::Core)


IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Control clang warnings
    SET(COMPILER_OPTIONS " -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-reserved-identifier -Wno-poison-system-directories -Wno-c++20-compat")

    SET_SOURCE_FILES_PROPERTIES(
            main.cpp
            PROPERTIES
            COMPILE_FLAGS " ${COMPILER_OPTIONS} ")

    MESSAGE(STATUS "ctpath: Clang detected - Warning Level Set")
ENDIF ()

