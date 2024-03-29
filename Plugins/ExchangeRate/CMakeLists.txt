CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

FIND_PACKAGE(cpr REQUIRED)
FIND_PACKAGE(FMT CONFIG REQUIRED)
FIND_PACKAGE(RapidJSON REQUIRED)

SET(Qt6_Components
        Gui
        LinguistTools
        Widgets)

IF (DEFINED CENTAUR_ENV_DETECTED)
    MESSAGE(STATUS "ExchangeRate: Qt6 package in environment variable")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED
            PATHS ${CENTAUR_ENV_DETECTED} NO_DEFAULT_PATH)
ELSE ()
    MESSAGE(STATUS "ExchangeRate: Qt6 package in default paths")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED)
ENDIF ()

SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)


SET(PROJECT_FILES
        include/ExchangeRate.hpp
        src/ExchangeRate.cpp
        src/SettingsWidget.cpp
        include/SettingsWidget.hpp)

SET(RESOURCE_FILES
        ui/SettingsWidget.ui
        res/resources.qrc)

ADD_LIBRARY(ExchangeRate SHARED
        ${PROJECT_FILES}
        ${RESOURCE_FILES}
        )


TARGET_INCLUDE_DIRECTORIES(ExchangeRate PRIVATE
        include
        ../../Library/cui/include
        ../../Library/Protocol/include
        ../../Library/uuid/include
        ${CENT_GLOBAL_INCLUDE_PATH})

TARGET_LINK_LIBRARIES(ExchangeRate PRIVATE Qt6::Gui)
TARGET_LINK_LIBRARIES(ExchangeRate PRIVATE Qt6::Widgets)
TARGET_LINK_LIBRARIES(ExchangeRate PRIVATE fmt::fmt)
TARGET_LINK_LIBRARIES(ExchangeRate PRIVATE cpr::cpr)

TARGET_LINK_LIBRARIES(ExchangeRate PRIVATE cui)
TARGET_LINK_LIBRARIES(ExchangeRate PRIVATE uuid)
TARGET_LINK_LIBRARIES(ExchangeRate PRIVATE Protocol)


IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Control clang warnings
    SET(COMPILER_OPTIONS " -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-reserved-identifier -Wno-poison-system-directories ")

    SET_SOURCE_FILES_PROPERTIES(
            ${PROJECT_FILES}
            PROPERTIES
            COMPILE_FLAGS "${COMPILER_OPTIONS}")

    MESSAGE(STATUS "ExchangeRate: Clang detected - Warning Level Set")
ENDIF ()





