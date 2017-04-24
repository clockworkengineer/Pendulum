/*
 * File:   Pendulum_MailBox.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on Apil 10, 2017, 2:34 PM
 * 
 * Copyright 2017.
 * 
 */

#ifndef PENDULUM_MAILBOX_HPP
#define PENDULUM_MAILBOX_HPP

//
// C++ STL definitions
//

#include <string>
#include <vector>
#include <utility>

//
// Antikythera Classes
//

#include "CIMAP.hpp"

// =========
// NAMESPACE
// =========

namespace Pendulum_MailBox {
    
    // =======
    // IMPORTS
    // =======

    using Antik::IMAP::CIMAP;
    
    //
    // Mailbox details
    //
    
    struct MailBoxDetails {
        std::string nameStr;
        std::uint64_t searchUID;
    };
    
    //
    // IMAP server connection data
    //
    
    struct ServerConn {
        CIMAP server;           // IMAP server connection
        int connectCount { 0 }; // Connection count
        int retryCount;         // Retry count
    };

    //
    // Maximum subject line to take in file name
    //

    constexpr const int kMaxSubjectLine = 80;
    
    
    //
    // Initial server connect with retry
    //
    
    void serverConnect(ServerConn& imapConnection);

    //
    // Return a vector of mailbox names to be processed
    //
    
    std::vector<MailBoxDetails> fetchMailBoxList(ServerConn& imapConnection, const std::string& mailBoxNameStr, bool bAllMailBoxes);

    //
    // Return a vector of e-mail  UIDs to be archived (.eml file created).
    //
    
    std::vector<uint64_t> fetchMailBoxMessages(ServerConn& imapConnection, const MailBoxDetails& mailBoxEntry);

    //
    // Return string pair of an e-mails subject line and contents.
    //
    
    std::pair<std::string, std::string> fetchEmailContents(ServerConn& imapConnection, const std::string& mailBoxNameStr, std::uint64_t uid);

} // namespace Pendulum_MailBox

#endif /* PENDULUM_MAILBOX_HPP */

