CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

FIND_PACKAGE(FMT REQUIRED)
SET(Qt6_Components
        Gui
        LinguistTools
        Widgets
        StateMachine
        Xml)

IF (DEFINED CENTAUR_ENV_DETECTED)
    MESSAGE(STATUS "CtTheme: Qt6 package in environment variable")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED
            PATHS ${CENTAUR_ENV_DETECTED} NO_DEFAULT_PATH)
ELSE ()
    MESSAGE(STATUS "CtTheme: Qt6 package in default paths")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED)
ENDIF ()

SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)


SET(INCLUDE_FILES
        ${CENT_GLOBAL_INCLUDE_PATH}/ThemeInterface.hpp
        include/ThemeParser.hpp
        include/ThemePlugin.hpp
        include/CentTheme.hpp)

SET(SOURCE_FILES
        src/ThemeParser.cpp
        src/ThemePlugin.cpp
        src/CentTheme.cpp)

SET(RESOURCE_FILES
        qrc/resources.qrc)

ADD_LIBRARY(CentTheme SHARED
        ${INCLUDE_FILES}
        ${SOURCE_FILES}
        ${RESOURCE_FILES}
)

TARGET_COMPILE_DEFINITIONS(CentTheme PRIVATE USE_CENT_LIBRARY)

IF (CNT_THEME_TESTING)
    TARGET_COMPILE_DEFINITIONS(CentTheme PRIVATE USE_THEME_TESTING THEME_USE_EXPORTS)
ENDIF ()

TARGET_INCLUDE_DIRECTORIES(CentTheme PRIVATE
        include
        ${CENT_GLOBAL_INCLUDE_PATH})

TARGET_LINK_LIBRARIES(CentTheme PRIVATE fmt::fmt)
TARGET_LINK_LIBRARIES(CentTheme PRIVATE
        Qt6::Gui
        Qt6::Widgets
        Qt6::StateMachine
        Qt6::Xml)


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

    MESSAGE(STATUS "CtTheme: Clang detected - Warning Level Set")
ENDIF ()


IF (CNT_GENERATE_BUILTIN_PLPACK)
    FIND_PACKAGE(PythonInterp 3 REQUIRED)

    # plpack must be build first otherwise
    ADD_DEPENDENCIES(CentTheme
            plpack)

    SET(LOCAL_PATH "${PROJECT_SOURCE_DIR}/local")
    SET(SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/scripts")

    ADD_CUSTOM_COMMAND(TARGET CentTheme
            POST_BUILD
            COMMAND ${PYTHON_EXECUTABLE}
            ARGS "${SCRIPTS_PATH}/theme-plpack.py" -l "${LOCAL_PATH}" -b "${CMAKE_BINARY_DIR}/bin" -o "${PLPACK_OUTDIR}" -i "${CMAKE_BINARY_DIR}/lib"
            COMMENT Running theme-plpack.py
            VERBATIM)
ENDIF ()



