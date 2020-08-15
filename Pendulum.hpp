#ifndef PENDULUM_HPP
#define PENDULUM_HPP

// =========
// NAMESPACE
// =========

namespace Pendulum {

    //
    // .eml file extention
    //

    constexpr char const *kEMLFileExt{".eml"};
    
    //
    // Main processing functionality (archive e-mail).
    //
    
    void archiveEmail(int argc, char** argv);

}
#endif /* PENDULUM_HPP */

