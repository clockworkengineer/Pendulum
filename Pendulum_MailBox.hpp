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

namespace Pendulum_MailBox {

    using Antik::Mail::CIMAP;

    //
    // Maximum subject line to take in file name
    //

    constexpr const int kMaxSubjectLine = 80;

    std::vector<std::string> fetchMailBoxList(CIMAP& imap, const std::string& mailBoxNameStr, bool bAllMailBoxes);

    std::vector<uint64_t> fetchMailBoxMessages(CIMAP& imap, std::string& mailBoxStr, std::uint64_t searchUID);

    std::pair<std::string, std::string> fetchEmailContents(CIMAP& imap, const std::string& mailBoxNameStr, std::uint64_t index);

} // namespace Pendulum_MailBox

#endif /* PENDULUM_MAILBOX_HPP */

