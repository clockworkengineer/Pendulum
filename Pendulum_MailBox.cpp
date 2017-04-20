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
    
    using namespace Antik::Mail;
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
            
            string commandResponseStr;
            commandResponseStr = imap.sendCommand(commandStr);
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
    // Returning a boolean to indicate success or failure.
    //
    
    static bool serverReconnect(CIMAP& imap, const string& mailBoxNameStr) {

        bool bReconnected=false;

        try {

            imap.connect();

            if (imap.getConnectedStatus()) {
                CIMAPParse::COMMANDRESPONSE parsedResponse;
                parsedResponse = sendCommand(imap, "SELECT " + mailBoxNameStr);
                if ((parsedResponse) && (parsedResponse->status == CIMAPParse::RespCode::OK)) {
                    cerr << "Reconnected to MailBox [" << mailBoxNameStr << "]" << endl;
                    bReconnected = true;
                }
            }
        } catch (...) { // Catch all as we are trying to reconnect
        }

        return (bReconnected);

    }
    
    //
    // Send a command to IMAP server. If the server disconnects try to reconnect
    // and resend command; a total of retryCount times. Catch all exceptions and
    // if not connected then try to reconnect otherwise re-throw.
    //
    
    static CIMAPParse::COMMANDRESPONSE sendCommandRetry(CIMAP& imap, 
            const string& mailBoxNameStr, const string& commandStr, int retryCount) {
        
        CIMAPParse::COMMANDRESPONSE parsedResponse;
        
        do {
            
            try {
                parsedResponse = sendCommand(imap, commandStr);
                break;
            } catch (...) {
                // If not connected try to reconnect otherwise re-throw error
                
                if (!imap.getConnectedStatus()) {
                    cerr << "Server Disconnect.\nTrying to reconnect ..." << endl;
                    while ((retryCount > 0) && !serverReconnect(imap, mailBoxNameStr)) {
                        cerr << "Trying to reconnect ..." << endl;
                        retryCount--;
                    }
                } else {
                    throw;
                }
            }

        } while (retryCount > 0);
        
        // If not reconnected then throw
        
        if (!imap.getConnectedStatus()) {
            throw CIMAP::Exception("Failed to reconnect to server.");
        }

        return(parsedResponse);
        
    }
    
    // ================
    // PUBLIC FUNCTIONS
    // ================

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
