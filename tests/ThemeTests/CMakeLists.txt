CMAKE_MINIMUM_REQUIRED(VERSION 3.14)


SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(Qt6_Components
        Core
        Gui
        Widgets
        Charts
        Sql
        WebSockets)

IF (DEFINED CENTAUR_ENV_DETECTED)
    MESSAGE(STATUS "ThemeTests: Qt6 package in environment variable")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED
            PATHS ${CENTAUR_ENV_DETECTED})
ELSE ()
    MESSAGE(STATUS "ThemeTests: Qt6 package in default paths")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED)
ENDIF ()

SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)


SET(SOURCE_FILES
        main.cpp
        TestApplication.cpp)

SET(INCLUDE_FILES
        TestApplication.hpp)

SET(UI_FILES
        window.ui)


ADD_EXECUTABLE(ThemeTest
        ${INCLUDE_FILSE}
        ${SOURCE_FILES}
        ${UI_FILES}
        )


TARGET_INCLUDE_DIRECTORIES(ThemeTest PRIVATE
        ${CENT_CUI_INCLUDE_PATH}
        ${CENT_GLOBAL_INCLUDE_PATH}
        )

ADD_DEPENDENCIES(ThemeTest
        CentTheme
        cui)

TARGET_LINK_LIBRARIES(ThemeTest PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets)

TARGET_LINK_LIBRARIES(ThemeTest PRIVATE cui)


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


    MESSAGE(STATUS "ThemeTests: Clang detected - Warning Level Set")
ENDIF ()

