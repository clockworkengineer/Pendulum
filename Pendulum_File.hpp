#ifndef PENDULUM_FILE_HPP
#define PENDULUM_FILE_HPP

//
// C++ STL
//

#include <string>
#include <utility>

namespace Pendulum_File {
    
    //
    // Create destination for mailbox archive
    //

    std::string createMailboxFolder(const std::string& destFolder, const std::string& mailBoxName);
     
    //
    // Create .eml file for a given e-mail message
    //

    void createEMLFile(const std::pair<std::string, std::string>& emailContents, std::uint64_t uid, const std::string& destFolder);

    //
    // Return the UID of the newest e-mail message archived for a mailbox.
    //

    std::uint64_t getNewestUID(const std::string& destFolder);
    

} // namespace Pendulum_File
#endif /* PENDULUM_FILE_HPP */

