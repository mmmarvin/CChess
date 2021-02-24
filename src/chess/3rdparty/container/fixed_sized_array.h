/**********
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 *
 **********/
#ifndef GUARD_MAR_container_fixed_sized_array_H
#define GUARD_MAR_container_fixed_sized_array_H

// standard headers
#include <memory>
#include <type_traits>

#ifndef MAR_NULL
#if __cplusplus >= 201103L
#define MAR_NULL nullptr
#else
#define MAR_NULL 0
#endif // C++11
#endif // MAR_NULL

#ifndef MAR_NO_EXCEPT
#if __cplusplus >= 201103L
#define MAR_NO_EXCEPT noexcept
#else
#define MAR_NO_EXCEPT
#endif // C++11
#endif // MAR_NO_EXCEPT

namespace mar
{
namespace container
{
    template<class T,
             std::size_t Size,
             class AllocatorT = std::allocator<T>>
    class fixed_sized_array : AllocatorT
    {
    public:
        typedef AllocatorT                                  allocator_type;
        typedef std::allocator_traits<AllocatorT>           allocator_trait;
        typedef typename allocator_trait::value_type        value_type;
        typedef typename allocator_trait::pointer           pointer;
        typedef typename allocator_trait::const_pointer     const_pointer;
        typedef typename allocator_type::reference          reference;
        typedef typename allocator_type::const_reference    const_reference;
        typedef typename allocator_trait::size_type         size_type;
        typedef typename allocator_trait::pointer           iterator;
        typedef typename allocator_trait::const_pointer     const_iterator;
        typedef std::reverse_iterator<iterator>             reverse_iterator;
        typedef std::reverse_iterator<const_iterator>       const_reverse_iterator;

        template<class = std::enable_if_t<std::is_default_constructible<T>::value>> fixed_sized_array(const allocator_type& alloc = allocator_type());
        fixed_sized_array(const fixed_sized_array& rhs);
        fixed_sized_array(fixed_sized_array&& rhs);
        fixed_sized_array& operator=(const fixed_sized_array& rhs);
        fixed_sized_array& operator=(fixed_sized_array&& rhs);
        fixed_sized_array(std::initializer_list<T> il, const allocator_type& alloc = allocator_type());
        fixed_sized_array(const value_type& v, const allocator_type& alloc = allocator_type());
        ~fixed_sized_array();

        iterator begin() MAR_NO_EXCEPT { return m_data; }
        const_iterator begin() const MAR_NO_EXCEPT  { return m_data; }
        reverse_iterator rbegin() MAR_NO_EXCEPT { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const MAR_NO_EXCEPT { return reverse_iterator(end()); }
        iterator end() MAR_NO_EXCEPT { return m_data + Size; }
        const_iterator end() const MAR_NO_EXCEPT { return m_data + Size; }
        reverse_iterator rend() MAR_NO_EXCEPT { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const MAR_NO_EXCEPT { return reverse_iterator(begin()); }

        reference operator[](size_type n) MAR_NO_EXCEPT { return m_data[n]; }
        const_reference operator[](size_type n) const MAR_NO_EXCEPT { return m_data[n]; }

        void swap(fixed_sized_array& other) MAR_NO_EXCEPT;

        pointer data() MAR_NO_EXCEPT { return m_data; }
        const_pointer data() const MAR_NO_EXCEPT { return m_data; }
        constexpr size_type capacity() const MAR_NO_EXCEPT { return Size; }

    private:
        void swap_data(fixed_sized_array& other) MAR_NO_EXCEPT;

        allocator_type& allocator() MAR_NO_EXCEPT { return *this; }
        const allocator_type& allocator() const MAR_NO_EXCEPT { return *this; }

        pointer m_data;
    };
}
}

// definitions
#include "fixed_sized_array.inl"

#endif // GUARD_MAR_container_fixed_sized_array_H
