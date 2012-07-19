//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software.

#ifndef _H_Debug
#define _H_Debug

#ifdef ALL_LOGGING_DISABLED
#  include "DebugMsgEmpty.hh"
#else
#  ifdef USE_EUROPA_LOGGER
#    include "EuropaLoggerMacros.hh"
#  else
#    include "DebugMsg.hh"  //the original logging class for europa
#  endif /* USE_EUROPA_LOGGER */ 
#endif /* DEBUG_MESSAGE_SUPPORT */

#endif /* _H_Debug */
