#ifndef _H_Domain
#define _H_Domain

#include "../ConstraintEngine/EnumeratedDomain.hh"

/**
 * @file Domain.hh
 * @brief Defines a template class for an enumerated domain of Id's
 */
namespace Prototype {

  template<class ELEMENT_TYPE>
  class Domain: public EnumeratedDomain {
  public:
    /**
     * @brief Constructor
     * @param labels The initial set of labels to populate the domain.
     * @param closed Indicate if the set is initially closed.
     * @param listener Allows connection of a listener to change events on the domain. 
     * @see AbstractDomain::isDynamic()
     */
    Domain(const std::list< ELEMENT_TYPE >& values, 
	     bool closed = true,
	     const DomainListenerId& listener = DomainListenerId::noId());

    /**
     * @brief Copy constructor
     * @param org The source domain.
     */
    Domain(const Domain& org);

    /**
     * @brief Empty domain default constructor
     */
    Domain();

    /**
     * @brief NO-OP destructor
     */
    ~Domain();

    /**
     * @brief Fill the given list with the contents of the set.
     * @param results The list to be filled. Must be empty.
     * @see EnumeratedDomain::getValues()
     */
    void getValues(std::list< ELEMENT_TYPE >& results) const;

    /**
     * @brief Obtain the singleton value for this domain. It must be a singleton.
     */
    ELEMENT_TYPE getValue() const;
  };

  /**
   * Utility method
   */
  template <class ELEMENT_TYPE>
  std::list<double> convert(const std::list<ELEMENT_TYPE>& source){
    std::list<double> target;
    for(std::list<ELEMENT_TYPE>::const_iterator it = source.begin(); it != source.end(); ++it)
      target.push_back((double) *it);
    return(target);
  }

  template <class ELEMENT_TYPE>
  Domain<ELEMENT_TYPE>::Domain(const std::list<ELEMENT_TYPE>& labels, bool closed, const DomainListenerId& listener)
    : EnumeratedDomain(convert(labels), closed, listener){}

  template <class ELEMENT_TYPE>
  Domain<ELEMENT_TYPE>::Domain(const Domain& org)
    : EnumeratedDomain(static_cast<const EnumeratedDomain&>(org)){}

  template <class ELEMENT_TYPE>
  Domain<ELEMENT_TYPE>::Domain()
    : EnumeratedDomain(){}

  template <class ELEMENT_TYPE>
  Domain<ELEMENT_TYPE>::~Domain(){}

  template <class ELEMENT_TYPE>
  void Domain<ELEMENT_TYPE>::getValues(std::list<ELEMENT_TYPE>& results) const{
    check_error(results.empty());
    check_error(isFinite());
    for(std::set<double>::iterator it = m_values.begin(); it != m_values.end(); ++it){
      double value = *it;;
      results.push_back(ELEMENT_TYPE(value));
    }
  }

  template <class ELEMENT_TYPE>
  ELEMENT_TYPE Domain<ELEMENT_TYPE>::getValue() const {return EnumeratedDomain::getValue();}

  class LabelStr;
  typedef Domain<LabelStr> LabelSet;
}

#endif
