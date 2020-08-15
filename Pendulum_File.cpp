//
// Module: Pendulum_File
//
// Description: Pendulum file processing functionality.
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antik Classes      : CPath, CFile.
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
// Antik Classes
//

#include "CFile.hpp"
#include "CPath.hpp"

//
// Pendulum and Pendulum File
//

#include "Pendulum.hpp"
#include "Pendulum_File.hpp"

// =========
// NAMESPACE
// =========

namespace Pendulum_File {

    // =======
    // IMPORTS
    // =======

    using namespace Antik::File;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Create destination for mailbox archive
    //

    std::string createMailboxFolder(const std::string& destFolder, const std::string& mailBoxName) {

        std::string mailBoxFolder { mailBoxName };

        // Clear any quotes from mailbox name for folder name

        if (mailBoxFolder.front() == '\"') mailBoxFolder = mailBoxName.substr(1);
        if (mailBoxFolder.back() == '\"') mailBoxFolder.pop_back();

        // Create mailbox destination folder

        CPath mailBoxPath {destFolder };
        
        mailBoxPath.join(mailBoxFolder);
        if (!CFile::exists(mailBoxPath)) {
            std::cout << "Creating destination folder = [" << mailBoxPath.toString() << "]" << std::endl;
            CFile::createDirectory(mailBoxPath);
        }
        
        return(mailBoxPath.toString());
        
    }
    
    //
    // Create .eml for downloaded email.
    //

    void createEMLFile(const std::pair<std::string, std::string>& emailContents, uint64_t uid, const std::string& destFolder) {

        if (!emailContents.second.empty()) {
            CPath fullFilePath { destFolder };
            fullFilePath.join("(" + std::to_string(uid) + ") " + emailContents.first + Pendulum::kEMLFileExt);
            if (!CFile::exists(fullFilePath)) {
                std::istringstream emailBodyStream { emailContents.second };
                std::ofstream emlFileStream { fullFilePath.toString(), std::ios::binary };
                if (emlFileStream.is_open()) {
                    std::cout << "Creating [" << fullFilePath.toString() << "]" << std::endl;
                    for (std::string line; getline(emailBodyStream, line, '\n');) {
                        line.push_back('\n');
                        emlFileStream.write(&line[0], line.length());
                    }
                } else {
                    std::cerr << "Failed to create file [" << fullFilePath.toString() << "]" << std::endl;
                }
            }
        }

    }

    //
    // Find the Index on the last message saved and search from that. Each saved .eml file has a "(Index)"
    // prefix; get the Index from this.
    //

    uint64_t getNewestUID(const std::string& destFolder) {

        if (CFile::exists(destFolder) && CFile::isDirectory(destFolder)) {

            uint64_t highestIndex { 1 };
            uint64_t currentIndex { 0 };
            CPath destPath { destFolder };
            
            for (auto& file : CFile::directoryContentsList(destPath)) {
                if (CFile::isFile(file) && ( CPath(file).extension().compare(Pendulum::kEMLFileExt) == 0)) {
                    std::string uid { CPath(file).fileName()};
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

