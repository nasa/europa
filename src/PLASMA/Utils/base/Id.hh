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

#ifndef _H_ID
#define _H_ID

#include "CommonDefs.hh"
#include "IdTable.hh"
#include "Error.hh"
#include <typeinfo>

/**
 * @file Id.hh
 * @author Conor McGann
 * @brief Defines a template class to wrap pointers to objects.
 * @date  July, 2003
 * @see IdTable
*/

namespace EUROPA {

  class IdErr {
  public:
    DECLARE_ERROR(IdMgrInvalidItemPtrError);
  };

  /**
   * @class Id
   * @brief Provides a safe, efficient and easy to use reference object to wrap access to a pointer.
   *
   * The key capabilities are:
   * @li Safety for identifying dangling pointers through provision of an 'isValid' check on the id.
   * @li Safety for preventing multiple reference allocations for the same original pointer.
   * @li Support for references to derived types from pointers to the base through built in casting.
   * @li Easy initialization - no need for clients to use an id manager.
   * @li Support for explicit memory release or just Id cleanup as necessary through the Id.
   *
   * @par Initialization
   * There are a number of ways to initialize an Id, including the original initialization which creates the internal
   * wrapping mechanism used by all subsequent copies, as well as copying and assignment. The principal methods for
   * initialization are shown below:
   * @verbatim
Id<Foo> id1(new Foo()); // Original allocation
Id<Foo> id2(id1); // Allocation by copy.
assert(id1 == id2); // Will now be true.

Id<Foo> id3; // Default initialization
assert(id3 == Id<Foo>::noId()); // Will now be true.

id3 = id2; // Assignment
assert(id1 == id3); // Will now be true.@endverbatim
   *
   * @par Protection against duplicate wrapping of original instance
   * Original initialization takes a pointer to the object to be referenced. We must prevent more than one such wrapper
   * object from being created, so we build in protection as shown below:
   * @verbatim
Foo* foo = new Foo();
Id<Foo> id1(foo);
assert(id1 != Id<Foo>::noId()); // Will be true since it is the first one.
Id<Foo> id2(foo); // Try to do another initialization to wrap the same instance. Not Allowed in non fast version.! Will cause an error.@endverbatim
   * @par Explicit memory management & dangling pointer protection
   * For a number of reasons, one may wish to directly release the instance being accessed. A mthod is provided on an id to do this.
   * It will invalidate other ids to the same instance in a detectable manner (non fast version only).
   * @verbatim
Id<Foo> id1(new Foo());
Id<Foo> id2(id1);
assert(id1.isValid()); // True since memeory has not been released. Ref count is 2.
id2.release(); // Release the object, any Id can do this. Ref count is still 2.
assert(id1.isInvalid()); // True since we now have a dangling pointer, but at least we can check for it cheaply!@endverbatim
   * @par Casting
   * It is possible to cast an Id to its pointer, or to another pointer that represents a valid cast. For example:
   * @verbatim
// Assume class Foo exists and is base class for a class Bar.
Foo* f = new Foo();
Id<Foo> fooId(f);
Foo* fooFromCast = (Foo*) fooId; // This is fine.
assert(fooFromCast == f);
Bar* badCast = (Bar*) fooId(); // Not correct since it is not an instance of Bar.
assert(badCast == 0); // Results in a null pointer

// Now try an instance of Bar.
Bar* b = new Bar();
Id<Foo> fooId2((Foo*) b);
Bar* goodCastAsBar = (Bar*) fooId2;
Foo* goodCastAsFoo = (Foo*) fooId2;
assert(goodCastAsBar && goodCastAsFoo);

// Now show a compiler error - assume class Baz is not related to Foo
Baz* baz = (Baz*) fooId; // Will not compile.@endverbatim
   * @par Helful hints
   * @li Never use isValid for flow control. It will always return true when compiled under EUROPA_FAST. If you wish
   * to test id's, ensure there is valid data for isNoId(). For example, explicitly clear Id's if the object they are
   * referrring to is deleted. This cannot be done automatically without incurring high overhead, so it is up to the user to manage.
   * @par Implementation notes
   * Some points of interest in the implementation are:
   * @li An IdTable class is used to implement protection against duplication of wrappers for the same instance. This can also be
   * used to check for memory leaks (indicated by remaining entries in the table) and track occurence of dangling pointers (track
   * cause of removal of id prematurely).
   * @li The API is fully backward compatible with previous Id template class.
   * @li The IdManager is no longer required, but can be used for backward compatibility without any problems.
   * @li Compilation as EUROPA_FAST gets rid of all access to IdTable and thus isValid and isNotValid are not effective.
   * @see IdManager, IdTable
   */
  template<class T>
  class Id {
  public:

    /**
     * @brief Initial construction to wrap ptr.
     * @param ptr The pointer to create a new Id for.
     * @see Id::noId()
     */
    inline Id(T* ptr) {
#ifndef EUROPA_FAST
      check_error(ptr != 0, std::string("Cannot generate an Id<") + typeid(T).name() + "> for 0 pointer.",
                  IdErr::IdMgrInvalidItemPtrError());
      m_key = IdTable::insert((unsigned long int)(ptr), typeid(T).name());
      check_error(m_key != 0, std::string("Cannot generate an Id<") + typeid(T).name() + "> for a pointer that has not been cleaned up.",
                  IdErr::IdMgrInvalidItemPtrError());
#endif
      m_ptr = ptr;
    }

    /**
     * @brief Copy constructor.
     * @param org The constant reference to original Id from which to copy.
     */
    inline Id(const Id& org) {
      m_ptr = org.m_ptr;
#ifndef EUROPA_FAST
      m_key = org.m_key;
#endif
    }

    /**
     * @brief Default Constructor. Results in an object equal to noId().
     * @see Id::noId()
     */
    inline Id() {
      m_ptr = 0;
#ifndef EUROPA_FAST
      m_key = 0;
#endif
    }

    /**
     * @brief Permit type casting of doubles on construction.
     * @param val A double value encoding of the address of the instance to be pointed to.
     * Must be 0, or an address for which an Id has already been allocated.
     */
    inline Id(double val) {
#ifndef EUROPA_FAST
      if (val == 0)
        m_key = 0;
      else {
        m_key = IdTable::getKey((unsigned long int) val);
        check_error(m_key != 0,
                    std::string("Cannot instantiate an Id<") + typeid(T).name() + "> for this address. No instance present.",
                    IdErr::IdMgrInvalidItemPtrError());
      }
#endif
      m_ptr = (T*) (unsigned long int) val;
    }

    /**
     * @brief Copy constructor from an Id of a different type.
     * @param org The constant reference to original Id from which to copy.
     * @note Must be able to cast from X* to T*.
     */
    template <class X>
    inline Id(const Id<X>& org) {
      copyAndCastFromId(org);
    }

    /**
     * @brief Cast the pointer to a double.
     */
    inline operator double() const {
        return((double) (unsigned long int) m_ptr);
    }

    /**
     * @brief Assignment operator.
     * @param org The constant reference to original Id to be assigned to.
     * @return A reference to self.
     */
    inline Id& operator=(const Id& org) {
      m_ptr = org.m_ptr;
#ifndef EUROPA_FAST
      m_key = org.m_key;
#endif
      return(*this);
    }

    /**
     * @brief Equality test for mixed types. No casting used
     * @param An Id of a possibly different type.
     * @return true if the addresses match
     */
    template <class X>
    inline bool equals(const Id<X>& org) const{
      return (operator double()) == (org.operator double());
    }

    /**
     * @brief Assignment operator.
     * @param org The Id to be copied.
     * @return A reference to self.
     */
    template <class X>
    inline Id& operator=(const Id<X>& org) {
      copyAndCastFromId(org);
      return(*this);
    }

    /**
     * @brief Overload dereferencing operator to obtain the original pointer.
     * @return A pointer to the instance being referenced.
     * @see operator X*()
     */
    inline T* operator->() const {
      return(m_ptr);
    }

    /**
     * @brief Overload casting operator to obtain the original pointer in the required form.
     * @note Will cause a fatal error if it is not a safe cast.
     */
    template <class X>
    inline operator X* () const {
      return(static_cast<X*>(m_ptr));
    }

    /**
     * @brief Overload dereferencing operator to obtain the original pointer.
     * @return A reference to the instance being referenced.
     * @see operator X->()
     */
    inline T& operator*() const {
      return(*m_ptr);
    }

    /**
     * @brief Provide a way to test if an Id<X> can be used as if it was of this type.
     */
    template <class X>
    static bool convertable(const Id<X>& id) {
      T* ptr = dynamic_cast<T*>(id.operator->());
      return(ptr != 0);
    }

    /**
     * @brief Handy static method to check for the "empty" id.
     * @return A reference to an Id created as Id(). This is a static member of the class.
     */
    static inline const Id& noId() {
      static const Id s_noId;
      return(s_noId);
    }

    /**
     * @brief Handy method to directly test for noId without requiring comparison with another object.
     */
    inline bool isNoId() const {
#ifndef EUROPA_FAST
      return(m_ptr == 0 && m_key == 0);
#else
      return(m_ptr == 0);
#endif
    }

    /**
     * @brief Handy method to directly test if an Id without requiring comparison with another object.
     * @see isNoId()
     */
    inline bool isId() const {
#ifndef EUROPA_FAST
      return(m_ptr != 0 && m_key != 0);
#else
      return(m_ptr != 0);
#endif
    }

    /**
     * @brief Use this to check if the Id is a "dangling pointer" or a noId.
     * The fast version only tests for the pointer not being 0 so it is inappropriate to use this function to
     * do flow control in your program. It should only be used as an assertion for safety.
     * @return true if the pointer is non zero and the instance to which it points has not been deallocated. False otherwise.
     * @see Id::isInvalid(), Id::isNoId()
     */
    inline bool isValid() const {
#ifndef EUROPA_FAST
      return(m_ptr != 0 && m_key != 0 && IdTable::getKey((unsigned long int)m_ptr) == m_key);
#else
      return(m_ptr != 0);
#endif
    }

    /**
     * @brief !isValid()
     * @see Id::isValid()
     */
    inline bool isInvalid() const {
#ifndef EUROPA_FAST
      return(!isValid());
#else
      return(m_ptr == 0);
#endif
    }

    /**
     * @brief Equality operator.
     * @param comp The Id to be compared against.
     * @return true if the Id's point to the same pointer, and have the same key (non fast version only).
     * @see Id::isValid()
     */
    inline bool operator==(const Id& comp) const {
#ifndef EUROPA_FAST
      return(m_ptr == comp.m_ptr && m_key == comp.m_key);
#else
      return(m_ptr == comp.m_ptr);
#endif
    }

    /**
     * @brief Inequality operator.
     * @param comp The Id to be compared against.
     * @return false if ==, true otherwise.
     */
    inline bool operator!=(const Id& comp) const {
#ifndef EUROPA_FAST
      return (!(operator==(comp)));
#else
      return(m_ptr != comp.m_ptr);
#endif
    }

    /**
     * @brief Arbitrary ordering based on pointer values.
     * @param comp The Id to be compared against.
     * @return true if the address of this object is greater than comp.
     */
    inline bool operator>(const Id& comp) const {
      return(m_ptr > comp.m_ptr);
    }

    /**
     * @brief Arbitrary ordering based on pointer values.
     * @param comp The Id to be compared against.
     * @return true if the address of this object is less than comp.
     */
    inline bool operator<(const Id& comp) const {
      return(m_ptr < comp.m_ptr);
    }

    /**
     * @brief Print the Id to the stream.
     * @param os The output stream to use.
     */
    inline void print(std::ostream& os) const {
      if (isNoId())
        os << "noId";
      else
#ifndef EUROPA_FAST
        os << "id_" << m_key;
#else
      os << "ptr_" << m_ptr;
#endif
    }

    /**
     * @brief Deallocate the referenced object if it has not already been deallocated.
     * @note Will cause an error if the Id is not valid.
     */
    inline void release() {
      // Take local copy of pointer to delete since we will want to null m_ptr prior to deletion
      // as a safety measure for objects which embed id's and deallocate them on destruction.
      T* ptr = m_ptr;
#ifndef EUROPA_FAST
      check_error(isValid(), std::string("Cannot release an invalid Id<") + typeid(T).name() + ">.",
                  IdErr::IdMgrInvalidItemPtrError());
      m_key = 0;
      IdTable::remove((unsigned long int) ptr);
#endif
      m_ptr = 0;
      delete ptr;
    }

    /**
     * @brief Clear the IdTable entry for this pointer.
     * @note Will cause an error if the Id is not valid.
     * @see IdTable::release()
     */
    inline void remove() {
#ifndef EUROPA_FAST
      check_error(isValid(), std::string("Cannot remove an invalid Id<") + typeid(T).name() + ">.",
                  IdErr::IdMgrInvalidItemPtrError());
      IdTable::remove((unsigned long int) m_ptr);
      m_key = 0;
#endif
      m_ptr = 0;
    }

  private:

    template <class X>
    inline void copyAndCastFromId(const Id<X>& org) {
      m_ptr = dynamic_cast<T*>(org.operator->());
#ifndef EUROPA_FAST
      if (org.isNoId()) {
        m_key = 0;
        return;
      }
      check_error(Id<T>::convertable(org), std::string("Invalid cast from Id<") + typeid(X).name() + "> to Id<" + typeid(T).name() + ">.",
                  IdErr::IdMgrInvalidItemPtrError());
      m_key = IdTable::getKey((unsigned long int) m_ptr);
      check_error(m_key != 0, std::string("Cannot create an Id<") + typeid(X).name() + "> for this address since no instance is present.",
                  IdErr::IdMgrInvalidItemPtrError());
#endif
    }

    /**
     * Actual pointer to the data.
     */
    T* m_ptr;

#ifndef EUROPA_FAST
    /**
     * Key within the IdTable.
     */
    unsigned int m_key;
#endif
  };

  template<class T>
  std::ostream& operator<<(std::ostream& outStream, const Id<T>& id) {
    id.print(outStream);
    return(outStream);
  }

} // End namespace

#endif
