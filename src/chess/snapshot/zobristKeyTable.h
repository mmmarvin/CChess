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
#pragma once

// headers
#include "../3rdparty/container/fixed_sized_array.h"
#include <random>

namespace cchess
{
    template<std::size_t Width,
             std::size_t Height,
             std::size_t NumberOfStates>
    class ZobristKeyTable
    {
        static std::mt19937 randomizer;

    public:
        using state_type = unsigned long long int;

        ZobristKeyTable();

        state_type toggleState(state_type state, std::size_t x, std::size_t y, std::size_t z) const;

    private:
        using table_type = mar::container::fixed_sized_array<std::array<state_type, NumberOfStates>, Width * Height>;

        state_type getRandomInteger() const;

        table_type m_table;
    };

    template<std::size_t Width,
             std::size_t Height,
             std::size_t NumberOfStates>
    std::mt19937 ZobristKeyTable<Width, Height, NumberOfStates>::randomizer(01234567);
}

// definitions
#include "zobristKeyTable.inl"
