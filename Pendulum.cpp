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
// Antik Classes      : CMIME, CIMAP, CIMAPParse, CFile, CSocket.
// Linux              : Target platform
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

//
// Antik Classes
//

#include "CRedirect.hpp"
#include "CIMAP.hpp"
#include "CIMAPParse.hpp"
#include "CSocket.hpp"
#include "CFile.hpp"

//
// Program components.
//

#include "Pendulum.hpp"
#include "Pendulum_CommandLine.hpp"
#include "Pendulum_MailBox.hpp"
#include "Pendulum_File.hpp"

// =========
// NAMESPACE
// =========

namespace Pendulum {

    // =======
    // IMPORTS
    // =======

    using namespace Pendulum_CommandLine;
    using namespace Pendulum_MailBox;
    using namespace Pendulum_File;

    using namespace Antik::IMAP;
    using namespace Antik::Util;
    using namespace Antik::File;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Exit with error message/status
    //

    static void exitWithError(const std::string errMsg) {

        // Display error and exit.

        std::cerr << errMsg << std::endl;
        exit(EXIT_FAILURE);

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

    void archiveEmail(int argc, char** argv) {

        try {

            CRedirect logFile{std::cout};
            ServerConnection imapConnection;
            std::vector<MailBoxDetails> mailBoxList;
             
            // Setup option data
            
            PendulumOptions optionData { fetchCommandLineOptions(argc, argv) };

            // Output to log file ( CRedirect(std::cout) is the simplest solution). Once the try is exited
            // CRedirect object will be destroyed and std::cout restored.

            if (!optionData.logFileName.empty()) {
                logFile.change(optionData.logFileName, std::ios_base::out | std::ios_base::app);
                std::cout << std::string(100, '=') << std::endl;
            }

            // Set mail account user name and password

            imapConnection.server.setServer(optionData.serverURL);
            imapConnection.server.setUserAndPassword(optionData.userName, optionData.userPassword);
            
            // Set retry count
            
            imapConnection.retryCount = optionData.retryCount;
            
            do {

                // Connect

                std::cout << "Connecting to server [" << imapConnection.server.getServer() << "][" << imapConnection.connectCount << "]" << std::endl;

                serverConnect(imapConnection);
                
                // Reset reconnect mailbox to none
                
                imapConnection.reconnectMailBox = "";

                // Create mailbox list if doesn't exist

                if (mailBoxList.empty()) {
                    mailBoxList = fetchMailBoxList(imapConnection, optionData.mailBoxList, optionData.ignoreList, optionData.bAllMailBoxes);
                }
                
                // Process mailboxes

                for (MailBoxDetails& mailBoxEntry : mailBoxList) {
                    
                    // Set mailbox to select on reconnect.
                    
                    imapConnection.reconnectMailBox = mailBoxEntry.name;
                            
                    // Set mailbox archive folder

                    if (mailBoxEntry.path.empty()) {
                        mailBoxEntry.path = createMailboxFolder(optionData.destinationFolder, mailBoxEntry.name);
                    }
                    
                     // If only updates specified find highest UID to search from

                    if (optionData.bOnlyUpdates && (imapConnection.connectCount==0)) {                       
                        mailBoxEntry.searchUID = getNewestUID(mailBoxEntry.path);
                    }

                    // Get vector of new mail UID(s)

                    std::vector<uint64_t> messageUID { fetchMailBoxMessages(imapConnection, mailBoxEntry) };

                    // If messages found then create new EML files.

                    if (messageUID.size()) {
                        std::cout << "Messages found = " << messageUID.size() << std::endl;
                        for (auto uid : messageUID) {
                            std::pair<std::string, std::string> emailContents { fetchEmailContents(imapConnection, uid) };
                            if (emailContents.first.size() && emailContents.second.size()) {
                                createEMLFile(emailContents, uid, mailBoxEntry.path);
                            } else {
                                std::cerr << "E-mail file not created as subject or contents empty" << std::endl;
                            }
                            
                        }
                        mailBoxEntry.searchUID = messageUID.back(); // Update search UID
                    } else {
                        std::cout << "No messages found." << std::endl;
                    }

                }

                // Disconnect from server

                std::cout << "Disconnecting from server [" << optionData.serverURL << "]" << std::endl;

                imapConnection.server.disconnect();
                
                // Increment connection count 
                
                imapConnection.connectCount++;
                
                // Wait poll interval (pollTime == 0 then one pass)

                std::this_thread::sleep_for(std::chrono::minutes(optionData.pollTime));

            } while (optionData.pollTime);

        //
        // Catch any errors
        //    
 
        } catch (const std::exception & e) {
            exitWithError(e.what());
        }


    }

} // namespace Pendulum

// ============================
// ===== MAIN ENTRY POINT =====
// ============================

int main(int argc, char** argv) {

    Pendulum::archiveEmail(argc, argv);

    exit(EXIT_SUCCESS);

}
