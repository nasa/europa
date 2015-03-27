#ifndef H_PSUtils
#define H_PSUtils

#include <vector>
#include <algorithm>

namespace EUROPA {

template<class T>
class PSList
{
 public:
  PSList() : m_elements() {}
  long size() const { return static_cast<long>(m_elements.size()); }
  T& get(long idx) { return m_elements[static_cast<unsigned>(idx)]; }
  const T& get(long idx) const { return m_elements[static_cast<unsigned>(idx)]; }
  void remove(long idx) {m_elements.erase(std::advance(m_elements.begin(),
                                                       static_cast<unsigned>(idx)));}
  void remove(const T& value)
  {
    typename std::vector<T>::iterator it =
        std::find(m_elements.begin(), m_elements.end(), value);
    if(it != m_elements.end())
      m_elements.erase(it);
  }
  void push_back(const T& value) {m_elements.push_back(value);}
  void clear() {m_elements.clear();}
  bool operator==(const PSList<T>& other) const {
    return m_elements == other.m_elements;
  }
 protected:
  std::vector<T> m_elements;
};

template <typename T>
class PSList<T*> {
 public:
  PSList() : m_elements() {}
  long size() const { return static_cast<long>(m_elements.size()); }
  T* get(long idx) { return m_elements[static_cast<unsigned>(idx)]; }
  const T* get(long idx) const { return m_elements[static_cast<unsigned>(idx)]; }
  void remove(long idx) {m_elements.erase(std::advance(m_elements.begin(),
                                                       static_cast<unsigned>(idx)));}
  void remove(T* const value)
  {
    typename std::vector<T*>::iterator it =
        std::find(m_elements.begin(), m_elements.end(), value);
    if(it != m_elements.end())
      m_elements.erase(it);
  }
  void push_back(T* const value) {m_elements.push_back(value);}
  void clear() {m_elements.clear();}
  bool operator==(const PSList<T>& other) const {
    return m_elements == other.m_elements;
  }
 protected:
  std::vector<T*> m_elements;
};

}

#endif

