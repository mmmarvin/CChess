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
#include <assert.h>
#include "piece.h"

namespace cchess
{
    Piece::Piece() :
        m_type(Type::EMPTY),
        m_color(eColor::white),
        m_hasMoved(false),
        m_id(0)
    {
    }

    Piece::Piece(Type type, unsigned char id, eColor color) :
        m_type(type),
        m_color(color),
        m_hasMoved(false),
        m_id(id)
    {
        assert(m_type == Type::PAWN ||
               m_type == Type::KNIGHT ||
               m_type == Type::BISHOP ||
               m_type == Type::ROOK ||
               m_type == Type::QUEEN ||
               m_type == Type::KING);
    }

    void Piece::setMovedFlag()
    {
//        if(!hasMoved())
            m_hasMoved = true;
    }

    void Piece::resetMovedFlag()
    {
//        if(hasMoved())
            m_hasMoved = false;
    }

    bool operator==(const Piece& lhs, const Piece& rhs)
    {
        return lhs.m_type == rhs.m_type &&
               lhs.m_color == rhs.m_color &&
               lhs.m_hasMoved == rhs.m_hasMoved &&
               lhs.m_id == rhs.m_id;
    }

    bool operator!=(const Piece& lhs, const Piece& rhs)
    {
        return !(lhs == rhs);
    }

    char getCharacterOfPiece(Piece piece)
    {
        return static_cast<char>(piece.getType());
    }

    Piece::eColor getOppositeColor(Piece::eColor color)
    {
        return color == Piece::eColor::white ? Piece::eColor::black : Piece::eColor::white;
    }

    std::underlying_type_t<Piece::eColor> getColorIndex(Piece::eColor color)
    {
        using underlying_color_type = std::underlying_type_t<Piece::eColor>;
        return static_cast<underlying_color_type>(color);
    }
}
