
#include "HOST.hpp"
/*
 * File:   Pendulum.cpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:33 PM
 *
 * Copyright 2016.
 *
 */

//
// Program: Pendulum
//
// Description: This is a command line program to log on to an IMAP server and download e-mails
// from a configured mailbox, command separated mailbox list or all mailboxes for an account.
// A file (.eml) is created for each e-mail in a folder with the same name as the mailbox; with
// the files name being a combination of the mails UID/index prefix and the subject name. All 
// parameters and their meaning are obtained by running the program with the parameter --help.
//
// Pendulum Example Application
// Program Options:
//  --help                   Print help messages
//   -c [ --config ] arg      Config File Name
//   -s [ --server ] arg      IMAP Server URL and port
//   -u [ --user ] arg        Account username
//   -p [ --password ] arg    User password
//   -m [ --mailbox ] arg     Mailbox name
//   -d [ --destination ] arg Destination for attachments
//   -u [ --updates ]         Search since last file archived.
//   -a [ --all ]             Download files for all mailboxes.
//
// Note: MIME encoded words in the email subject line are decoded to the best ASCII fit
// available.
// 
// Dependencies: C11++, Classes (CFileMIME, CMailIMAP, CMailIMAPParse, CMailIMAPBodyStruct),
//               Linux, Boost C++ Libraries.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL definitions
//

#include <iostream>

//
// Program components.
//

#include "Pendulum.hpp"
#include "Pendulum_CommandLine.hpp"
#include "Pendulum_MailBox.hpp"
#include "Pendulum_File.hpp"

using namespace Pendulum;
using namespace Pendulum_CommandLine;
using namespace Pendulum_MailBox;
using namespace Pendulum_File;

//
// Antikythera Classes
//

#include "CIMAP.hpp"
#include "CIMAPParse.hpp"

using namespace Antik::Mail;

//
// Boost file system and iterator definitions
//

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

// ======================
// LOCAL TYES/DEFINITIONS
// ======================

// ===============
// LOCAL FUNCTIONS
// ===============

//
// Exit with error message/status
//

void exitWithError(const std::string errMsgStr) {

    // Closedown email, display error and exit.

    CIMAP::closedown();
    std::cerr << errMsgStr << std::endl;
    exit(EXIT_FAILURE);

}

// ============================
// ===== MAIN ENTRY POINT =====
// ============================

int main(int argc, char** argv) {

    try {

        ParamArgData argData;
        CIMAP imap;
        std::vector<std::string> mailBoxList;

        // Read in command line parameters and process

        procCmdLine(argc, argv, argData);

        // Initialise CMailIMAP internals

        CIMAP::init();

        // Set mail account user name and password

        imap.setServer(argData.serverURLStr);
        imap.setUserAndPassword(argData.userNameStr, argData.userPasswordStr);

        // Connect

        std::cout << "Connecting to server [" << argData.serverURLStr << "]" << std::endl;

        imap.connect();

        // Create mailbox list

        mailBoxList = fetchMailBoxList(imap, argData.mailBoxNameStr, argData.bAllMailBoxes);

        for (std::string mailBoxNameStr : mailBoxList) {

            std::uint64_t searchUID = 0;
            std::string mailBoxFolderStr = mailBoxNameStr;

            // Clear any quotes from mailbox name for folder name

            if (mailBoxFolderStr.front() == '\"') mailBoxFolderStr = mailBoxNameStr.substr(1);
            if (mailBoxFolderStr.back() == '\"') mailBoxFolderStr.pop_back();

            // Create mailbox destination folder
            
            fs::path mailBoxPath {argData.destinationFolderStr};
            mailBoxPath /= mailBoxFolderStr;
            if (!argData.destinationFolderStr.empty() && !fs::exists(mailBoxPath)) {
                std::cout << "Creating destination folder = [" << mailBoxPath.native() << "]" << std::endl;
                fs::create_directories(mailBoxPath);
            }

            // If only updates specified find high UID to search from
            
            if (argData.bOnlyUpdates) {
                searchUID = getHighestUID(mailBoxPath.string());
            }

            // Get vector of new mail UIDs
            
            std::vector<std::uint64_t> messageIDs = fetchMailBoxMessages(imap, mailBoxNameStr, searchUID);

            // If messages found then create new EML files.
            
            if (messageIDs.size()) {
                std::cout << "Messages found = " << messageIDs.size() << std::endl;
                for (auto index : messageIDs) {
                    std::pair<std::string, std::string> emailContents = fetchEmailContents(imap, mailBoxNameStr, index);
                    createEMLFile(emailContents, index, mailBoxPath.string());
                }
            } else {
                std::cout << "No messages found." << std::endl;
            }

        }

        // Disconnect from server
        
        std::cout << "Disconnecting from server [" << argData.serverURLStr << "]" << std::endl;

        imap.disconnect();

    //
    // Catch any errors
    //    

    } catch (CIMAP::Exception &e) {
        exitWithError(e.what());
    } catch (CIMAPParse::Exception &e) {
        exitWithError(e.what());
    } catch (const fs::filesystem_error & e) {
        exitWithError(std::string("BOOST file system exception occured: [") + e.what() + "]");
    } catch (std::exception & e) {
        exitWithError(std::string("Standard exception occured: [") + e.what() + "]");
    }

    // IMAP closedown

    CIMAP::closedown();

    exit(EXIT_SUCCESS);


}
