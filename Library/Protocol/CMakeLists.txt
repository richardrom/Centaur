cmake_minimum_required(VERSION 3.24)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_COLOR_DIAGNOSTICS ON)


# Required libraries
FIND_PACKAGE(fmt REQUIRED)
FIND_PACKAGE(RapidJson REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(cryptopp CONFIG REQUIRED)

SET(INCLUDE_FILES
        ../../include/Centaur.hpp
        include/Protocol.hpp
        include/ProtocolType.hpp)

SET(SOURCE_FILES
        src/protocol.cpp
        src/generator.cpp
        src/encryption.cpp)

ADD_LIBRARY(Protocol SHARED
        ${INCLUDE_FILES}
        ${SOURCE_FILES})

TARGET_INCLUDE_DIRECTORIES(Protocol PRIVATE
        include
        ${CENT_GLOBAL_INCLUDE_PATH})


TARGET_LINK_LIBRARIES(Protocol PRIVATE fmt::fmt)
TARGET_LINK_LIBRARIES(Protocol PRIVATE OpenSSL::SSL OpenSSL::Crypto)
TARGET_LINK_LIBRARIES(Protocol PRIVATE ZLIB::ZLIB)
TARGET_LINK_LIBRARIES(Protocol PRIVATE cryptopp::cryptopp)


IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Control clang warnings
    SET(COMPILER_OPTIONS " -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-reserved-identifier -Wno-poison-system-directories ")

    SET_SOURCE_FILES_PROPERTIES(
            ${INCLUDE_FILES}
            PROPERTIES
            COMPILE_FLAGS "${COMPILER_OPTIONS}")

    SET_SOURCE_FILES_PROPERTIES(
            ${SOURCE_FILES}
            PROPERTIES
            COMPILE_FLAGS "${COMPILER_OPTIONS}")


    MESSAGE(STATUS "libProtocol: Clang detected - Warning Level Set")
ENDIF ()
