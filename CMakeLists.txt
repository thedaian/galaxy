cmake_minimum_required(VERSION 3.0.0)
project(galaxy VERSION 1.0.0 LANGUAGES CXX)

#Add SFML's CMake scripts that build it:
add_subdirectory(extlib)

#Make a new executable for galaxy:
add_executable(galaxy src/main.cpp)
target_include_directories(galaxy PUBLIC ${PROJECT_SOURCE_DIR}/include/ PRIVATE ${PROJECT_SOURCE_DIR}/extlibs/SFML-2.5.1/include/)
target_link_libraries(galaxy sfml-system sfml-graphics sfml-window)

set_target_properties(galaxy PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(DIRECTORY res/ DESTINATION ${CMAKE_BINARY_DIR}/galaxy/res OPTIONAL)
install(FILES $<TARGET_FILE:sfml-system> DESTINATION ${CMAKE_BINARY_DIR}/galaxy/lib OPTIONAL)
install(FILES $<TARGET_FILE:sfml-graphics> DESTINATION ${CMAKE_BINARY_DIR}/galaxy/lib OPTIONAL)
install(FILES $<TARGET_FILE:sfml-window> DESTINATION ${CMAKE_BINARY_DIR}/galaxy/lib OPTIONAL)

set_target_properties(galaxy PROPERTIES 
    INSTALL_RPATH ${CMAKE_BINARY_DIR}/galaxy/lib
)
install(TARGETS galaxy DESTINATION ${CMAKE_BINARY_DIR}/galaxy OPTIONAL)