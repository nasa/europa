#ifndef _H_PSUtils
#define _H_PSUtils

#include <vector>
#include <algorithm>
#include "Id.hh"
#include "LabelStr.hh"

namespace EUROPA {

  template<class T>
  class PSList
  {
    public:
      int size() const { return m_elements.size(); }
      T& get(int idx) { return m_elements[idx]; }
      void remove(int idx) {m_elements.erase(std::advance(m_elements.begin(), idx));}
      void remove(const T& value) 
      {
        typename std::vector<T>::iterator it =
        std::find(m_elements.begin(), m_elements.end(), value);
        if(it != m_elements.end())
	    m_elements.erase(it);
      }
      void push_back(const T& value) {m_elements.push_back(value);}
    
    protected:
      std::vector<T> m_elements;    	
  };

  typedef int PSEntityKey;
 
  class PSEntity
  {
    public: 
      virtual ~PSEntity() {}
    
      virtual PSEntityKey getKey() const = 0;
      virtual const std::string& getEntityName() const = 0;        
      virtual const std::string& getEntityType() const = 0;
      virtual std::string toString() const
      {
     	  std::stringstream sstr;
     	  sstr << getEntityName() << "(" << getKey() << ")";
     	  return sstr.str();
       }
  };	      

}

#endif

