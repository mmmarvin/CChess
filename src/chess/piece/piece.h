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
#include <locale>

namespace cchess
{
    class Piece
    {
    public:
        enum class eColor : unsigned char { white = 0, black = 1 };
        using color_index_type = std::underlying_type_t<eColor>;

        static constexpr unsigned char NUMBER_OF_COLOR = 2;

        enum class Type : char {
            EMPTY = ' ',
            PAWN = 'P',
            KNIGHT = 'N',
            BISHOP = 'B',
            ROOK = 'R',
            QUEEN = 'Q',
            KING = 'K'
        };
//        static constexpr auto EMPTY = ' ';
//        static constexpr auto PAWN = 'P';
//        static constexpr auto KNIGHT = 'N';
//        static constexpr auto BISHOP = 'B';
//        static constexpr auto ROOK = 'R';
//        static constexpr auto QUEEN = 'Q';
//        static constexpr auto KING = 'K';

        Piece();
        Piece(Type type, unsigned char id, eColor color);

        bool isEmpty() const { return m_type == Type::EMPTY; }

        Type getType() const { return m_type; }
        eColor getColor() const { return m_color; }
        bool hasMoved() const { return m_hasMoved; }
        unsigned char getId() const { return m_id; }

        void setMovedFlag();
        void resetMovedFlag();

    private:
        friend bool operator==(const Piece& lhs, const Piece& rhs);

        Type            m_type;
        eColor          m_color;
        bool            m_hasMoved;
        unsigned char   m_id;
    };

    bool operator==(const Piece& lhs, const Piece& rhs);
    bool operator!=(const Piece& lhs, const Piece& rhs);

    char getCharacterOfPiece(Piece piece);
    Piece::eColor getOppositeColor(Piece::eColor color);
    std::underlying_type_t<Piece::eColor> getColorIndex(Piece::eColor color);
}
