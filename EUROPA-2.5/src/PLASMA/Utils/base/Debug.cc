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

#include "Debug.hh"

#ifndef ALL_LOGGING_DISABLED
#  ifdef USE_EUROPA_LOGGER
#    include "EuropaLogger.cc"
#  else
#    include "DebugMsg.cc"  //the original logging class for europa
#  endif /* USE_EUROPA_LOGGER */ 
#endif /* DEBUG_MESSAGE_SUPPORT */
