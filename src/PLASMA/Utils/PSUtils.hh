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

  // TODO:  Use PSEntityKey everywhere, or just int
  typedef int PSEntityKey;
 
  class PSEntity
  {
    public: 
      PSEntity();
      virtual ~PSEntity() {}
    
      inline PSEntityKey getKey() const {return m_key;}

      // TODO:  Do we need two get name methods?
      virtual const std::string& getEntityName() const;
      virtual const LabelStr& getName() const;
            
      virtual const std::string& getEntityType() const;

      virtual std::string toString() const;

      static int allocateKey();

    protected:
    	const int m_key;
  };	      

}

#endif

