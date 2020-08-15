//
// Module: Pendulum_CommandLine
//
// Description: Pendulum program options processing functionality.
// 
// Dependencies: 
// 
// C11++              : Use of C11++ features.
// Antik Classes      : CFile.
// Boost              : File system, program option,.
//

// =============
// INCLUDE FILES
// =============

//
// C++ STL
//

#include <iostream>

//
// Antik Classes
//

#include "CFile.hpp"

//
// Pendulum command line processing
//

#include "Pendulum_CommandLine.hpp"

//
// Boost file system & program options
//

#include "boost/program_options.hpp"

// =========
// NAMESPACE
// =========

namespace Pendulum_CommandLine {

    // =======
    // IMPORTS
    // =======
    
    using namespace Antik::File;
        
    namespace po = boost::program_options;
    
    // ===============
    // LOCAL FUNCTIONS
    // ===============

    //
    // Add options common to both command line and config file
    //

    static void addCommonOptions(po::options_description& commonOptions, PendulumOptions& argData) {

        commonOptions.add_options()
                ("server,s", po::value<std::string>(&argData.serverURL)->required(), "IMAP Server URL and port")
                ("user,u", po::value<std::string>(&argData.userName)->required(), "Account username")
                ("password,p", po::value<std::string>(&argData.userPassword)->required(), "User password")
                ("mailbox,m", po::value<std::string>(&argData.mailBoxList)->required(), "Mailbox name (or mailbox comma separated list)")
                ("destination,d", po::value<std::string>(&argData.destinationFolder)->required(), "Destination folder for archived e-mail")
                ("poll", po::value<int>(&argData.pollTime), "Poll time in minutes")
                ("retry,r", po::value<int>(&argData.retryCount), "Server reconnect retry count")
                ("log,l",po::value<std::string>(&argData.logFileName), "Log file")
                ("ignore,i",po::value<std::string>(&argData.ignoreList), "Ignore mailbox list")
                ("updates,u", "Search since last file archived.")
                ("all,a", "Download files for all mailboxes.");

    }

    // ================
    // PUBLIC FUNCTIONS
    // ================

    //
    // Read in and process command line options using boost.
    //

    PendulumOptions fetchCommandLineOptions(int argc, char** argv) {

        PendulumOptions optionData;

        // Define and parse the program options

        po::options_description commandLine("Program Options");
        commandLine.add_options()
                ("help", "Print help messages")
                ("config,c", po::value<std::string>(&optionData.configFileName), "Config File Name");

        addCommonOptions(commandLine, optionData);

        po::options_description configFile("Config Files Options");

        addCommonOptions(configFile, optionData);

        po::variables_map vm {};

        try {

            // Process options

            po::store(po::parse_command_line(argc, argv, commandLine), vm);

            // Display options and exit with success

            if (vm.count("help")) {
                std::cout << "Pendulum Email Archiver" << std::endl << commandLine << std::endl;
                exit(EXIT_SUCCESS);
            }

            if (vm.count("config")) {
                if (CFile::exists(vm["config"].as<std::string>())) {
                    std::ifstream configFileStream{vm["config"].as<std::string>()};
                    if (configFileStream) {
                        po::store(po::parse_config_file(configFileStream, configFile), vm);
                    }
                } else {
                    throw po::error("Specified config file does not exist.");
                }
            }

            // Search for new e-mails only

            if (vm.count("updates")) {
                optionData.bOnlyUpdates = true;
            }

            // Download all mailboxes

            if (vm.count("all")) {
                optionData.bAllMailBoxes = true;
            }

            po::notify(vm);

        } catch (po::error& e) {
            std::cerr << "Pendulum Error: " << e.what() << "\n" << std::endl;
            exit(EXIT_FAILURE);
        }

        return(optionData);
        
    }

} // namespace Pendulum_CommandLine

