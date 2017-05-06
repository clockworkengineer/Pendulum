/*
 * File:   Pendulum_CommandLine.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on Apil 10, 2017, 2:34 PM
 * 
 * Copyright 2017.
 * 
 */

#ifndef PENDULUM_COMMANDLINE_HPP
#define PENDULUM_COMMANDLINE_HPP

//
// C++ STL definitions
//

#include <string>

// =========
// NAMESPACE
// =========

namespace Pendulum_CommandLine {

    //
    // Decoded option argument data.
    //
    
    struct PendulumOptions {
        std::string userNameStr;            // Email account user name
        std::string userPasswordStr;        // Email account user name password
        std::string serverURLStr;           // IMAP server URL
        std::string mailBoxNameStr;         // Mailbox name
        std::string destinationFolderStr;   // Destination folder for attachments
        std::string configFileNameStr;      // Configuration file name
        bool bOnlyUpdates { false };        // = true search from UID of last .eml archived
        bool bAllMailBoxes { false };       // = true archive all mailboxes
        int pollTime { 0 };                 // Poll time in minutes
        int retryCount { 5 };               // Server reconnect retry count
        std::string logFileNameStr;         // Log file

    };

    PendulumOptions fetchCommandLineOptions(int argc, char** argv);

} // namespace Pendulum_CommandLine
#endif /* PENDULUM_COMMANDLINE_HPP */

