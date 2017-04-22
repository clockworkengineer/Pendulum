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
// Antikythera Classes: CMailIMAPParse.
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

    using namespace std;
    
    using Antik::Mail::CIMAPParse;
    
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

    std::string createMailboxFolder(const string& destFolderStr, const string& mailBoxNameStr) {

        string mailBoxFolderStr { mailBoxNameStr };

        // Clear any quotes from mailbox name for folder name

        if (mailBoxFolderStr.front() == '\"') mailBoxFolderStr = mailBoxNameStr.substr(1);
        if (mailBoxFolderStr.back() == '\"') mailBoxFolderStr.pop_back();

        // Create mailbox destination folder

        fs::path mailBoxPath {destFolderStr };
        
        mailBoxPath /= mailBoxFolderStr;
        if (!fs::exists(mailBoxPath)) {
            cout << "Creating destination folder = [" << mailBoxPath.native() << "]" << endl;
            fs::create_directories(mailBoxPath);
        }
        
        return(mailBoxPath.string());
        
    }
    
    //
    // Create .eml for downloaded email.
    //

    void createEMLFile(const pair<string, string>& emailContents, uint64_t index, const string& destFolderStr) {

        if (!emailContents.second.empty()) {
            fs::path fullFilePath { destFolderStr };
            fullFilePath /= "(" + to_string(index) + ") " + emailContents.first + Pendulum::kEMLFileExt;
            if (!fs::exists(fullFilePath)) {
                istringstream emailBodyStream { emailContents.second };
                ofstream emlFileStream { fullFilePath.string(), ios::binary };
                if (emlFileStream.is_open()) {
                    cout << "Creating [" << fullFilePath.native() << "]" << endl;
                    for (string lineStr; getline(emailBodyStream, lineStr, '\n');) {
                        lineStr.push_back('\n');
                        emlFileStream.write(&lineStr[0], lineStr.length());
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

    uint64_t getNewestUID(const string& destFolderStr) {

        if (fs::exists(destFolderStr) && fs::is_directory(destFolderStr)) {

            uint64_t highestIndex { 1 };
            uint64_t currentIndex { 0 };
            fs::path destPath { destFolderStr };
            
            for (auto& entry : boost::make_iterator_range(fs::directory_iterator(destPath),{})) {
                if (fs::is_regular_file(entry.status()) && (entry.path().extension().compare(Pendulum::kEMLFileExt) == 0)) {
                    currentIndex = strtoull(CIMAPParse::stringBetween(entry.path().filename().string(), '(', ')').c_str(), nullptr, 10);
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

