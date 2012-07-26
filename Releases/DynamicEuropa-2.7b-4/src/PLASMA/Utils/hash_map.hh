#ifndef _H_hash_map
#define _H_hash_map

/**
 * @brief Utility header to move hash map include stuff to one place, since that changes between GCC versions.
 * 
 */

#ifdef _MSC_VER
#  include <hash_map>

#else
#  include <ext/hash_map>

// Come on you GCC guys...
#  if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
#    if (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#      include <backward/hash_fun.h>
#    else
#      include <ext/hash_fun.h>
#    endif
#  else
#    include <ext/stl_hash_fun.h>
#  endif

#endif //_MSC_VER


#endif /* _H_hash_map */
