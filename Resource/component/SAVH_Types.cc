//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software

#include "SAVH_Types.hh"

namespace EUROPA 
{
  namespace SAVH 
  {
    std::ostream& operator<<( std::ostream& os, const EdgeIdentity& fei ) 
    {
      os << "[" << fei.first << " -> " << fei.second << "]";

      return os;
    }

  }
}
