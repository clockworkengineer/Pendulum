
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
#include <thread>
#include <chrono>

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

            CIMAP imapServer;
            
            // Setup argument data
            
            ParamArgData argumentData { fetchCommandLineArgs(argc, argv) };
 
            // Initialize CIMAP internals

            CIMAP::init();

            // Set mail account user name and password

            imapServer.setServer(argumentData.serverURLStr);
            imapServer.setUserAndPassword(argumentData.userNameStr, argumentData.userPasswordStr);

            do {

                // Connect

                cout << "Connecting to server [" << argumentData.serverURLStr << "]" << endl;

                imapServer.connect();

                // Create mailbox list

                vector<string> mailBoxList { fetchMailBoxList(imapServer, argumentData.mailBoxNameStr, 
                                              argumentData.bAllMailBoxes, argumentData.retryCount) };

                for (string& mailBoxNameStr : mailBoxList) {

                    uint64_t searchUID { 0 };      
                    std::string mailBoxPathStr { createMailboxFolder(argumentData.destinationFolderStr, mailBoxNameStr) };
                    
                     // If only updates specified find highest UID to search from

                    if (argumentData.bOnlyUpdates) {
                        searchUID = getNewestIndex(mailBoxPathStr);
                    }

                    // Get vector of new mail UIDs

                    vector<uint64_t> messageUID { fetchMailBoxMessages(imapServer, mailBoxNameStr, searchUID, argumentData.retryCount) };

                    // If messages found then create new EML files.

                    if (messageUID.size()) {
                        cout << "Messages found = " << messageUID.size() << endl;
                        for (auto uid : messageUID) {
                            pair<string, string> emailContents { fetchEmailContents(imapServer, mailBoxNameStr, uid, argumentData.retryCount) };
                            if (emailContents.first.size() && emailContents.second.size()) {
                                createEMLFile(emailContents, uid, mailBoxPathStr);
                            } else {
                                cerr << "E-mail file not created as subject or contents empty" << endl;
                            }
                        }
                    } else {
                        cout << "No messages found." << endl;
                    }

                }

                // Disconnect from server

                cout << "Disconnecting from server [" << argumentData.serverURLStr << "]" << endl;

                imapServer.disconnect();
                
                // Wait poll interval pollTime == 0 then one pass

                std::this_thread::sleep_for(std::chrono::minutes(argumentData.pollTime));

            } while (argumentData.pollTime);

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
