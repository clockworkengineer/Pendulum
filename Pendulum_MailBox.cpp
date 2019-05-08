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
// Antik Classes      : CIMAP, CIMAPParse, CMIME.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <iostream>
#include <algorithm>

//
// Antik Classes
//

#include "CIMAPParse.hpp"
#include "CMIME.hpp"

//
// Pendulum mailbox.
//

#include "Pendulum_MailBox.hpp"

// =========
// NAMESPACE
// =========

namespace Pendulum_MailBox {

    // =======
    // IMPORTS
    // =======

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

    static CIMAPParse::COMMANDRESPONSE sendCommand(ServerConnection& imapConnection, const std::string& command) {

        CIMAPParse::COMMANDRESPONSE parsedResponse;

        try {
            
            std::string commandResponse { imapConnection.server.sendCommand(command) };
            if (commandResponse.size()) {
                parsedResponse = CIMAPParse::parseResponse(commandResponse);
            }
       
        } catch (...) {
            throw;  // Re-throw IMAP and parser exceptions
        }

        // Report server disconnect or command error after command response 
        // successfully received and response parsed.

        if (parsedResponse->byeSent) {
            throw CIMAP::Exception("Received BYE from server: " + parsedResponse->errorMessage);
        } else if (parsedResponse->status != CIMAPParse::RespCode::OK) {
            throw CIMAP::Exception(command + ": " + parsedResponse->errorMessage);
        }

        return (parsedResponse);

    }

    //
    // Reconnect to IMAP server and select passed in (current) mailbox. 
    //
    
    static void serverReconnect(ServerConnection& imapConnection) {

        serverConnect(imapConnection);

        if (imapConnection.server.getConnectedStatus() && imapConnection.reconnectMailBox.size()) {
            CIMAPParse::COMMANDRESPONSE parsedResponse;
            parsedResponse = sendCommand(imapConnection, "SELECT " + imapConnection.reconnectMailBox);
            if ((parsedResponse) && (parsedResponse->status == CIMAPParse::RespCode::OK)) {
                std::cerr << "Reconnected to MailBox [" << imapConnection.reconnectMailBox << "]" << std::endl;
            }
        }

    }
    
    //
    // Send a command to IMAP server. If the server disconnects try to reconnect
    // and resend command even if it was successful.
    //
    
    static CIMAPParse::COMMANDRESPONSE sendCommandRetry(ServerConnection& imapConnection, const std::string& command) {
        
        CIMAPParse::COMMANDRESPONSE parsedResponse;

        try {
            parsedResponse = sendCommand(imapConnection, command);
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
                std::cerr << "Server Disconnect.\nTrying to reconnect ..." << std::endl;
                serverReconnect(imapConnection);
                parsedResponse = sendCommand(imapConnection, command);
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
                std::cout << "Connected." << std::endl;
                break;
            } 
            
            // Retry count == 0 try so throw last saved error
            
            if (!(--retryCount)) {   
                std::rethrow_exception(thrownException);
            }
            
            std::cerr << "Trying to reconnect ..." << std::endl;

        }

    }
    
    //
    // Convert list of comma separated mailbox names / list all mailboxes and 
    // place into vector of mailbox name strings to be returned.
    //

    std::vector<MailBoxDetails> fetchMailBoxList(ServerConnection& imapConnection, const std::string& mailBoxList,  
                           const std::string& ignoreList, bool bAllMailBoxes) {

        std::vector<MailBoxDetails> mailBoxesList;
        std::vector<std::string> ignoreMailBoxesList;

        // Create mailbox ignore list
        
        if (!ignoreList.empty()) { 
            
            std::istringstream ignoreListStream { ignoreList };
            
            for (std::string ignoreMailbox; getline(ignoreListStream, ignoreMailbox, ',');) {
                ignoreMailbox = ignoreMailbox.substr(ignoreMailbox.find_first_not_of(' '));
                ignoreMailbox = ignoreMailbox.substr(0, ignoreMailbox.find_last_not_of(' ') + 1);
                ignoreMailBoxesList.push_back(ignoreMailbox );
            } 
            
        }
        
        if (bAllMailBoxes) {
            
            CIMAPParse::COMMANDRESPONSE parsedResponse;
            
            // Get list of all mailboxes
            
            parsedResponse = sendCommandRetry(imapConnection, "LIST \"\" *");

            if (parsedResponse) {

                // Ignore mailbox with attribute no select or that is on ignore list
                
                for (auto& mailBoxEntry : parsedResponse->mailBoxList) {
                    if ((std::find(ignoreMailBoxesList.begin(), ignoreMailBoxesList.end(), mailBoxEntry.mailBoxName) == ignoreMailBoxesList.end()) &&
                        (mailBoxEntry.attributes.find("\\Noselect") == std::string::npos)) {
                        mailBoxesList.push_back( { mailBoxEntry.mailBoxName, 0} );
                    } else {
                        std::cout << "Ignoring mailbox [" << mailBoxEntry.mailBoxName << "]" << std::endl;                       
                    }
                }

            }

        } else {
            
            // Add mailbox list from config file or command line parameter
            
            std::istringstream mailBoxStream { mailBoxList };
            
            for (std::string mailBox; std::getline(mailBoxStream, mailBox, ',');) {
                mailBox = mailBox.substr(mailBox.find_first_not_of(' '));
                mailBox = mailBox.substr(0, mailBox.find_last_not_of(' ') + 1);
                if (std::find(ignoreMailBoxesList.begin(), ignoreMailBoxesList.end(), mailBox) == ignoreMailBoxesList.end()) {
                    mailBoxesList.push_back({ mailBox, 0} );
                } else {
                    std::cout << "Ignoring mailbox [" << mailBox << "]" << std::endl;
                }
            }
            
        }

        return (mailBoxesList);

    }

    //
    // Search a mailbox for e-mails with UIDs greater than searchUID and return
    // a vector of their  UIDs.
    //

    std::vector<uint64_t> fetchMailBoxMessages(ServerConnection& imapConnection, const MailBoxDetails& mailBoxEntry) {

        CIMAPParse::COMMANDRESPONSE parsedResponse;
        std::vector<uint64_t> messageID {};

        std::cout << "MAIL BOX [" << mailBoxEntry.name << "]" << std::endl;

        // SELECT mailbox (ignore response)

        parsedResponse = sendCommandRetry(imapConnection, "SELECT " + mailBoxEntry.name);

        // SEARCH for all or new e-mail messages

        uint64_t searchUID { mailBoxEntry.searchUID };
        if (searchUID == 0) {
            searchUID++; // Search from 1 (all messages)
        }
        
        std::cout << "Searching from UID [" << std::to_string(searchUID) << "]" << std::endl;
        
        parsedResponse = sendCommandRetry(imapConnection, "UID SEARCH UID " + std::to_string(searchUID) + ":*");

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

    std::pair<std::string, std::string> fetchEmailContents(ServerConnection& imapConnection, uint64_t uid) {

        std::string subject;
        std::string emailBody;
        CIMAPParse::COMMANDRESPONSE parsedResponse;
        
        parsedResponse = sendCommandRetry(imapConnection, 
                "UID FETCH " + std::to_string(uid) + " (BODY[] BODY[HEADER.FIELDS (SUBJECT)])");

        if (parsedResponse) {

            for (auto& fetchEntry : parsedResponse->fetchList) {
                std::cout << "EMAIL MESSAGE NO. [" << fetchEntry.index << "]" << std::endl;
                for (auto& resp : fetchEntry.responseMap) {
                    if (resp.first.find("BODY[]") == 0) {
                        emailBody = resp.second;
                    } else if (resp.first.find("BODY[HEADER.FIELDS (SUBJECT)]") == 0) {
                        if (resp.second.find("Subject:") != std::string::npos) { // Contains "Subject:"
                            subject = resp.second.substr(8);
                            subject = CMIME::convertMIMEStringToASCII(subject);
                            if (subject.length() > kMaxSubjectLine) { // Truncate for file name
                                subject = subject.substr(0, kMaxSubjectLine);
                            }
                            for (auto &ch : subject) { // Remove all but alpha numeric from subject
                                if (!isalnum(ch)) ch = ' ';
                            }
                        }
                    }
                }
            }
            
        }

        return (make_pair(subject, emailBody));

    }

} // namespace Pendulum_MailBox
