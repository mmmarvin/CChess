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
#include "zobristKeyTable.h"

namespace cchess
{
    template<std::size_t Width, std::size_t Height, std::size_t NumberOfStates>
    ZobristKeyTable<Width, Height, NumberOfStates>::ZobristKeyTable()
    {
        // initialize the zobrist table
        for(std::size_t i = 0, i_size = Width * Height; i < i_size; ++i) {
            for(std::size_t j = 0; j < NumberOfStates; ++j) {
                m_table[i][j] = getRandomInteger();
            }
        }
    }

    template<std::size_t Width, std::size_t Height, std::size_t NumberOfStates>
    typename ZobristKeyTable<Width, Height, NumberOfStates>::state_type ZobristKeyTable<Width, Height, NumberOfStates>::toggleState(state_type state, std::size_t x, std::size_t y, std::size_t z) const
    {
        assert(x < Width && y < Height && z < NumberOfStates);
        auto pos = (x * Width + y);
        state ^= m_table[pos][z];

        return state;
    }

    template<std::size_t Width, std::size_t Height, std::size_t NumberOfStates>
    typename ZobristKeyTable<Width, Height, NumberOfStates>::state_type ZobristKeyTable<Width, Height, NumberOfStates>::getRandomInteger() const
    {
        std::uniform_int_distribution<state_type> dist(0, std::numeric_limits<state_type>::max());
        return dist(randomizer);
    }
}
