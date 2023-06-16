CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(CMAKE_CXX_STANDARD 20)

FIND_PACKAGE(Catch2 CONFIG REQUIRED)

IF (CNT_THEME_TESTING)
    SET(Qt6_Components
            Core
            Gui
            Widgets
            Charts
            Sql
            WebSockets)

    IF (DEFINED CENTAUR_ENV_DETECTED)
        MESSAGE(STATUS "GeneralTests: Qt6 package in environment variable")
        FIND_PACKAGE(Qt6 COMPONENTS
                ${Qt6_Components}
                REQUIRED
                PATHS ${CENTAUR_ENV_DETECTED})
    ELSE ()
        MESSAGE(STATUS "GeneralTests: Qt6 package in default paths")
        FIND_PACKAGE(Qt6 COMPONENTS
                ${Qt6_Components}
                REQUIRED)
    ENDIF ()
ENDIF ()

SET(SOURCE_FILES main.cpp tests.cpp theme_parser.cpp generated/general.h.in)

ADD_EXECUTABLE(tests
        ${SOURCE_FILES})

IF (UNIX AND NOT APPLE)
    MESSAGE(STATUS "TESTS: Linux Detected")
ELSEIF (WIN32)
    MESSAGE(STATUS "TESTS: Windows Detected")
    SET(CENTAUR_EXECUTABLE WIN32)
ELSEIF (UNIX AND APPLE)
    MESSAGE(STATUS "TESTS: Apple Detected")
    SET(CMAKE_MACOSX_RPATH 1)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_BINARY_DIR}/lib")
ELSE ()
    message(FATAL_ERROR "TESTS: Unknown target platform")
ENDIF ()


TARGET_INCLUDE_DIRECTORIES(tests PRIVATE
        ../../Library/uuid/include
        ../../Library/Protocol/include
        ../../include)


TARGET_LINK_LIBRARIES(tests PRIVATE Catch2::Catch2WithMain)
TARGET_LINK_LIBRARIES(tests PRIVATE ${CMAKE_BINARY_DIR}/lib/libProtocol.dylib)
TARGET_LINK_LIBRARIES(tests PRIVATE ${CMAKE_BINARY_DIR}/lib/libuuid.dylib)

IF (CNT_THEME_TESTING)
    TARGET_COMPILE_DEFINITIONS(tests PRIVATE
            USE_THEME_TESTING)

    SET(THEME_TESTING_LOCAL_FILES "${PROJECT_SOURCE_DIR}/tests/general/theme")

    CONFIGURE_FILE(generated/general.h.in auto_theme/general.h @ONLY)

    TARGET_INCLUDE_DIRECTORIES(tests SYSTEM PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

    TARGET_INCLUDE_DIRECTORIES(tests
            PRIVATE
            ${CENT_THEME_INCLUDE_PATH})

    TARGET_LINK_LIBRARIES(tests PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Widgets
            CentTheme)

    ADD_DEPENDENCIES(tests CentTheme)
ENDIF ()


IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Control clang warnings
    SET(COMPILER_OPTIONS "-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-reserved-identifier -Wno-poison-system-directories ")

    SET_SOURCE_FILES_PROPERTIES(
            ${SOURCE_FILES}
            PROPERTIES
            COMPILE_FLAGS "${COMPILER_OPTIONS}")


    MESSAGE(STATUS "Tests: Clang detected - Warning Level Set")
ENDIF ()

