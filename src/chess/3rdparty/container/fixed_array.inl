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
// custom headers
#include "../debug/assert.h"
#include "fixed_array.h"
#include "temporary_array.h"

namespace mar
{
namespace container
{
    template<class T, class AllocatorT>
    fixed_array<T, AllocatorT>::fixed_array(const fixed_array& rhs) :
        fixed_array<T, AllocatorT>(rhs.m_data, rhs.m_data + rhs.m_capacity, rhs)
    {
    }

    template<class T, class AllocatorT>
    fixed_array<T, AllocatorT>::fixed_array(fixed_array&& rhs) MAR_NO_EXCEPT(std::is_nothrow_move_constructible<T>()) :
        allocator_type(std::move(rhs.allocator())),
        m_data(MAR_NULL),
        m_capacity(0)
    {
        swap_data(rhs);
    }

    template<class T, class AllocatorT>
    fixed_array<T, AllocatorT>& fixed_array<T, AllocatorT>::operator=(const fixed_array& rhs)
    {
        fixed_array<T, AllocatorT> temp(rhs);
        swap(temp);

        return *this;
    }

    template<class T, class AllocatorT>
    fixed_array<T, AllocatorT>& fixed_array<T, AllocatorT>::operator=(fixed_array&& rhs) MAR_NO_EXCEPT(std::is_nothrow_move_assignable<T>())
    {
        swap(rhs);

        return *this;
    }

    template<class T, class AllocatorT>
    fixed_array<T, AllocatorT>::fixed_array(std::initializer_list<T> il, const allocator_type& alloc) :
        fixed_array<T, AllocatorT>(il.begin(), il.end(), alloc)
    {
    }

    template<class T, class AllocatorT>
    template<class IteratorT>
    fixed_array<T, AllocatorT>::fixed_array(IteratorT first, IteratorT last, const allocator_type& alloc) :
        allocator_type(alloc),
        m_data(MAR_NULL),
        m_capacity(0)
    {
        temporary_array<T, AllocatorT> temp(first, last, alloc);
        auto capacity = temp.capacity();
        if(capacity > 0) {
            m_capacity = capacity;
            m_data = temp.transfer();
        }
    }

    template<class T, class AllocatorT>
    fixed_array<T, AllocatorT>::fixed_array(size_type count, const value_type& value, const allocator_type& alloc) :
        allocator_type(alloc),
        m_data(MAR_NULL),
        m_capacity(0)
    {
        temporary_array<T, AllocatorT> temp(count, value, alloc);
        auto capacity = temp.capacity();
        if(capacity > 0) {
            m_capacity = capacity;
            m_data = temp.transfer();
        }
    }

    template<class T, class AllocatorT>
    fixed_array<T, AllocatorT>::~fixed_array() MAR_NO_EXCEPT
    {
        if(m_data) {
            for(size_type i = 0; i < m_capacity; ++i) {
                allocator().destroy(m_data + i);
            }
            allocator().deallocate(m_data, m_capacity);
        }
    }

    template<class T, class AllocatorT>
    void fixed_array<T, AllocatorT>::initialize(std::initializer_list<T> il, const allocator_type& alloc)
    {
        initialize(il.begin(), il.end(), alloc);
    }

    template<class T, class AllocatorT>
    template<class InputIteratorT>
    void fixed_array<T, AllocatorT>::initialize(InputIteratorT first, InputIteratorT last, const allocator_type& alloc)
    {
        static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<InputIteratorT>::iterator_category>(), "InputIteratorT must be an input iterator");

        fixed_array<T, AllocatorT> temp(first, last, alloc);
        swap(temp);
    }

    template<class T, class AllocatorT>
    void fixed_array<T, AllocatorT>::initialize(size_type count, const value_type& value, const allocator_type& alloc)
    {
        fixed_array<T, AllocatorT> temp(count, value, alloc);
        swap(temp);
    }

    template<class T, class AllocatorT>
    void fixed_array<T, AllocatorT>::swap(fixed_array& other) MAR_NO_EXCEPT
    {
        mar_assert(allocator_trait::propagate_on_container_swap::value || allocator() == other.allocator(), "Cannot swap with other container");
        std::swap(allocator(), other.allocator());
        swap_data(other);
    }

    template<class T, class AllocatorT>
    void fixed_array<T, AllocatorT>::swap_data(fixed_array& other) MAR_NO_EXCEPT
    {
        std::swap(m_data, other.m_data);
        std::swap(m_capacity, other.m_capacity);
    }
}
}
