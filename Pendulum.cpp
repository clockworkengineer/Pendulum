
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
// A file (.eml) is created for each e-mail in a folder with the same name as its mailbox; with
// the files name being a combination of the mails UID/index prefix and the subject name. This
// program is based on the code for example program ArchiveMailBox but has been re-factored heavily
// to enable easier future development. All parameters and their meaning are obtained by running 
// he program with the parameter --help.
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
// Dependencies: 
// 
// C11++              : Use of C11++ features.
// Antikythera Classes: CMIME, CIMAP, CIMAPParse.
// Linux              : Target platform
// Boost              : File system, program option, iterator.
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

//
// Antikythera Classes
//

#include "CIMAP.hpp"
#include "CIMAPParse.hpp"

//
// Boost file system and iterator definitions
//

#include <boost/filesystem.hpp>

// =========
// NAMESPACE
// =========

namespace Pendulum {

    // =======
    // IMPORTS
    // =======
    
    using namespace std;
    
    using namespace Pendulum_CommandLine;
    using namespace Pendulum_MailBox;
    using namespace Pendulum_File;

    using namespace Antik::Mail;

    namespace fs = boost::filesystem;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Exit with error message/status
    //

    static void exitWithError(const string errMsgStr) {

        // Closedown email, display error and exit.

        CIMAP::closedown();
        cerr << errMsgStr << endl;
        exit(EXIT_FAILURE);

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

    int archiveEmail(int argc, char** argv) {

        try {

            ParamArgData argumentData;
            CIMAP imapServer;
            vector<string> mailBoxList;

            // Read in command line parameters and process

            fetchCommandLineArgs(argc, argv, argumentData);

            // Initialize CIMAP internals

            CIMAP::init();

            // Set mail account user name and password

            imapServer.setServer(argumentData.serverURLStr);
            imapServer.setUserAndPassword(argumentData.userNameStr, argumentData.userPasswordStr);

            // Connect

            cout << "Connecting to server [" << argumentData.serverURLStr << "]" << endl;

            imapServer.connect();

            // Create mailbox list

            mailBoxList = fetchMailBoxList(imapServer, argumentData.mailBoxNameStr, argumentData.bAllMailBoxes);

            for (string mailBoxNameStr : mailBoxList) {

                uint64_t searchUID { 0 };
                string mailBoxFolderStr { mailBoxNameStr };

                // Clear any quotes from mailbox name for folder name

                if (mailBoxFolderStr.front() == '\"') mailBoxFolderStr = mailBoxNameStr.substr(1);
                if (mailBoxFolderStr.back() == '\"') mailBoxFolderStr.pop_back();

                // Create mailbox destination folder

                fs::path mailBoxPath{argumentData.destinationFolderStr};
                mailBoxPath /= mailBoxFolderStr;
                if (!argumentData.destinationFolderStr.empty() && !fs::exists(mailBoxPath)) {
                    cout << "Creating destination folder = [" << mailBoxPath.native() << "]" << endl;
                    fs::create_directories(mailBoxPath);
                }

                // If only updates specified find highest UID to search from

                if (argumentData.bOnlyUpdates) {
                    searchUID = getNewestIndex(mailBoxPath.string());
                }

                // Get vector of new mail UIDs

                vector<uint64_t> messageIDs = fetchMailBoxMessages(imapServer, mailBoxNameStr, searchUID);

                // If messages found then create new EML files.

                if (messageIDs.size()) {
                    cout << "Messages found = " << messageIDs.size() << endl;
                    for (auto index : messageIDs) {
                        pair<string, string> emailContents = fetchEmailContents(imapServer, mailBoxNameStr, index);
                        createEMLFile(emailContents, index, mailBoxPath.string());
                    }
                } else {
                    cout << "No messages found." << endl;
                }

            }

            // Disconnect from server

            cout << "Disconnecting from server [" << argumentData.serverURLStr << "]" << endl;

            imapServer.disconnect();

        //
        // Catch any errors
        //    

        } catch (CIMAP::Exception &e) {
            exitWithError(e.what());
        } catch (CIMAPParse::Exception &e) {
            exitWithError(e.what());
        } catch (const fs::filesystem_error & e) {
            exitWithError(string("BOOST file system exception occured: [") + e.what() + "]");
        } catch (exception & e) {
            exitWithError(string("Standard exception occured: [") + e.what() + "]");
        }

        // IMAP closedown

        CIMAP::closedown();

    }

} // namespace Pendulum

// ============================
// ===== MAIN ENTRY POINT =====
// ============================

int main(int argc, char** argv) {

    Pendulum::archiveEmail(argc, argv);

    exit(EXIT_SUCCESS);

}
