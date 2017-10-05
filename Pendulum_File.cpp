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
// Boost              : File system, iterator.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <iostream>
#include <fstream>
#include <sstream>

//
// Pendulum and Pendulum File
//

#include "Pendulum.hpp"
#include "Pendulum_File.hpp"

//
// Boost file system & range iterator
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

    using namespace std;
    
    namespace fs = boost::filesystem;
    
    // ===============
    // LOCAL FUNCTIONS
    // ===============

    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Create destination for mailbox archive
    //

    std::string createMailboxFolder(const string& destFolder, const string& mailBoxName) {

        string mailBoxFolder { mailBoxName };

        // Clear any quotes from mailbox name for folder name

        if (mailBoxFolder.front() == '\"') mailBoxFolder = mailBoxName.substr(1);
        if (mailBoxFolder.back() == '\"') mailBoxFolder.pop_back();

        // Create mailbox destination folder

        fs::path mailBoxPath {destFolder };
        
        mailBoxPath /= mailBoxFolder;
        if (!fs::exists(mailBoxPath)) {
            cout << "Creating destination folder = [" << mailBoxPath.native() << "]" << endl;
            fs::create_directories(mailBoxPath);
        }
        
        return(mailBoxPath.string());
        
    }
    
    //
    // Create .eml for downloaded email.
    //

    void createEMLFile(const pair<string, string>& emailContents, uint64_t uid, const string& destFolder) {

        if (!emailContents.second.empty()) {
            fs::path fullFilePath { destFolder };
            fullFilePath /= "(" + to_string(uid) + ") " + emailContents.first + Pendulum::kEMLFileExt;
            if (!fs::exists(fullFilePath)) {
                istringstream emailBodyStream { emailContents.second };
                ofstream emlFileStream { fullFilePath.string(), ios::binary };
                if (emlFileStream.is_open()) {
                    cout << "Creating [" << fullFilePath.native() << "]" << endl;
                    for (string line; getline(emailBodyStream, line, '\n');) {
                        line.push_back('\n');
                        emlFileStream.write(&line[0], line.length());
                    }
                } else {
                    cerr << "Failed to create file [" << fullFilePath << "]" << endl;
                }
            }
        }

    }

    //
    // Find the Index on the last message saved and search from that. Each saved .eml file has a "(Index)"
    // prefix; get the Index from this.
    //

    uint64_t getNewestUID(const string& destFolder) {

        if (fs::exists(destFolder) && fs::is_directory(destFolder)) {

            uint64_t highestIndex { 1 };
            uint64_t currentIndex { 0 };
            fs::path destPath { destFolder };
            
            for (auto& entry : boost::make_iterator_range(fs::directory_iterator(destPath),{})) {
                if (fs::is_regular_file(entry.status()) && (entry.path().extension().compare(Pendulum::kEMLFileExt) == 0)) {
                    std::string uid { entry.path().filename().string()};
                    uid = uid.substr(uid.find_first_of(('('))+1);
                    uid = uid.substr(0, uid.find_first_of((')')));
                    currentIndex = strtoull(uid.c_str(), nullptr, 10);
                    if (currentIndex > highestIndex) {
                        highestIndex = currentIndex;
                    }
                }
            }

            return (highestIndex);

        }

        return (0);

    }

} // namespace Pendulum_File

