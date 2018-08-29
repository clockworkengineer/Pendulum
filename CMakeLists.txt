cmake_minimum_required(VERSION 3.10.2)

project("pendulum" VERSION 0.1.0 DESCRIPTION "E-mail Archive Program")

# Build Antik library

add_subdirectory(../Antik antik)

# Pendulum sources and includes

file(GLOB PROGRAM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB PROGRAM_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" )

# Pendulum target

add_executable(${PROJECT_NAME} ${PROGRAM_SOURCES} )
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} antik)

# Install Pendulum

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
