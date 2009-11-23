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

#include "IdTable.hh"
#include "CommonDefs.hh"
#include "Debug.hh"
#include "LabelStr.hh"
#include "Mutex.hh"
#include "Entity.hh"

/**
 * @file IdTable.cc
 * @author Conor McGann
 * @brief Implements IdTable
 * @par Implementation notes
 * @li If system is compiled with EUROPA_FAST then this class is not used.
 * @li Use the size method as a check to ensure memory is deallocated correctly. On destruction, size should be 0.
 * @li Use the output function to display pointer address and key pairs that have not been deallocated.
 * @li Use debug messages this information in conjunction with the output.
 * @li A dangling pointer failure can be traced by looking for the removal event for a given <pointer, key> pair.
 * @date  July, 2003
 * @see Id<T>
 */

namespace EUROPA {

  unsigned int getEntryKey(std::pair<unsigned int,double>& entry)
  {
    return entry.first;
  }

  LabelStr getEntryType(std::pair<unsigned int,double>& entry)
  {
    return entry.second;
  }

  pthread_mutex_t& IdTableMutex()
  {
      static pthread_mutex_t sl_mutex = PTHREAD_MUTEX_INITIALIZER;
      return sl_mutex;
  }

  IdTable::IdTable() {
  }

  IdTable::~IdTable() {
  }

  IdTable& IdTable::getInstance() {
    static IdTable sl_instance;
    return(sl_instance);
  }
  unsigned int IdTable::size() {
    MutexGrabber mg(IdTableMutex());
    return(getInstance().m_collection.size());
  }

  bool IdTable::allocated(unsigned long int id) {
    MutexGrabber mg(IdTableMutex());
    return(getInstance().m_collection.find(id) != getInstance().m_collection.end());
  }

  unsigned int IdTable::getKey(unsigned long int id) {
    MutexGrabber mg(IdTableMutex());
    std::map<unsigned long int, std::pair<unsigned int,double> >::iterator it = getInstance().m_collection.find(id);
    if (it != getInstance().m_collection.end())
      return getEntryKey(it->second);
    else
      return(0);
  }

  unsigned int IdTable::insert(unsigned long int id, const char* baseType) {
    MutexGrabber mg(IdTableMutex());
    static unsigned int sl_nextId(1);
    debugMsg("IdTable:insert", "id,key:" << id << ", " << sl_nextId << ", " << baseType << ")");
    if (sl_nextId == 101)
        debugMsg("IdTable:insert","Found 101");

    std::map<unsigned long int, std::pair<unsigned int,double> >::iterator it =
      getInstance().m_collection.find(id);

    if (it != getInstance().m_collection.end())
      return(0); /* Already in table. */

    getInstance().m_collection.insert(std::make_pair(id, std::make_pair(sl_nextId,LabelStr(baseType))));

    std::map<std::string, unsigned int>::iterator tCit = getInstance().m_typeCnts.find(baseType);
    if (tCit == getInstance().m_typeCnts.end())
      getInstance().m_typeCnts.insert(std::pair<std::string, unsigned int>(baseType, 1));
    else
      tCit->second++;

    return(sl_nextId++);
  }

  void IdTable::remove(unsigned long int id) {
    MutexGrabber mg(IdTableMutex());
    static unsigned int sl_key;
    std::map<unsigned long int, std::pair<unsigned int,double> >::iterator it = getInstance().m_collection.find(id);
    sl_key = getEntryKey(it->second);
    std::string type = getEntryType(it->second).toString();

    debugMsg("IdTable:remove", "<" << id << ", " << sl_key << "," << type << ">");
    std::map<std::string, unsigned int>::iterator tCit = getInstance().m_typeCnts.find(type);
    tCit->second--;

    getInstance().m_collection.erase(id);
  }

  void IdTable::printTypeCnts(std::ostream& os) {
    MutexGrabber mg(IdTableMutex());
    os << "Id instances by type:\n";
    for (std::map<std::string, unsigned int>::iterator it = getInstance().m_typeCnts.begin();
         it != getInstance().m_typeCnts.end();
         ++it)
      os << "  " << it->second << "  " << it->first << '\n';
    os << std::endl;
  }

  void IdTable::output(std::ostream& os) {
    printTypeCnts(os);
    MutexGrabber mg(IdTableMutex());
    os << "Id Contents:";
    for (std::map<unsigned long int, std::pair<unsigned int,double> >::iterator it = getInstance().m_collection.begin();
         it != getInstance().m_collection.end();
         ++it)
      os << " (" << it->first << ", " << getEntryKey(it->second) << "," << getEntryType(it->second).toString() << ')';
    os << std::endl;
  }

  void IdTable::checkResult(bool result, unsigned int id_count)
  {
	  Entity::garbageCollect();

	  if (result && IdTable::size() <= id_count) {
		  debugMsg("Test"," PASSED.");
	  }
	  else {
		  if (result) {
			  std::cerr << " FAILED = DID NOT CLEAN UP ALLOCATED IDs:\n";
			  IdTable::output(std::cerr);
			  std::cerr << "\tWere " << id_count << " IDs before; " << IdTable::size() << " now";
			  std::cerr << std::endl;
			  throw Error::GeneralMemoryError();
		  }
		  else {
			  std::cerr << "      " << " FAILED TO PASS UNIT TEST." << std::endl;
			  throw Error::GeneralUnknownError();
		  }
	  }
  }

}
