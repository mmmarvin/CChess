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
#include "time.h"

namespace mar
{
    time& operator+=(time& lhs, const time& rhs)
    {
        lhs.m_value += rhs.m_value;
        return lhs;
    }

    time operator+(const time& lhs, const time& rhs)
    {
        auto ret = lhs;
        ret += rhs;

        return ret;
    }

    time& operator-=(time& lhs, const time& rhs)
    {
        lhs.m_value -= rhs.m_value;
        return lhs;
    }

    time operator-(const time& lhs, const time& rhs)
    {
        auto ret = lhs;
        ret -= rhs;

        return ret;
    }

    bool operator<(const time& lhs, const time& rhs)
    {
        return lhs.m_value < rhs.m_value;
    }

    bool operator>(const time& lhs, const time& rhs)
    {
        return lhs.m_value > rhs.m_value;
    }

    bool operator==(const time& lhs, const time& rhs)
    {
        return lhs.m_value == rhs.m_value;
    }
}
