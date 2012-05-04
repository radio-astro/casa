
/*
 *  VOCLIENT.HPP == C++ Wrapper functions for VOClient Interface.
 *
 *  W. Young, NRAO 2012
 *
 * Note dal,,registry, sesame, skybot, message  functions are nested in vao namespace
 * i.e.
 * vao::dal::myfunction is the same as voc_myfunction
 * vao::message::myfunction is the same as msg_myfunction
 *
 * The use of namespaces is a more natural C++ way of doing things.
 */

#ifndef _VOCLIENT_HPP
#define _VOCLIENT_HPP

#define _VOCLIENT_LIB_ 1

// Include from the voclient C library
#include <VOClient.h>

// Broken up to make making python modules easier
#include <tools/VOdal.hpp>
#include <tools/VOregistry.hpp>
#include <tools/VOsesame.hpp>
#include <tools/VOmessage.hpp>
#include <tools/VOskybot.hpp>


#endif /* _VOCLIENT_HPP */
