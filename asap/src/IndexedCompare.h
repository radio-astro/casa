/// @file
/// 
/// @brief A binary predicate to compare two numbers referred by indices 
/// @details While sorting a vector, it is often necessary to track
/// permutations. One way of doing this is to write a std::pair-like class
/// with comparison operators using one element of the pair only and store
/// both value and its index. This is done in the PairOrderedFirst class 
/// in AIPS++, which I wrote some time ago. However, there exists a more elegant
/// solution using a version of std::sort with a user-suppled binary predicate.
/// This file defines such binary predicate class comparing two values
/// stored in a container defined by its random access iterator of the origin
/// each time it is asked to compare two indicies. Each instance of the class
/// holds a copy of the rangom access iterator.
///
/// @copyright (c) 2007 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file was originally written as a part of the ASKAP software 
/// distribution and then adapted and reused in the ASAP project.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef INDEXED_COMPARE_H
#define INDEXED_COMPARE_H

#include <functional>

namespace asap {

/// @brief A binary predicate to compare two numbers referred by indices 
/// @details While sorting a vector, it is often necessary to track
/// permutations. One way of doing this is to write a std::pair-like class
/// with comparison operators using one element of the pair only and store
/// both value and its index. This is done in the PairOrderedFirst class 
/// in AIPS++, which I wrote some time ago. However, there exists a more elegant
/// solution using a version of std::sort with a user-suppled binary predicate.
/// This is such binary predicate class comparing two values
/// stored in a container defined by its random access iterator of the origin
/// each time it is asked to compare two indicies. Each instance of the class
/// holds a copy of the rangom access iterator.
template<typename IndexType, typename Iter, 
         typename Cmp = std::less<typename Iter::value_type> >
struct IndexedCompare : public std::binary_function<IndexType,IndexType,bool> {
   /// @brief constructor with a default comparator initialization
   /// @details
   /// A comparator (which type is a template parameter) is set up with its
   /// default constructor.
   /// @param[in] iter random access iterator to work with
   IndexedCompare(const Iter &iter) : itsIter(iter) {}
   
   /// @brief constructor with a user-specified comparator initialization
   /// @details
   /// A comparator (which type is a template parameter) is set up using a
   /// a copy constructor
   /// @param[in] iter random access iterator to work with
   IndexedCompare(const Iter &iter, const Cmp &cmp) : itsIter(iter),
                  itsComparator(cmp) {}
   
   
   /// @brief main operator of the predicate
   /// @details Returns result of comparison of the value referred to by the 
   /// first and the second indices
   /// @param[in] index1 index of the first value
   /// @param[in] index2 index of the second value
   /// @return result of comparison
   bool operator()(const IndexType &index1, const IndexType &index2) const 
   {
      return itsComparator(*(itsIter+index1), *(itsIter+index2)); 
   }
   
   
private:
   /// random access iterator to work with
   Iter itsIter;
   /// underlying binary predicate to do the comparison
   Cmp itsComparator;
};

/// @brief helper function to construct IndexedCompare object
/// @details It is handy to have a helper method to avoid 
/// writing type names all the time. This function can extract the 
/// template parameter from the argument type, i.e. automatically
/// @param[in] iter random access iterator to work with
/// @param[in] cmp an object which does the actual comparison 
template<typename IndexType, typename Iter, typename Cmp>
IndexedCompare<IndexType,Iter,Cmp> indexedCompare(const Iter &iter, const Cmp &cmp = 
                            std::less<typename Iter::value_type>()) 
{
   return IndexedCompare<IndexType,Iter,Cmp>(iter,cmp);
}

template<typename IndexType, typename Iter>
IndexedCompare<IndexType,Iter> indexedCompare(const Iter &iter) 
{
   return IndexedCompare<IndexType,Iter>(iter);
}

} // namespace asap

#endif // #define INDEXED_COMPARE_H

