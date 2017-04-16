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

    using namespace Antik::Mail;
    using namespace Antik::File;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Parse command response and return pointer to parsed data.
    //

    static CIMAPParse::COMMANDRESPONSE parseCommandResponse(const std::string& commandStr,
            const std::string& commandResponseStr) {

        CIMAPParse::COMMANDRESPONSE parsedResponse;

        try {
            parsedResponse = CIMAPParse::parseResponse(commandResponseStr);
        } catch (CIMAPParse::Exception &e) {
            std::cerr << "RESPONSE IN ERRROR: [" << commandResponseStr << "]" << std::endl;
            throw (e);
        }

        if (parsedResponse->bBYESent) {
            throw CIMAP::Exception("Received BYE from server: " + parsedResponse->errorMessageStr);
        } else if (parsedResponse->status != CIMAPParse::RespCode::OK) {
            throw CIMAP::Exception(commandStr + ": " + parsedResponse->errorMessageStr);
        }

        return (parsedResponse);

    }

    //
    // Send command to IMAP server. At present it checks for any errors and just exits.
    //

    static std::string sendCommand(CIMAP& imap, const std::string& mailBoxNameStr,
            const std::string& commandStr) {

        std::string commandResponseStr;

        try {
            commandResponseStr = imap.sendCommand(commandStr);
        } catch (CIMAP::Exception &e) {
            std::cerr << "IMAP ERROR: Need to reconnect to server" << std::endl;
            throw (e);
        }

        return (commandResponseStr);

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Convert list of comma separated mailbox names / list all mailboxes and 
    // place into vector of mailbox name strings to be returned.
    //

    std::vector<std::string> fetchMailBoxList(CIMAP& imap, const std::string& mailBoxNameStr, bool bAllMailBoxes) {

        std::vector<std::string> mailBoxList;

        if (bAllMailBoxes) {
            std::string commandStr, commandResponseStr;
            CIMAPParse::COMMANDRESPONSE parsedResponse;

            commandStr = "LIST \"\" *";
            commandResponseStr = sendCommand(imap, "", commandStr);
            parsedResponse = parseCommandResponse(commandStr, commandResponseStr);

            if (parsedResponse) {

                for (auto mailBoxEntry : parsedResponse->mailBoxList) {
                    if (mailBoxEntry.mailBoxNameStr.front() == ' ') mailBoxEntry.mailBoxNameStr = mailBoxEntry.mailBoxNameStr.substr(1);
                    if (mailBoxEntry.attributesStr.find("\\Noselect") == std::string::npos) {
                        mailBoxList.push_back(mailBoxEntry.mailBoxNameStr);
                    }
                }

            }

        } else {
            std::istringstream mailBoxStream(mailBoxNameStr);
            for (std::string mailBoxStr; std::getline(mailBoxStream, mailBoxStr, ',');) {
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

    std::vector<uint64_t> fetchMailBoxMessages(CIMAP& imap, std::string& mailBoxStr, std::uint64_t searchUID) {

        CIMAPParse::COMMANDRESPONSE parsedResponse;
        std::string commandStr;
        std::string commandResponseStr;
        std::vector<uint64_t> messageID;

        std::cout << "MAIL BOX [" << mailBoxStr << "]" << std::endl;

        // SELECT mailbox

        commandStr = "SELECT " + mailBoxStr;
        commandResponseStr = sendCommand(imap, mailBoxStr, commandStr);
        parsedResponse = parseCommandResponse(commandStr, commandResponseStr);

        // SEARCH for all present email and then create an archive for them.

        if (searchUID != 0) {
            std::cout << "Searching from [" << std::to_string(searchUID) << "]" << std::endl;
            commandStr = "UID SEARCH " + std::to_string(searchUID) + ":*";
        } else {
            commandStr = "UID SEARCH 1:*";
        }

        commandResponseStr = sendCommand(imap, mailBoxStr, commandStr);
        parsedResponse = parseCommandResponse(commandStr, commandResponseStr);
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

    std::pair<std::string, std::string> fetchEmailContents(CIMAP& imap, const std::string& mailBoxNameStr, std::uint64_t index) {

        std::string commandStr, commandResponseStr, subject, emailBody;
        CIMAPParse::COMMANDRESPONSE parsedResponse;

        commandStr = "UID FETCH " + std::to_string(index) + " (BODY[] BODY[HEADER.FIELDS (SUBJECT)])";
        commandResponseStr = sendCommand(imap, mailBoxNameStr, commandStr);
        parsedResponse = parseCommandResponse(commandStr, commandResponseStr);

        if (parsedResponse) {

            for (auto fetchEntry : parsedResponse->fetchList) {
                std::cout << "EMAIL MESSAGE NO. [" << fetchEntry.index << "]" << std::endl;
                for (auto resp : fetchEntry.responseMap) {
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
                                if (!std::isalnum(ch)) ch = ' ';
                            }
                        }
                    }
                }
            }
        }

        return (std::make_pair(subject, emailBody));

    }

} // namespace Pendulum_MailBox
