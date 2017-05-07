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

    static CIMAPParse::COMMANDRESPONSE sendCommand(ServerConnection& imapConnection, const string& commandStr) {

        CIMAPParse::COMMANDRESPONSE parsedResponse;

        try {
            
            string commandResponseStr { imapConnection.server.sendCommand(commandStr) };
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
    
    static void serverReconnect(ServerConnection& imapConnection) {

        serverConnect(imapConnection);

        if (imapConnection.server.getConnectedStatus() && imapConnection.reconnectMailBoxStr.size()) {
            CIMAPParse::COMMANDRESPONSE parsedResponse;
            parsedResponse = sendCommand(imapConnection, "SELECT " + imapConnection.reconnectMailBoxStr);
            if ((parsedResponse) && (parsedResponse->status == CIMAPParse::RespCode::OK)) {
                cerr << "Reconnected to MailBox [" << imapConnection.reconnectMailBoxStr << "]" << endl;
            }
        }

    }
    
    //
    // Send a command to IMAP server. If the server disconnects try to reconnect
    // and resend command even if it was successful.
    //
    
    static CIMAPParse::COMMANDRESPONSE sendCommandRetry(ServerConnection& imapConnection, const string& commandStr) {
        
        CIMAPParse::COMMANDRESPONSE parsedResponse;

        try {
            parsedResponse = sendCommand(imapConnection, commandStr);
        } catch (...) {
            // If still connected  re-throw error as not connection related.
            if (imapConnection.server.getConnectedStatus()) {
                throw;
            }
        }

        try {
            // The command may have been successful but if a disconnect was detected
            // try to reconnect and repeat command just in case.
            if (!imapConnection.server.getConnectedStatus()) {
                cerr << "Server Disconnect.\nTrying to reconnect ..." << endl;
                serverReconnect(imapConnection);
                parsedResponse = sendCommand(imapConnection, commandStr);
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
    
    void serverConnect(ServerConnection& imapConnection) {

        std::exception_ptr thrownException { nullptr };
        int retryCount {imapConnection.retryCount };
       
        // Connect retry loop
        
        while(true) {

            // Try to connect
            
            try {             
                imapConnection.server.connect(); 
                thrownException = nullptr;  // Possible success
            } catch (...) {
                thrownException = std::current_exception(); // Error        
            }

            // If connected return
            
            if (imapConnection.server.getConnectedStatus() && !thrownException) {
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

    vector<MailBoxDetails> fetchMailBoxList(ServerConnection& imapConnection, const string& mailBoxNameStr, bool bAllMailBoxes) {

        vector<MailBoxDetails> mailBoxList;

        if (bAllMailBoxes) {
            
            CIMAPParse::COMMANDRESPONSE parsedResponse;
            
            // Get list of all mailboxes
            
            parsedResponse = sendCommandRetry(imapConnection, "LIST \"\" *");

            if (parsedResponse) {

                // Ignore mailbox with attribute no select
                
                for (auto& mailBoxEntry : parsedResponse->mailBoxList) {
                    if (mailBoxEntry.attributesStr.find("\\Noselect") == string::npos) {
                        mailBoxList.push_back( { mailBoxEntry.mailBoxNameStr, 0} );
                    }
                }

            }

        } else {
            
            // Add mailbox list from config file or command line parameter
            
            istringstream mailBoxStream { mailBoxNameStr };
            
            for (string mailBoxStr; getline(mailBoxStream, mailBoxStr, ',');) {
                mailBoxStr = mailBoxStr.substr(mailBoxStr.find_first_not_of(' '));
                mailBoxStr = mailBoxStr.substr(0, mailBoxStr.find_last_not_of(' ') + 1);
                mailBoxList.push_back({ mailBoxNameStr, 0} );
            }
            
        }

        return (mailBoxList);

    }

    //
    // Search a mailbox for e-mails with UIDs greater than searchUID and return
    // a vector of their  UIDs.
    //

    vector<uint64_t> fetchMailBoxMessages(ServerConnection& imapConnection, const MailBoxDetails& mailBoxEntry) {

        CIMAPParse::COMMANDRESPONSE parsedResponse;
        vector<uint64_t> messageID {};

        cout << "MAIL BOX [" << mailBoxEntry.nameStr << "]" << endl;

        // SELECT mailbox (ignore response)

        parsedResponse = sendCommandRetry(imapConnection, "SELECT " + mailBoxEntry.nameStr);

        // SEARCH for all or new e-mail messages

        uint64_t searchUID { mailBoxEntry.searchUID };
        if (searchUID == 0) {
            searchUID++; // Search from 1 (all messages)
        }
        
        cout << "Searching from UID [" << to_string(searchUID) << "]" << endl;
        
        parsedResponse = sendCommandRetry(imapConnection, "UID SEARCH UID " + to_string(searchUID) + ":*");

        // Parse response and create vector of message UID(s)
        
        if (parsedResponse) {
            
            for (auto uid :  parsedResponse->indexes) {
                if (uid > mailBoxEntry.searchUID) {
                   messageID.push_back(uid); 
                }
            }
            
        }

        return (messageID);

    }

    //
    // For a given message UID fetch its subject line and body and return as a pair.
    //

    pair<string, string> fetchEmailContents(ServerConnection& imapConnection, uint64_t uid) {

        string subjectStr;
        string emailBodyStr;
        CIMAPParse::COMMANDRESPONSE parsedResponse;
        
        parsedResponse = sendCommandRetry(imapConnection, 
                "UID FETCH " + to_string(uid) + " (BODY[] BODY[HEADER.FIELDS (SUBJECT)])");

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
