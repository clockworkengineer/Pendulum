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
// C++ STL
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
        std::string name;        // Mailbox name
        std::uint64_t searchUID;    // Current search UID
        std::string path;        // Email archive folder path     
    };
    
    //
    // IMAP server connection data
    //
    
    struct ServerConnection {
        CIMAP server;                    // IMAP server connection
        std::string reconnectMailBox; // Reconnect select mailbox
        int connectCount { 0 };          // Connection count
        int retryCount;                  // Retry count
    };

    //
    // Maximum subject line to take in file name
    //

    constexpr int kMaxSubjectLine = 80;
    
    
    //
    // Initial server connect with retry
    //
    
    void serverConnect(ServerConnection& imapConnection);

    //
    // Return a vector of mailbox names to be processed
    //
    
    std::vector<MailBoxDetails> fetchMailBoxList(ServerConnection& imapConnection, const std::string& mailBoxList, 
                                                 const std::string& ignoreList, bool bAllMailBoxes);

    //
    // Return a vector of e-mail  UIDs to be archived (.eml file created).
    //
    
    std::vector<uint64_t> fetchMailBoxMessages(ServerConnection& imapConnection, const MailBoxDetails& mailBoxEntry);

    //
    // Return string pair of an e-mails subject line and contents.
    //
    
    std::pair<std::string, std::string> fetchEmailContents(ServerConnection& imapConnection, std::uint64_t uid);

} // namespace Pendulum_MailBox
#endif /* PENDULUM_MAILBOX_HPP */

