#ifndef _H_PSUtils
#define _H_PSUtils

#include "Entity.hh"

namespace EUROPA {

  typedef int PSEntityKey;
  
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

  class PSEntity
  {
    public: 
      PSEntity();
      PSEntity(const EntityId& entity);
      virtual ~PSEntity() {}
    
      virtual PSEntityKey getEntityKey() const;
      virtual const std::string& getEntityName() const;
      virtual const std::string& getEntityType() const;

      virtual std::string toString() const;

    protected:
      EntityId m_entity;    

      virtual const EntityId getEntity() const { return m_entity; }
  };	      
}

#endif

