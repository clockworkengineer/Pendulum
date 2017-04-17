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
// Description: Pendulum IMAP mailbox functionality.
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
    // At present it catches any thrown exceptions reports them then re-throws.
    //

    static CIMAPParse::COMMANDRESPONSE sendCommand(CIMAP& imap, const string& mailBoxNameStr,
            const string& commandStr) {

        string commandResponseStr;
        CIMAPParse::COMMANDRESPONSE parsedResponse;

        try {
            commandResponseStr = imap.sendCommand(commandStr);
            parsedResponse = CIMAPParse::parseResponse(commandResponseStr);
        } catch (CIMAP::Exception &e) {
            cerr << "IMAP ERROR: Need to reconnect to server" << endl;
            throw (e);
        } catch (CIMAPParse::Exception &e) {
            cerr << "RESPONSE IN ERRROR: [" << commandResponseStr << "]" << endl;
            throw (e);
        }

        // Report server disconnect or command error
        
        if (parsedResponse->bBYESent) {
            throw CIMAP::Exception("Received BYE from server: " + parsedResponse->errorMessageStr);
        } else if (parsedResponse->status != CIMAPParse::RespCode::OK) {
            throw CIMAP::Exception(commandStr + ": " + parsedResponse->errorMessageStr);
        }

        return (parsedResponse);

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Convert list of comma separated mailbox names / list all mailboxes and 
    // place into vector of mailbox name strings to be returned.
    //

    vector<string> fetchMailBoxList(CIMAP& imap, const string& mailBoxNameStr, bool bAllMailBoxes) {

        vector<string> mailBoxList;

        if (bAllMailBoxes) {
            
            string commandStr;
            CIMAPParse::COMMANDRESPONSE parsedResponse;

            commandStr = "LIST \"\" *";
            parsedResponse = sendCommand(imap, "", commandStr);

            if (parsedResponse) {

                for (auto mailBoxEntry : parsedResponse->mailBoxList) {
                    if (mailBoxEntry.mailBoxNameStr.front() == ' ') { 
                        mailBoxEntry.mailBoxNameStr = mailBoxEntry.mailBoxNameStr.substr(1);
                    }
                    if (mailBoxEntry.attributesStr.find("\\Noselect") == string::npos) {
                        mailBoxList.push_back(mailBoxEntry.mailBoxNameStr);
                    }
                }

            }

        } else {
            
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

    vector<uint64_t> fetchMailBoxMessages(CIMAP& imap, string& mailBoxStr, uint64_t searchUID) {

        CIMAPParse::COMMANDRESPONSE parsedResponse;
        string commandStr;
        vector<uint64_t> messageID { 0 };

        cout << "MAIL BOX [" << mailBoxStr << "]" << endl;

        // SELECT mailbox

        commandStr = "SELECT " + mailBoxStr;
        parsedResponse = sendCommand(imap, mailBoxStr, commandStr);

        // SEARCH for all present email and then create an archive for them.

        if (searchUID != 0) {
            cout << "Searching from [" << to_string(searchUID) << "]" << endl;
            commandStr = "UID SEARCH " + to_string(searchUID) + ":*";
        } else {
            commandStr = "UID SEARCH 1:*";
        }

        parsedResponse = sendCommand(imap, mailBoxStr, commandStr);
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

    pair<string, string> fetchEmailContents(CIMAP& imap, const string& mailBoxNameStr, uint64_t uid) {

        string commandStr;
        string subjectStr;
        string emailBodyStr;
        CIMAPParse::COMMANDRESPONSE parsedResponse;

        commandStr = "UID FETCH " + to_string(uid) + " (BODY[] BODY[HEADER.FIELDS (SUBJECT)])";
        parsedResponse = sendCommand(imap, mailBoxNameStr, commandStr);

        if (parsedResponse) {

            for (auto fetchEntry : parsedResponse->fetchList) {
                cout << "EMAIL MESSAGE NO. [" << fetchEntry.index << "]" << endl;
                for (auto resp : fetchEntry.responseMap) {
                    if (resp.first.find("BODY[]") == 0) {
                        emailBodyStr = resp.second;
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
