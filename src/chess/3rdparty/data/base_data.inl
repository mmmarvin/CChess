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
// headers
#include <cassert>
#include "base_data.h"

namespace mar
{
namespace detail
{
    template<class PointerPolicyT>
    template<class ComponentT, class... Args>
    ComponentT& base_data<PointerPolicyT>::add_component(Args&&... args)
    {
        static_assert(std::is_base_of<data_component, ComponentT>(), "ComponentT must inherit from data_component");
        assert(!does_have_component<ComponentT>());

        auto id = component_id::template get_id<ComponentT>();
        auto it = m_components.emplace(id, PointerPolicyT::template make_ptr<ComponentT>(std::forward<Args>(args)...));
        return *static_cast<ComponentT*>((*(it.first)).second.get());
    }

    template<class PointerPolicyT>
    template<class ComponentT>
    bool base_data<PointerPolicyT>::does_have_component() const
    {
        static_assert(std::is_base_of<data_component, ComponentT>(), "ComponentT must inherit from data_component");

        auto id = component_id::template get_id<ComponentT>();
        return m_components.find(id) != m_components.end();
    }

    template<class PointerPolicyT>
    template<class ComponentT>
    ComponentT& base_data<PointerPolicyT>::get_component()
    {
        static_assert(std::is_base_of<data_component, ComponentT>(), "ComponentT must inherit from data_component");
        assert(does_have_component<ComponentT>());

        auto id = component_id::template get_id<ComponentT>();
        return *static_cast<ComponentT*>(m_components.at(id).get());
    }

    template<class PointerPolicyT>
    template<class ComponentT>
    const ComponentT& base_data<PointerPolicyT>::get_component() const
    {
        static_assert(std::is_base_of<data_component, ComponentT>(), "ComponentT must inherit from data_component");
        assert(does_have_component<ComponentT>());

        auto id = component_id::template get_id<ComponentT>();
        return *static_cast<ComponentT*>(m_components.at(id).get());
    }

    template<class PointerPolicyT> std::atomic<typename base_data<PointerPolicyT>::component_id_type> base_data<PointerPolicyT>::component_id::m_id(0);
}
}
