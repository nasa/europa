#ifndef _H_StringDomain
#define _H_StringDomain

/**
 * @file StringDomain.hh
 * @author Andrew Bachmann
 * @brief Declares an enumerated domain of Strings
 */
#include "EnumeratedDomain.hh"
#include "LabelStr.hh"

namespace Prototype {

  /**
   * @class StringDomain
   * @brief an enumerated domain of strings
   */
  class StringDomain : public EnumeratedDomain {
  public:

    /**
     * @brief Constructs an initially empty and open domain
     */
    StringDomain();

    /**
     * @brief Constructor.
     * @param values The initial set of values to populate the domain.
     * @param closed Indicate if the set is initially closed.
     * @param isNumeric Indicate if the set is to be used to store numeric or symbolic values
     * @param listener Allows connection of a listener to change events on the domain. 
     * @see AbstractDomain::isDynamic()
     */
    StringDomain(const std::list<double>& values, 
                 bool closed = true,
                 const DomainListenerId& listener = DomainListenerId::noId());

    /**
     * @brief Constructor.
     * @param value Constructs a singleton domain. Closed on construction.
     * @param isNumeric Indicate if the set is to be used to store numeric or symbolic values
     * @param listener Allows connection of a listener to change events on the domain. 
     */
    StringDomain(double value,
                 const DomainListenerId& listener = DomainListenerId::noId());

    /**
     * @brief Copy constructor.
     * @param org The source domain.
     */
    StringDomain(const StringDomain& org);

    /**
     * @brief Get the type of the domain to aid in type checking.
     * @see AbstractDomain::DomainType
     */
    const DomainType& getType() const;

    /**
     * @brief Get the name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    const LabelStr& getTypeName() const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual StringDomain *copy() const;

  };

} // namespace Prototype

#endif // _H_StringDomain

