include(FetchContent)

message(STATUS "Fetching SFML...")

FetchContent_Declare(
  SFML
  URL "https://github.com/SFML/SFML/archive/2.5.1.zip"
)

if(NOT sfml_POPULATED)
    FetchContent_Populate(SFML)
endif()

# No need to build audio and network modules
set(SFML_BUILD_AUDIO CACHE BOOL FALSE)
set(SFML_BUILD_NETWORK CACHE BOOL FALSE)
add_subdirectory(${sfml_SOURCE_DIR} ${sfml_BINARY_DIR} EXCLUDE_FROM_ALL)

message(STATUS "DONE!")