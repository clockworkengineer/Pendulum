#include "HOST.hpp"
/*
 * File:   Pendulum_CommandLine.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on Apil 10, 2017, 2:34 PM
 * 
 * Copyright 2017.
 * 
 */

//
// Module: Pendulum_CommandLine
//
// Description: Pendulum program options processing functionality.
// 
// Dependencies: 
// 
// C11++              : Use of C11++ features.
// Boost              : File system, program option,.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL definitions
//

#include <iostream>

//
// Pendulum_CommandLine namespace definitions
//

#include "Pendulum_CommandLine.hpp"

//
// Boost file system & program options definitions
//

#include "boost/program_options.hpp" 
#include <boost/filesystem.hpp>

// =========
// NAMESPACE
// =========

namespace Pendulum_CommandLine {

    // =======
    // IMPORTS
    // =======

    using namespace std;
        
    namespace po = boost::program_options;
    namespace fs = boost::filesystem;

    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Add options common to both command line and config file
    //

    static void addCommonOptions(po::options_description& commonOptions, ParamArgData& argData) {

        commonOptions.add_options()
                ("server,s", po::value<string>(&argData.serverURLStr)->required(), "IMAP Server URL and port")
                ("user,u", po::value<string>(&argData.userNameStr)->required(), "Account username")
                ("password,p",                po::value<string>(&argData.userPasswordStr)->required(), "User password")
                ("mailbox,m", po::value<string>(&argData.mailBoxNameStr)->required(), "Mailbox name")
                ("destination,d", po::value<string>(&argData.destinationFolderStr)->required(), "Destination for archived e-mail")
                ("poll", po::value<int>(&argData.pollTime), "Poll time in minutes")
                ("retry,r", po::value<int>(&argData.retryCount), "Server reconnect retry count")
                ("log,l",po::value<string>(&argData.logFileNameStr), "Log file")
                ("updates,u", "Search since last file archived.")
                ("all,a", "Download files for all mailboxes.");

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Read in and process command line arguments using boost.
    //

    ParamArgData fetchCommandLineArgs(int argc, char** argv) {

        ParamArgData argumentData;

        // Define and parse the program options

        po::options_description commandLine("Program Options");
        commandLine.add_options()
                ("help", "Print help messages")
                ("config,c", po::value<string>(&argumentData.configFileNameStr)->required(), "Config File Name");

        addCommonOptions(commandLine, argumentData);

        po::options_description configFile("Config Files Options");

        addCommonOptions(configFile, argumentData);

        po::variables_map vm {};

        try {

            // Process arguments

            po::store(po::parse_command_line(argc, argv, commandLine), vm);

            // Display options and exit with success

            if (vm.count("help")) {
                cout << "Pendulum Example Application" << endl << commandLine << endl;
                exit(EXIT_SUCCESS);
            }

            if (vm.count("config")) {
                if (fs::exists(vm["config"].as<string>().c_str())) {
                    ifstream ifs{vm["config"].as<string>().c_str()};
                    if (ifs) {
                        po::store(po::parse_config_file(ifs, configFile), vm);
                    }
                } else {
                    throw po::error("Specified config file does not exist.");
                }
            }

            // Search for new e-mails only

            if (vm.count("updates")) {
                argumentData.bOnlyUpdates = true;
            }

            // Download all mailboxes

            if (vm.count("all")) {
                argumentData.bAllMailBoxes = true;
            }

            po::notify(vm);

        } catch (po::error& e) {
            cerr << "Pendulum Error: " << e.what() << endl << endl;
            cerr << commandLine << endl;
            exit(EXIT_FAILURE);
        }

        return(argumentData);
        
    }

} // namespace Pendulum_CommandLine

