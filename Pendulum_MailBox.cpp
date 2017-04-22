#include "HOST.hpp"
/*
 * File:   Pendulum_MailBox.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on Apil 10, 2017, 2:34 PM
 * 
 * Copyright 2017.
 * 
 */

//
// Module: Pendulum_MailBox
//
// Description: Pendulum IMAP mailbox functionality.Note while processing
// a command if the  server disconnects a reconnect is tried and the current
// mailbox reselected and the command re-issued.
// 
// Dependencies:
// 
// C11++              : Use of C11++ features.
// Antikythera Classes: CIMAP, CIMAPParse, CMIME.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL definitions
//

#include <iostream>

//
// Pendulum mailbox definitions.
//

#include "Pendulum_MailBox.hpp"

//
// Antikythera Classes
//

#include "CIMAPParse.hpp"
#include "CMIME.hpp"

// =========
// NAMESPACE
// =========

namespace Pendulum_MailBox {

    // =======
    // IMPORTS
    // =======

    using namespace std;
    
    using namespace Antik::IMAP;
    using namespace Antik::File;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Send command to IMAP server, parse received response and return it.
    // At present it catches any thrown exceptions then re-throws. Also any 
    // server disconnect or command error are also signaled by an exception.
    //

    static CIMAPParse::COMMANDRESPONSE sendCommand(CIMAP& imap, const string& commandStr) {

        CIMAPParse::COMMANDRESPONSE parsedResponse;

        try {
            
            string commandResponseStr { imap.sendCommand(commandStr) };
            if (commandResponseStr.size()) {
                parsedResponse = CIMAPParse::parseResponse(commandResponseStr);
            }
       
        } catch (...) {
            throw;  // Re-throw IMAP and parser exceptions
        }

        // Report server disconnect or command error after command response 
        // successfully received and response parsed.

        if (parsedResponse->bBYESent) {
            throw CIMAP::Exception("Received BYE from server: " + parsedResponse->errorMessageStr);
        } else if (parsedResponse->status != CIMAPParse::RespCode::OK) {
            throw CIMAP::Exception(commandStr + ": " + parsedResponse->errorMessageStr);
        }

        return (parsedResponse);

    }

    //
    // Reconnect to IMAP server and select passed in (current) mailbox. 
    //
    
    static void serverReconnect(CIMAP& imap, const string& mailBoxNameStr, int retryCount) {

        serverConnect(imap, retryCount);

        if (imap.getConnectedStatus() && mailBoxNameStr.size()) {
            CIMAPParse::COMMANDRESPONSE parsedResponse;
            parsedResponse = sendCommand(imap, "SELECT " + mailBoxNameStr);
            if ((parsedResponse) && (parsedResponse->status == CIMAPParse::RespCode::OK)) {
                cerr << "Reconnected to MailBox [" << mailBoxNameStr << "]" << endl;
            }
        }

    }
    
    //
    // Send a command to IMAP server. If the server disconnects try to reconnect
    // and resend command even if it was successful.
    //
    
    static CIMAPParse::COMMANDRESPONSE sendCommandRetry(CIMAP& imap, 
            const string& mailBoxNameStr, const string& commandStr, int retryCount) {
        
        CIMAPParse::COMMANDRESPONSE parsedResponse;

        try {
            parsedResponse = sendCommand(imap, commandStr);
        } catch (...) {
            // If still connected  re-throw error as not connection related.
            if (imap.getConnectedStatus()) {
                throw;
            }
        }

        try {
            // The command may have been successful but if a disconnect was detected
            // try to reconnect and repeat command just in case.
            if (!imap.getConnectedStatus()) {
                cerr << "Server Disconnect.\nTrying to reconnect ..." << endl;
                serverReconnect(imap, mailBoxNameStr, retryCount);
                parsedResponse = sendCommand(imap, commandStr);
            }
        } catch (...) {
            throw;  // Signal reconnect/command failure.
        }

        return(parsedResponse);
        
    }
    
    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Connect to IMAP server (performing retryCount times until successful).
    //
    
    void serverConnect(CIMAP& imap, int retryCount) {

        std::exception_ptr thrownException { nullptr };
        
        // Connect retry loop
        
        while(true) {

            // Try to connect
            
            try {             
                imap.connect(); 
                thrownException = nullptr;  // Possible success
            } catch (...) {
                thrownException = std::current_exception(); // Error        
            }

            // If connected return
            
            if (imap.getConnectedStatus() && !thrownException) {
                cout << "Connected." << endl;
                break;
            } 
            
            // Retry count == 0 try so throw last saved error
            
            if (!(--retryCount)) {   
                std::rethrow_exception(thrownException);
            }
            
            cerr << "Trying to reconnect ..." << endl;

        }

    }
    
    //
    // Convert list of comma separated mailbox names / list all mailboxes and 
    // place into vector of mailbox name strings to be returned.
    //

    vector<string> fetchMailBoxList(CIMAP& imap, const string& mailBoxNameStr, bool bAllMailBoxes, int retryCount) {

        vector<string> mailBoxList;

        if (bAllMailBoxes) {
            
            CIMAPParse::COMMANDRESPONSE parsedResponse;
            
            // Get list of all mailboxes
            
            parsedResponse = sendCommandRetry(imap, "", "LIST \"\" *", retryCount);

            if (parsedResponse) {

                for (auto& mailBoxEntry : parsedResponse->mailBoxList) {
                    if (mailBoxEntry.attributesStr.find("\\Noselect") == string::npos) {
                        mailBoxList.push_back(mailBoxEntry.mailBoxNameStr);
                    }
                }

            }

        } else {
            
            // Add mailbox list from config file or command line parameter
            
            istringstream mailBoxStream { mailBoxNameStr };
            
            for (string mailBoxStr; getline(mailBoxStream, mailBoxStr, ',');) {
                mailBoxStr = mailBoxStr.substr(mailBoxStr.find_first_not_of(' '));
                mailBoxStr = mailBoxStr.substr(0, mailBoxStr.find_last_not_of(' ') + 1);
                mailBoxList.push_back(mailBoxStr);
            }
            
        }

        return (mailBoxList);

    }

    //
    // Search a mailbox for e-mails with UIDs greater than searchUID and return
    // a vector of their  UIDs.
    //

    vector<uint64_t> fetchMailBoxMessages(CIMAP& imap, const string& mailBoxStr, uint64_t searchUID, int retryCount) {

        CIMAPParse::COMMANDRESPONSE parsedResponse;
        string commandStr;
        vector<uint64_t> messageID { 0 };

        cout << "MAIL BOX [" << mailBoxStr << "]" << endl;

        // SELECT mailbox

        parsedResponse = sendCommandRetry(imap, mailBoxStr, "SELECT " + mailBoxStr, retryCount);

        // SEARCH for all present email and then create an archive for them.

        if (searchUID != 0) {
            cout << "Searching from [" << to_string(searchUID) << "]" << endl;
            commandStr = "UID SEARCH " + to_string(searchUID) + ":*";
        } else {
            commandStr = "UID SEARCH 1:*";
        }

        parsedResponse = sendCommandRetry(imap, mailBoxStr, commandStr, retryCount);
        if (parsedResponse) {
            if ((parsedResponse->indexes.size() == 1) && (parsedResponse->indexes[0] == searchUID)) {
                parsedResponse->indexes.clear();
            }
        }

        return (parsedResponse->indexes);

    }

    //
    // For a given message UID fetch its subject line and body and return as a pair.
    //

    pair<string, string> fetchEmailContents(CIMAP& imap, const string& mailBoxNameStr, uint64_t uid, int retryCount) {

        string subjectStr;
        string emailBodyStr;
        CIMAPParse::COMMANDRESPONSE parsedResponse;
        
        parsedResponse = sendCommandRetry(imap, mailBoxNameStr, 
                "UID FETCH " + to_string(uid) + " (BODY[] BODY[HEADER.FIELDS (SUBJECT)])",
                 retryCount);

        if (parsedResponse) {

            for (auto& fetchEntry : parsedResponse->fetchList) {
                cout << "EMAIL MESSAGE NO. [" << fetchEntry.index << "]" << endl;
                for (auto& resp : fetchEntry.responseMap) {
                    if (resp.first.find("BODY[]") == 0) {
                        emailBodyStr = std::move(resp.second);
                    } else if (resp.first.find("BODY[HEADER.FIELDS (SUBJECT)]") == 0) {
                        if (resp.second.find("Subject:") != string::npos) { // Contains "Subject:"
                            subjectStr = resp.second.substr(8);
                            subjectStr = CMIME::convertMIMEStringToASCII(subjectStr);
                            if (subjectStr.length() > kMaxSubjectLine) { // Truncate for file name
                                subjectStr = subjectStr.substr(0, kMaxSubjectLine);
                            }
                            for (auto &ch : subjectStr) { // Remove all but alpha numeric from subject
                                if (!isalnum(ch)) ch = ' ';
                            }
                        }
                    }
                }
            }
            
        }

        return (make_pair(subjectStr, emailBodyStr));

    }

} // namespace Pendulum_MailBox
