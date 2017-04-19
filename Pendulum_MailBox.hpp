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

    using Antik::Mail::CIMAP;

    //
    // Maximum subject line to take in file name
    //

    constexpr const int kMaxSubjectLine = 80;
    
    //
    // Maximum subject line to take in file name
    //

    constexpr const int kReconnectRetryCount = 5;

    //
    // Return a vector of mailbox names to be processed
    //
    
    std::vector<std::string> fetchMailBoxList(CIMAP& imap, const std::string& mailBoxNameStr, bool bAllMailBoxes);

    //
    // Return a vector of e-mail  UIDs to be archived (.eml file created).
    //
    
    std::vector<uint64_t> fetchMailBoxMessages(CIMAP& imap, std::string& mailBoxStr, std::uint64_t searchUID);

    //
    // Return string pair of an e-mails subject line and contents.
    //
    
    std::pair<std::string, std::string> fetchEmailContents(CIMAP& imap, const std::string& mailBoxNameStr, std::uint64_t uid);

} // namespace Pendulum_MailBox

#endif /* PENDULUM_MAILBOX_HPP */

