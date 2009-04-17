// Marshals demogroup
// http://www.marshals.cz
// Copyright (c) 2003 Antonin Hildebrand. All Rights Reserved

/*! \file stl_ext.h
\brief Ours STL extensions
*/

#ifndef __STL_EXT_H__
#define __STL_EXT_H__

#include <hash_map>
#include <functional>
#include <algorithm>
#include <deque>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <stack>
#include <utility>
#include <vector>
#include <string>

// namespace for our custom STL extensions
namespace stdext
{
  //!  Template for list of poiters to dynamically allocated data, with deleting pointed objects on destruction
  template<class T>
  class plist: public std::list<T>
  {
  public:
    //! Destructor - deleting of all objects pointed by items in list
    virtual
    ~plist()
    {
      plist<T>::iterator        pos;
      pos = begin();
      while (pos != end())
        delete (*pos++);
    }

    //! Method to be used as a replacement for clear(). Deletes all objects pointed by items in list before calling clear().
    void
    purge()
    {
      plist<T>::iterator        pos;
      pos = begin();
      while (pos != end())
        delete (*pos++);
      clear();
    }
  };


  //!  Template for vector of poiters to dynamically allocated data, with deleting pointed objects on destruction
  template<class T>
  class pvector: public std::vector<T>
  {
  public:
    //! Destructor - deleting of all objects pointed by items in vector
    virtual
      ~pvector()
    {
      pvector<T>::iterator        pos;
      pos = begin();
      while (pos != end())
        delete (*pos++);
    }

    //! Method to be used as a replacement for clear(). Deletes all objects pointed by items in vector before calling clear().
    void
      purge()
    {
      pvector<T>::iterator        pos;
      pos = begin();
      while (pos != end())
        delete (*pos++);
      clear();
    }
  };




  //!  Template for hash map of poiters to dynamically allocated data, with deleting pointed objects on destruction
  template<class _Key, class _Tp, class _HashFcn, class _EqualKey>
  class phash_map: public std::hash_map<_Key, _Tp, _HashFcn, _EqualKey>
  {
  public:
    //! Destructor - deleting of all objects pointed by items in list
    virtual
    ~phash_map()
    {
      iterator          pos = begin();
      while (pos != end())
      {
        delete (pos->second);
        pos++;
      }
    }

    //! Method to be used as a replacement for clear(). Deletes all objects pointed by items in hash_table before calling clear().
    void
    purge()
    {
      iterator          pos = begin();
      while (pos != end())
      {
        delete (pos->second);
        pos++;
      }
      clear();
    }
  };
} // namespace stdext

// STL 
#define xlist std::list
#define xvector std::vector
#define xmap std::map
#define xmultimap std::multimap
#define xhash_map stdext::hash_map
#define xhash_multimap stdext::hash_multimap
#define xpair std::pair
#define xstring std::string
#define xset std::set

// STL extensions
#define xplist stdext::plist
#define xphash_map stdext::phash_map
#define xpvector stdext::pvector

#endif // __STL_EXT_H__

