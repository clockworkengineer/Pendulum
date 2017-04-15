/*
 * File:   Pendulum_File.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on Apil 10, 2017, 2:34 PM
 * 
 * Copyright 2017.
 * 
 */

#ifndef PENDULUM_FILE_HPP
#define PENDULUM_FILE_HPP

//
// C++ STL definitions
//

#include <string>
#include <utility>

namespace Pendulum_File {

    void createEMLFile(const std::pair<std::string, std::string>& emailContents, std::uint64_t index, const std::string& destFolderStr);
    std::uint64_t getHighestUID(const std::string& destFolderStr);

} // namespace Pendulum_File

#endif /* PENDULUM_FILE_HPP */

