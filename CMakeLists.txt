cmake_minimum_required(VERSION 3.10.2)

project("Pendulum" VERSION 0.1.0 DESCRIPTION "E-mail Archive Program")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")

# Build Antik library

add_subdirectory(antik)

# Pendulum sources and includes

set (PENDULUM_SOURCES
    Pendulum.cpp
    Pendulum_CommandLine.cpp
    Pendulum_File.cpp
    Pendulum_MailBox.cpp
)

set (PENDULUM_INCLUDES
    Pendulum.hpp
    Pendulum_CommandLine.hpp
    Pendulum_File.hpp
    Pendulum_MailBox.hpp
)


# Pendulum target

add_executable(${PROJECT_NAME} ${PENDULUM_SOURCES} )
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} antik)

# Install Pendulum

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
