#ifndef NUMBERHASH_H_
#define NUMBERHASH_H_

#include "Number.hh"
#include <boost/unordered_map.hpp>

namespace boost {

template<> struct hash<EUROPA::edouble> {
  inline size_t operator()(EUROPA::edouble __x) const;
};

template<> struct hash<EUROPA::eint> {
  inline size_t operator()(EUROPA::eint __x) const;
};
}

namespace boost {
  //I'm not entirely sure this is safe, but it's worked so far.  Maybe this should be changed to
//*((size_t*)&(__x.m_v))
size_t hash<EUROPA::edouble>::operator()(EUROPA::edouble __x) const {return reinterpret_cast<size_t>(__x.m_v);}
size_t hash<EUROPA::eint>::operator()(EUROPA::eint __x) const {return reinterpret_cast<size_t>(static_cast<long>(__x.m_v));}
}


#endif /* NUMBERHASH_H_ */
