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

#ifndef _H_IdTable
#define _H_IdTable

#include <map>
#include <iostream>
#include <string>

/**
 * @file IdTable.hh
 * @author Conor McGann
 * @brief Defines a singleton class managing allocation and deallocation of ids for pointers.
 * @date  July, 2003
 * @see Id, IdManager
*/

namespace EUROPA {

  /**
   * @class IdTable
   * @brief Provides a singleton which manages <pointer,key> pairs.
   *
   * Main data structure is a map of integer pairs. The map is accessed
   * by an integer which should be the address of an object managed by an Id. A key is used to
   * check for allocations of an Id to a previously allocated address. This is necessary so that dangling
   * Ids can be detected even if the address has been recycled.
   * @see Id
   */
  class IdTable {
  public:
    static unsigned int size();
    static std::map<unsigned long int, unsigned int> getCollection();
    static IdTable& getInstance();

    /**
     * Print the number of times each type has been allocated.
     */
    static void printTypeCnts(std::ostream& os);

    static void output(std::ostream& os);
    static unsigned int insert(unsigned long int id, const char* baseType);
    static bool allocated(unsigned long int id);
    static unsigned int getKey(unsigned long int id);
    static void remove(unsigned long int id);
    ~IdTable(); // deallocating statics requires public access on beos
  private:
    IdTable();
    std::map<unsigned long int, unsigned int> m_collection;  /*<! Map from pointers to keys*/
    std::map<std::string, unsigned int> m_typeCnts;
  };
}

#endif
