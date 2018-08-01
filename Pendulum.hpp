/*
 * File:   Pendulum.hpp
 * 
 * Author: Robert Tizzard
 * 
 * Created on October 24, 2016, 2:33 PM
 *
 * Copyright 2016.
 *
 */

#ifndef PENDULUM_HPP
#define PENDULUM_HPP

// =========
// NAMESPACE
// =========

namespace Pendulum {

    //
    // .eml file extention
    //

    constexpr const char *kEMLFileExt{".eml"};
    
    //
    // Main processing functionality (archive e-mail).
    //
    
    void archiveEmail(int argc, char** argv);

}
#endif /* PENDULUM_HPP */

