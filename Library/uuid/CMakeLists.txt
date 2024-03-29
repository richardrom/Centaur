CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

SET(CMAKE_CXX_STANDARD 20)


FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(re2 CONFIG REQUIRED)

SET(INCLUDE_FILES
        include/uuid.hpp
        ../../include/Centaur.hpp)

SET(SOURCE_FILES
        src/uuid.cpp)


ADD_LIBRARY(uuid
        SHARED
        ${INCLUDE_FILES}
        ${SOURCE_FILES})

TARGET_COMPILE_DEFINITIONS(uuid PRIVATE BINAPI_LIBRARY)

TARGET_LINK_LIBRARIES(uuid PRIVATE re2::re2)

TARGET_INCLUDE_DIRECTORIES(uuid PRIVATE
        include
        ${CENT_GLOBAL_INCLUDE_PATH})


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


    MESSAGE(STATUS "libUUID: Clang detected - Warning Level Set")
ENDIF ()
