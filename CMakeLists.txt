cmake_minimum_required(VERSION 3.10.2)

set(PROGRAM_NAME "pendulum")
set(PROGRAM_VERSION 0.1.0)

project(${PROGRAM_NAME} VERSION ${PROGRAM_VERSION} DESCRIPTION "E-mail Archive Program")

# Build Antik library

add_subdirectory(../Antik antik)

# Pendulum sources and includes

file(GLOB PROGRAM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB PROGRAM_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" )

# Pendulum target

add_executable(${PROGRAM_NAME} ${PROGRAM_SOURCES} )
target_include_directories(${PROGRAM_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROGRAM_NAME} antik)

# Install Pendulum

install(TARGETS ${PROGRAM_NAME} DESTINATION bin)
