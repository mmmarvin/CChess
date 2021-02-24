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
#ifndef GUARD_MAR_data_base_data_H
#define GUARD_MAR_data_base_data_H

// headers
#include <atomic>
#include <memory>
#include <unordered_map>
#include "detail/data_component.h"

namespace mar
{
    template<class PointerPolicyT> class base_data;
namespace detail
{
    struct base_data_fast_policy
    {
        template<class T> using pointer_policy = std::unique_ptr<T>;

        template<class T, class... Args> static pointer_policy<T> make_ptr(Args&&... args)
        {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }
    };

    struct base_data_slow_policy
    {
        template<class T> using pointer_policy = std::shared_ptr<T>;

        template<class T, class... Args> static pointer_policy<T> make_ptr(Args&&... args)
        {
            return std::make_shared<T>(std::forward<Args>(args)...);
        }
    };

    template<class PointerPolicyT>
    class base_data
    {
        using component_id_type = std::size_t;
        template<class T> using pointer_policy = typename PointerPolicyT::template pointer_policy<T>;

        struct component_id
        {
        public:
            template<class T>
            static component_id_type get_id()
            {
                static const component_id_type id = m_id++;
                return id;
            }

        private:
            static std::atomic<component_id_type> m_id;
        };

        using container_type = std::unordered_map<component_id_type, pointer_policy<data_component>>;

    public:
        base_data() {}
        base_data(const base_data&) = delete;
        base_data& operator=(const base_data&) = delete;
        base_data(base_data&&) = default;
        base_data& operator=(base_data&&) = default;

        template<class ComponentT, class... Args> ComponentT& add_component(Args&&... args);
        template<class ComponentT> bool does_have_component() const;
        template<class ComponentT> ComponentT& get_component();
        template<class ComponentT> const ComponentT& get_component() const;

    private:
        container_type m_components;
    };
}
    using unique_base_data = detail::base_data<detail::base_data_fast_policy>;
    using shared_base_data = detail::base_data<detail::base_data_slow_policy>;
}

// definitions
#include "base_data.inl"

#endif // GUARD_MAR_data_base_data_H
