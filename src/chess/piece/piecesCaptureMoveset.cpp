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
#include <unordered_map>
#include "../chess.h"
#include "piecesMoveset.h"
#include "piecesCaptureMoveset.h"

namespace cchess
{
    static std::tuple<bool, position_t, position_t> isDefaultCaptureValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        return { isMoveValid(piece,xs, ys, xd, yd, chessboard).size(), xd, yd };
    }

    static std::tuple<bool, position_t, position_t> isPawnCaptureValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        auto color = piece.getColor();
        auto pieceMoveDirection = chessboard.getPawnDirection(color);
        if(yd == ys + pieceMoveDirection) {
            auto dx = xd - xs;
            auto adx = std::abs(dx);
            if(adx == 1)
                return { true, xd, yd };
        }

        return { false, -1, -1 };
    }

    std::tuple<bool, position_t, position_t> isPawnSpecialCaptureValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        auto color = piece.getColor();
        auto pieceMoveDirection = chessboard.getPawnDirection(color);
        if(yd == ys + pieceMoveDirection) {
            auto dx = xd - xs;
            auto adx = std::abs(dx);
            if(adx == 1) {
                const auto& enPassantPosition = chessboard.getEnPassantPosition(getOppositeColor(color));
                auto xc = enPassantPosition.x;
                auto yc = enPassantPosition.y;
                if(xc == xd && yc == ys)
                    return { true, xc, yc };
            }
        }

        return { false, -1, -1 };
    }

    std::tuple<bool, position_t, position_t> isCaptureValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        using function_ptr_type = std::tuple<bool, position_t, position_t>(*)(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard);
        static const std::unordered_map<Piece::Type, function_ptr_type> TYPE_TO_CAPTURE =
        {
            { Piece::Type::PAWN, &isPawnCaptureValid },
            { Piece::Type::KNIGHT, &isDefaultCaptureValid },
            { Piece::Type::BISHOP, &isDefaultCaptureValid },
            { Piece::Type::ROOK, &isDefaultCaptureValid },
            { Piece::Type::QUEEN, &isDefaultCaptureValid },
            { Piece::Type::KING, &isDefaultCaptureValid },
        };

        assert(!piece.isEmpty());
        if(isSelfMove(xs, ys, xd, yd))
            return { false, -1, -1 };

        assert(TYPE_TO_CAPTURE.find(piece.getType()) != TYPE_TO_CAPTURE.end()); // if this spits an erro, we made a mistake in make the piece
        return (*TYPE_TO_CAPTURE.at(piece.getType()))(piece, xs, ys, xd, yd, chessboard);
    }
}
