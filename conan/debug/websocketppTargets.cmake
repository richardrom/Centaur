# Load the debug and release variables
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB DATA_FILES "${_DIR}/websocketpp-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${websocketpp_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        conan_message(STATUS "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET websocketpp::websocketpp)
    add_library(websocketpp::websocketpp INTERFACE IMPORTED)
    conan_message(STATUS "Conan: Target declared 'websocketpp::websocketpp'")
endif()
# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/websocketpp-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()