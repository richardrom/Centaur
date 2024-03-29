CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_COLOR_DIAGNOSTICS ON)

FIND_PACKAGE(SQLite3 REQUIRED)
FIND_PACKAGE(RapidJSON CONFIG REQUIRED)
FIND_PACKAGE(libzip CONFIG REQUIRED)

if (CMAKE_BUILD_TYPE EQUAL "Debug")
    FIND_LIBRARY(QRENCODE_LIBRARY_DEBUG qrencoded)
    SET(QRENCODE_LIBRARIES debug ${QRENCODE_LIBRARY_DEBUG})
else ()
    FIND_LIBRARY(QRENCODE_LIBRARY_RELEASE qrencode)
    SET(QRENCODE_LIBRARIES ${QRENCODE_LIBRARY_RELEASE})
endif ()

SET(Qt6_Components
        Core
        Gui
        Widgets
        Charts
        Sql
        WebSockets
        Concurrent
        StateMachine)

IF (DEFINED CENTAUR_ENV_DETECTED)
    MESSAGE(STATUS "Centaur: Qt6 package in environment variable")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED
            PATHS "${CENTAUR_ENV_DETECTED}" NO_DEFAULT_PATH)

    MESSAGE(STATUS "Centaur QT Version: ${Qt6_VERSION_MAJOR}.${Qt6_VERSION_MINOR}.${Qt6_VERSION_PATCH}")
ELSE ()
    MESSAGE(STATUS "Centaur: Qt6 package in default paths")
    FIND_PACKAGE(Qt6 COMPONENTS
            ${Qt6_Components}
            REQUIRED)
ENDIF ()

SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)

SET(CENT_SOURCE
        src/main.cpp
        src/CentaurApp.cpp
        src/Logger.cpp
        src/Globals.cpp
        src/Plugins.cpp
        src/FavoritesDialog.cpp
        src/ConfigurationInterface.cpp
        src/ProtocolClient.cpp
        src/CandleViewWidget.cpp
        src/ProtocolServer.cpp
        src/LogDialog.cpp
        src/SplashDialog.cpp
        src/LoginDialog.cpp
        src/AddUserDialog.cpp
        src/SettingsDialog.cpp
        src/QRCodeDialog.cpp
        src/TOTP.cpp
        src/OrderbookDialog.cpp
        src/DepthChartDialog.cpp
        src/WatchlistWidget.cpp
        src/WatchlistModel.cpp
        src/PortfolioWidget.cpp
        src/PortfolioModel.cpp
        src/CandleChartDialog.cpp
        src/PixmapMemCacheDialog.cpp
        src/DAL.cpp
        src/SettingsShortcutsPage.cpp
        src/SettingsAdvancedPage.cpp
        src/SettingsPluginsPage.cpp
        src/SettingsSessionPage.cpp
        src/ShortcutDialog.cpp)

SET(CENT_INCLUDE
        include/CentaurApp.hpp
        include/CentaurGlobal.hpp
        include/CentaurUIState.hpp
        include/Logger.hpp
        include/Globals.hpp
        include/FavoritesDialog.hpp
        include/ConfigurationInterface.hpp
        include/crc64.hpp
        include/ProtocolServer.hpp
        include/CandleViewWidget.hpp
        include/ProtocolClient.hpp
        include/CandleViewWidget.hpp
        ../include/CentaurPlugin.hpp
        ../include/CentaurInterface.hpp
        ../include/ThemeInterface.hpp
        include/LogDialog.hpp
        include/SplashDialog.hpp
        include/AddUserDialog.hpp
        include/SettingsDialog.hpp
        include/LoginDialog.hpp
        include/QRCodeDialog.hpp
        include/TOTP.hpp
        include/OrderbookDialog.hpp
        include/DepthChartDialog.hpp
        include/WatchlistWidget.hpp
        include/WatchListModel.hpp
        include/PortfolioModel.hpp
        include/PortfolioWidget.hpp
        include/CandleChartDialog.hpp
        include/PixmapMemCacheDialog.hpp
        include/DAL.hpp
        include/SettingsDialogData.hpp
        include/ShortcutDialog.hpp)

SET(CENT_UI
        ui/CentaurApp.ui
        ui/FavoritesDialog.ui
        ui/CandleViewWidget.ui
        ui/LogDialog.ui
        ui/LoginDialog.ui
        ui/SettingsDialog.ui
        ui/AddUserDialog.ui
        ui/SplashDialog.ui
        ui/QRCodeDialog.ui
        ui/PixmapMemCacheDialog.ui)

SET(CENT_RESOURCE
        qrc/resource.qrc)

IF (UNIX AND NOT APPLE)
    MESSAGE(STATUS "Centaur: Linux Detected")
ELSEIF (WIN32)
    MESSAGE(STATUS "Centaur: Windows Detected")
    SET(CENTAUR_EXECUTABLE WIN32)
ELSEIF (UNIX AND APPLE)
    MESSAGE(STATUS "Centaur: Apple Detected")
    SET(CENTAUR_EXECUTABLE MACOSX_BUNDLE
            ../cmake/core-naming.hpp.in)
    SET(APPLE_PLATFORM TRUE)
    SET(CMAKE_MACOSX_RPATH 1)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_BINARY_DIR}/lib")
ELSE ()
    MESSAGE(FATAL_ERROR "Centaur: Unknown target platform")
ENDIF ()

LIST(APPEND ICON_FILE
        ${CMAKE_SOURCE_DIR}/local/cent-icon.icns)

LIST(APPEND REPAIR_FILES
        ${CMAKE_SOURCE_DIR}/local/logdb.sql
        ${CMAKE_SOURCE_DIR}/local/cdb.sql)

LIST(APPEND IMAGE_FILES
        ${CMAKE_SOURCE_DIR}/local/crp_img.zip)

LIST(APPEND EXTRA_FILES
        ${CMAKE_SOURCE_DIR}/local/plugins.schema.json
        ${CMAKE_SOURCE_DIR}/local/keymap.schema.json)

LIST(APPEND KEYMAP_FILES
        ${CMAKE_SOURCE_DIR}/local/macos.keymap.json
        ${CMAKE_SOURCE_DIR}/local/windows.keymap.json
        ${CMAKE_SOURCE_DIR}/local/linux.keymap.json)

LIST(APPEND FONT_FILES
        "${CMAKE_SOURCE_DIR}/local/Inter-VariableFont_slnt,wght.ttf")

ADD_EXECUTABLE(Centaur ${CENTAUR_EXECUTABLE}
        ${CENT_RESOURCE}
        ${CENT_UI}
        ${CENT_INCLUDE}
        ${CENT_SOURCE}
        ${REPAIR_FILES}
        ${IMAGE_FILES}
        ${ICON_FILE}
        ${EXTRA_FILES}
        ${KEYMAP_FILES}
        ${FONT_FILES})

ADD_DEPENDENCIES(Centaur
        BinanceSPOT
        ExchangeRate
        CentTheme)

IF (CNT_USE_PCH)
    TARGET_PRECOMPILE_HEADERS(Centaur
            PRIVATE
            <QAction> <QDir> <QDirIterator> <QMenu> <QMessageBox> <QSortFilterProxyModel> <QStandardPaths> <QAreaSeries>
            <QChartView> <QDateTimeAxis> <QGraphicsBlurEffect> <QImageReader> <QResizeEvent> <QShortcut> <QSqlRecord> <QFileInfo>
            <QSettings> <QDialog> <QFile> <QWidget> <QLabel> <QCoreApplication> <QTextStream> <QVariant> <QPixmap> <QIcon>
            <QFrame> <QHBoxLayout> <QMainWindow> <QPushButton> <QScrollArea> <QSpacerItem> <QSplitter> <QStackedWidget>
            <QToolButton> <QVBoxLayout> <QKeySequenceEdit>
            <rapidjson/document.h>
            <rapidjson/ostreamwrapper.h>
            <rapidjson/writer.h>
            <map> <vector> <iostream> <list> <limits> <unordered_map> <algorithm> <concepts> <fstream>)
ENDIF ()


# Create the Apple Bundle
IF (${APPLE_PLATFORM})

    SET_TARGET_PROPERTIES(Centaur PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME Centaur
            MACOSX_BUNDLE_GUI_IDENTIFIER com.centaur-project.Centaur
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            MACOSX_BUNDLE_ICON_FILE cent-icons.icns
    )

    SET_SOURCE_FILES_PROPERTIES(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/")
    SET_SOURCE_FILES_PROPERTIES(${REPAIR_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/Repair/")
    SET_SOURCE_FILES_PROPERTIES(${IMAGE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/Images/")
    SET_SOURCE_FILES_PROPERTIES(${EXTRA_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/Schema/")
    SET_SOURCE_FILES_PROPERTIES(${KEYMAP_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/Keymap/")
    SET_SOURCE_FILES_PROPERTIES(${FONT_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/Fonts/")
ENDIF ()


IF (${APPLE_PLATFORM})
    # Remove all builtin plugins from the Bundle/Plugins directory
    LIST(APPEND BUILTIN_PLUGINS_FILES
            libExchangeRate.dylib
            libBinanceSPOT.dylib)

    SET(PLUGINS_BUNDLE_PATH "${CMAKE_BINARY_DIR}/bin/Centaur.app/Contents/Plugins")
    FOREACH (fls IN ZIP_LISTS BUILTIN_PLUGINS_FILES)
        FIND_FILE(CURRENT_FOUND
                ${fls}
                PATHS ${PLUGINS_BUNDLE_PATH}
                NO_CACHE)
        IF (NOT ${CURRENT_FOUND} STREQUAL "CURRENT_FOUND-NOTFOUND")
            MESSAGE(STATUS "Plugin will be temporarily removed: ${fls}")
            ADD_CUSTOM_COMMAND(TARGET Centaur
                    PRE_BUILD
                    COMMAND ${CMAKE_COMMAND}
                    ARGS -E rm "${PLUGINS_BUNDLE_PATH}/${fls}"
                    VERBATIM)
        ENDIF ()
    ENDFOREACH ()
ENDIF ()


TARGET_INCLUDE_DIRECTORIES(Centaur PRIVATE "${CMAKE_BINARY_DIR}/config")

TARGET_INCLUDE_DIRECTORIES(Centaur PRIVATE
        include
        ${CENT_PROTOCOL_INCLUDE_PATH}
        ${CENT_CUI_INCLUDE_PATH}
        ${CENT_UUID_INCLUDE_PATH}
        ${CENT_THEME_INCLUDE_PATH}
        ${CENT_GLOBAL_INCLUDE_PATH})

TARGET_LINK_LIBRARIES(Centaur PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Charts
        Qt6::Sql
        Qt6::WebSockets
        Qt6::Concurrent
        Qt6::StateMachine)
TARGET_LINK_LIBRARIES(Centaur PRIVATE SQLite3)
TARGET_LINK_LIBRARIES(Centaur PRIVATE rapidjson)
TARGET_LINK_LIBRARIES(Centaur PRIVATE ${QRENCODE_LIBRARIES})
TARGET_LINK_LIBRARIES(Centaur PRIVATE libzip::zip)

# Local Libraries
TARGET_LINK_LIBRARIES(Centaur PRIVATE Protocol)
TARGET_LINK_LIBRARIES(Centaur PRIVATE uuid)
TARGET_LINK_LIBRARIES(Centaur PRIVATE CentTheme)
TARGET_LINK_LIBRARIES(Centaur PRIVATE cui)


IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Control clang warnings
    SET(COMPILER_OPTIONS " -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-reserved-identifier -Wno-poison-system-directories -Wno-c++20-compat")

    SET_SOURCE_FILES_PROPERTIES(
            ${CENT_INCLUDE}
            PROPERTIES
            COMPILE_FLAGS " ${COMPILER_OPTIONS} ")

    SET_SOURCE_FILES_PROPERTIES(
            ${CENT_SOURCE}
            PROPERTIES
            COMPILE_FLAGS " ${COMPILER_OPTIONS} ")

    MESSAGE(STATUS "Centaur: Clang detected - Warning Level Set")
ENDIF ()

IF (${APPLE_PLATFORM})
    # Copy the builtin plugins to the bundle
    FOREACH (fls IN LISTS BUILTIN_PLUGINS_FILES)
        ADD_CUSTOM_COMMAND(TARGET Centaur
                POST_BUILD
                COMMAND ${CMAKE_COMMAND}
                ARGS -E copy_if_different "${CMAKE_BINARY_DIR}/lib/${fls}" "${PLUGINS_BUNDLE_PATH}/${fls}"
                VERBATIM)
    ENDFOREACH ()
ENDIF ()

IF (CNT_COPY_BUILTIN_PLUGIN OR CNT_COPY_THEME_PLUGIN)
    # In this case ctpath must be build
    ADD_DEPENDENCIES(Centaur
            ctpath)

    SET(LOCAL_PATH "${PROJECT_SOURCE_DIR}/local")
    SET(SCRIPTS_PATH "${PROJECT_SOURCE_DIR}/scripts")

    SET(CTPATH_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
ENDIF ()

IF (CNT_COPY_THEME_PLUGIN)
    # Copy the theme plugin to the path
    FIND_PACKAGE(PythonInterp 3 REQUIRED)

    # This script will copy CentTheme to the themes path
    ADD_CUSTOM_COMMAND(TARGET Centaur
            POST_BUILD
            COMMAND ${PYTHON_EXECUTABLE}
            ARGS "${SCRIPTS_PATH}/cttheme-copy.py" -c "${CTPATH_DIR}" -l "${CENT_LOCAL_PATH}" -b "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
            COMMENT Running cttheme-copy.py
            VERBATIM)
ENDIF ()