
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
// A file (.eml) is created for each e-mail in a folder with the same name as its mailbox; 
// the files name being a combination of the mails UID/index prefix and the subject name. If configured
// it will poll the server every X minutes to archive any new mail. Lastly if the server disconnects it
// will retry the connection up to --retry times before failing with an error. The method used to search
// for updates relies on the mail UID being stored as part of the file name and is not sophisticated enough
// to keep 100% accuracy. It should  not miss mail but will fail to keep in sync with mail that is
// moved from one mailbox to another after it has already been archived.
//
// This program is based on the code for example program ArchiveMailBox but has been re-factored 
// heavily to enable easier future development. All options and their meaning are obtained by running 
// he program with the option --help.
//
// Pendulum Example Application
// Program Options:
//   --help                   Print help messages
//   -c [ --config ] arg      Config File Name
//   -s [ --server ] arg      IMAP Server URL and port
//   -u [ --user ] arg        Account username
//   -p [ --password ] arg    User password
//   -m [ --mailbox ] arg     Mailbox name
//   -d [ --destination ] arg Destination for archived e-mail
//   --poll arg               Poll time in minutes
//   -r [ --retry ] arg       Server reconnect retry count
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
#include <stdexcept>

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

#include "CRedirect.hpp"
#include "CIMAP.hpp"
#include "CIMAPParse.hpp"

//
// Boost file system definitions
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

    using namespace Antik::IMAP;
    using namespace Antik::Util;

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

            CRedirect logFile{std::cout};
            ServerConnection imapConnection;
            vector<MailBoxDetails> mailBoxList;
             
            // Setup option data
            
            PendulumOptions optionData { fetchCommandLineOptions(argc, argv) };

            // Output to log file ( CRedirect(std::cout) is the simplest solution). Once the try is exited
            // CRedirect object will be destroyed and cout restored.

            if (!optionData.logFileNameStr.empty()) {
                logFile.change(optionData.logFileNameStr, std::ios_base::out | std::ios_base::app);
                cout << std::string(100, '=') << endl;;
            }

            // Initialize CIMAP internals

            CIMAP::init();

            // Set mail account user name and password

            imapConnection.server.setServer(optionData.serverURLStr);
            imapConnection.server.setUserAndPassword(optionData.userNameStr, optionData.userPasswordStr);
            
            // Set retry count
            
            imapConnection.retryCount = optionData.retryCount;
            
            do {

                // Connect

                cout << "Connecting to server [" << imapConnection.server.getServer() << "][" << imapConnection.connectCount << "]" << endl;

                serverConnect(imapConnection);
                
                // Reset reconnect mailbox to none
                
                imapConnection.reconnectMailBoxStr = "";

                // Create mailbox list if doesn't exist

                if (mailBoxList.empty()) {
                    mailBoxList = fetchMailBoxList(imapConnection, optionData.mailBoxNameStr, optionData.bAllMailBoxes);
                }
                
                // Process mailboxes

                for (MailBoxDetails& mailBoxEntry : mailBoxList) {
                    
                    // Set mailbox to select on reconnect.
                    
                    imapConnection.reconnectMailBoxStr = mailBoxEntry.nameStr;
                            
                    // Set mailbox archive folder

                    if (mailBoxEntry.pathStr.empty()) {
                        mailBoxEntry.pathStr = createMailboxFolder(optionData.destinationFolderStr, mailBoxEntry.nameStr);
                    }
                    
                     // If only updates specified find highest UID to search from

                    if (optionData.bOnlyUpdates && (imapConnection.connectCount==0)) {                       
                        mailBoxEntry.searchUID = getNewestUID(mailBoxEntry.pathStr);
                    }

                    // Get vector of new mail UID(s)

                    vector<uint64_t> messageUID { fetchMailBoxMessages(imapConnection, mailBoxEntry) };

                    // If messages found then create new EML files.

                    if (messageUID.size()) {
                        cout << "Messages found = " << messageUID.size() << endl;
                        for (auto uid : messageUID) {
                            pair<string, string> emailContents { fetchEmailContents(imapConnection, uid) };
                            if (emailContents.first.size() && emailContents.second.size()) {
                                createEMLFile(emailContents, uid, mailBoxEntry.pathStr);
                            } else {
                                cerr << "E-mail file not created as subject or contents empty" << endl;
                            }
                            
                        }
                        mailBoxEntry.searchUID = messageUID.back(); // Update search UID
                    } else {
                        cout << "No messages found." << endl;
                    }

                }

                // Disconnect from server

                cout << "Disconnecting from server [" << optionData.serverURLStr << "]" << endl;

                imapConnection.server.disconnect();
                
                // Increment connection count 
                
                imapConnection.connectCount++;
                
                // Wait poll interval (pollTime == 0 then one pass)

                std::this_thread::sleep_for(std::chrono::minutes(optionData.pollTime));

            } while (optionData.pollTime);

        //
        // Catch any errors
        //    

        } catch (const CIMAP::Exception &e) {
            exitWithError(e.what());
        } catch (const CIMAPParse::Exception &e) {
            exitWithError(e.what());
        } catch (const fs::filesystem_error & e) {
            exitWithError(string("BOOST file system exception: [") + e.what() + "]");
        } catch (const exception & e) {
            exitWithError(string("Standard exception: [") + e.what() + "]");
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
