#ifndef _H_Utils
#define _H_Utils

/**
 * @file   Utils.hh
 * @author Conor McGann
 * @date   Mon Dec 27 17:19:39 2004
 * @brief  
 * @ingroup Utils
 */

#include "Id.hh"
#include "Entity.hh"

#include <vector>
#include <set>
#include <list>
#include <string>
#include <cmath>
#include <limits>

namespace EUROPA {

  /**
   * Utility class that might get promoted later.
   */
  class Infinity {
  public:
    static double plus(double n1, double n2, double defaultValue) {
      if (fabs(n1) >= PLUS_INFINITY || fabs(n2) >= PLUS_INFINITY)
	return(defaultValue);
      double retval = n1 + n2;
      if(fabs(retval) >= PLUS_INFINITY)
	return defaultValue;
      return retval;
    }

    static double minus(double n1, double n2, double defaultValue) {
      if (fabs(n1) >= PLUS_INFINITY || fabs(n2) >= PLUS_INFINITY)
	return(defaultValue);
      double retval = n1 - n2;
      if(fabs(retval) >= PLUS_INFINITY)
	return defaultValue;
      return retval;
    }
  };

  /**
   * @brief Utility to produce a string from a double
   */
  std::string toString(double value);

  /**
   * @brief Utility function to tokenzie a std string 
   */

  void tokenize(const std::string& str, 
		std::vector<std::string>& tokens,  
		const std::string& delimiters = " "); 

  template<class TYPE>
  bool allValid(const std::set<Id<TYPE> >& objects){
    typedef typename std::set<Id<TYPE> >::const_iterator object_iterator;
    for(object_iterator it = objects.begin(); it != objects.end(); ++it){
      Id<TYPE> id = *it;
      if(id.isNoId() || id.isInvalid())
        return false;
    }
    return true;
  }

  template<class TYPE1, class TYPE>
  bool allValid(const std::map<TYPE1, Id<TYPE> >& objects){
    typedef typename std::map<TYPE1, Id<TYPE> >::const_iterator object_iterator;
    for(object_iterator it = objects.begin(); it != objects.end(); ++it){
      Id<TYPE> id = it->second;
      if(id.isNoId() || id.isInvalid())
        return false;
    }
    return true;
  }

  template<class TYPE>
  bool allValid(const std::set<Id<TYPE>, EntityComparator<Id<Entity> > >& objects){
    typedef typename std::set<Id<TYPE> >::const_iterator object_iterator;
    for(object_iterator it = objects.begin(); it != objects.end(); ++it){
      Id<TYPE> id = *it;
      if(id.isNoId() || id.isInvalid())
        return false;
    }
    return true;
  }

  template<class TYPE>
  void cleanup(std::set<Id<TYPE> >& objects){
    typedef typename std::set<Id<TYPE> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      check_error((*it).isValid());
      delete (TYPE*) (*it++);
    }
    objects.clear();
  }

  template<class TYPE>
  void cleanup(std::set<Id<TYPE>, EntityComparator<Id<TYPE> > >& objects){
    typedef typename std::set<Id<TYPE>, EntityComparator<Id<TYPE> > >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      check_error((*it).isValid());
      delete (TYPE*) (*it++);
    }
    objects.clear();
  }

  template<class TYPE>
  void cleanup(std::set<Id<TYPE>, EntityComparator<Id<Entity> > >& objects){
    typedef typename std::set<Id<TYPE>, EntityComparator<Id<Entity> > >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      check_error((*it).isValid());
      delete (TYPE*) (*it++);
    }
    objects.clear();
  }

  template<class TYPE>
  void cleanup(std::vector<Id<TYPE> >& objects){
    typedef typename std::vector<Id<TYPE> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      checkError((*it).isValid(), *it);
      delete (TYPE*) (*it++);
    }
    objects.clear();
  }


  template<class TYPE>
  void discardAll(std::vector<Id<TYPE> >& objects){
    typedef typename std::vector<Id<TYPE> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      checkError((*it).isValid(), *it);
      Id<TYPE> elem = *it;
      elem->discard();
      ++it;
    }
    objects.clear();
  }

  template<class TYPE>
  void discardAll(std::list<Id<TYPE> >& objects){
    typedef typename std::list<Id<TYPE> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      Id<TYPE> object = *it;
      if(!object.isNoId()){
	check_error(object.isValid());
	object->discard();
      }
      ++it;
    }
    objects.clear();
  }

  template<class TYPE>
  void cleanup(std::list<Id<TYPE> >& objects){
    typedef typename std::list<Id<TYPE> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      Id<TYPE> object = *it;
      if(!object.isNoId()){
	check_error(object.isValid());
	delete (TYPE*) (*it++);
      }
      else
	++it;
    }
    objects.clear();
  }

  template<class TYPE1, class TYPE2>
  void cleanup(std::map<TYPE1, Id<TYPE2> >& objects){
    typedef typename std::map<TYPE1, Id<TYPE2> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      Id<TYPE2> item = (it++)->second;
      check_error(item.isValid());
      delete (TYPE2*) item;
    }
    objects.clear();
  }

  template<class TYPE1, class TYPE2>
  void cleanup(std::multimap<TYPE1, Id<TYPE2> >& objects){
    typedef typename std::multimap<TYPE1, Id<TYPE2> >::const_iterator object_iterator;
    object_iterator it = objects.begin();
    while(it != objects.end()){
      Id<TYPE2> item = (it++)->second;
      check_error(item.isValid());
      delete (TYPE2*) item;
    }
    objects.clear();
  }

  template<class TYPE>
  void cleanup(std::list<TYPE*>& objects){
    typedef typename std::list<TYPE*>::const_iterator object_iterator;
    for(object_iterator it = objects.begin(); it != objects.end(); ++it){
      TYPE* element = *it;
      delete element;
    }
    objects.clear();
  }

}
#endif
