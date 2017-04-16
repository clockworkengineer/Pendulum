#include "HOST.hpp"
/*
 * File:   Pendulum_File.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on Apil 10, 2017, 2:34 PM
 * 
 * Copyright 2017.
 * 
 */

//
// Module: Pendulum_File
//
// Description: Pendulum file processing functionality.
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antikythera Classes: CIMAPParse.
// Boost              : File system, iterator.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL definitions
//

#include <iostream>
#include <fstream>
#include <sstream>

//
// Pendulum and Pendulum File definitions
//

#include "Pendulum.hpp"
#include "Pendulum_File.hpp"

//
// Antikythera Classes
//

#include "CIMAPParse.hpp"

using Antik::Mail::CIMAPParse;

//
// Boost file system & range iterator definitions
//

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

// =========
// NAMESPACE
// =========

namespace Pendulum_File {

    // =======
    // IMPORTS
    // =======

    namespace fs = boost::filesystem;
    
    // ===============
    // LOCAL FUNCTIONS
    // ===============

    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Create .eml for downloaded email.
    //

    void createEMLFile(const std::pair<std::string, std::string>& emailContents, std::uint64_t index, const std::string& destFolderStr) {

        if (!emailContents.second.empty()) {
            fs::path fullFilePath = destFolderStr;
            fullFilePath /= "(" + std::to_string(index) + ") " + emailContents.first + Pendulum::kEMLFileExt;
            if (!fs::exists(fullFilePath)) {
                std::istringstream emailBodyStream(emailContents.second);
                std::ofstream emlFileStream(fullFilePath.string(), std::ios::binary);
                if (emlFileStream.is_open()) {
                    std::cout << "Creating [" << fullFilePath.native() << "]" << std::endl;
                    for (std::string lineStr; std::getline(emailBodyStream, lineStr, '\n');) {
                        lineStr.push_back('\n');
                        emlFileStream.write(&lineStr[0], lineStr.length());
                    }
                } else {
                    std::cerr << "Failed to create file [" << fullFilePath << "]" << std::endl;
                }
            }
        }

    }

    //
    // Find the UID on the last message saved and search from that. Each saved .eml file has a "(UID)"
    // prefix; get the UID from this.
    //

    std::uint64_t getNewestUID(const std::string& destFolderStr) {

        if (fs::exists(destFolderStr) && fs::is_directory(destFolderStr)) {

            std::uint64_t highestUID = 1, currentUID = 0;
            fs::path destPath { destFolderStr };
            
            for (auto& entry : boost::make_iterator_range(fs::directory_iterator(destPath),{})) {
                if (fs::is_regular_file(entry.status()) && (entry.path().extension().compare(Pendulum::kEMLFileExt) == 0)) {
                    currentUID = std::strtoull(CIMAPParse::stringBetween(entry.path().filename().string(), '(', ')').c_str(), nullptr, 10);
                    if (currentUID > highestUID) {
                        highestUID = currentUID;
                    }
                }
            }

            return (highestUID);

        }

        return (0);

    }


} // namespace Pendulum_File

