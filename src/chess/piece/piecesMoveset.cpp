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
#include <unordered_map>
#include "../chess.h"
#include "piecesMoveset.h"

namespace cchess
{
namespace detail
{
    static bool isBishopMoveValid(position_t xs, position_t ys, position_t xd, position_t yd, Piece::eColor, const Chess& chessboard)
    {
        auto dx = xd - xs;
        auto dy = yd - ys;
        auto adx = std::abs(dx);
        auto ady = std::abs(dy);
        if(adx == ady) {
            auto x = xs;
            auto y = ys;
            if(adx && ady) {
                auto inc_x = dx / adx;
                auto inc_y = dy / ady;

                for(position_t i = 0; i < adx - 1; ++i) {
                    x += inc_x;
                    y += inc_y;
                    auto piece = chessboard.getBoardPiece(x, y);
                    if(!piece.isEmpty())
                        return false;
                }
            }

            return true;
        }

        return false;
    }

    bool isRookMoveValid(position_t xs, position_t ys, position_t xd, position_t yd, Piece::eColor, const Chess& chessboard)
    {
        auto dx = xd - xs;
        auto dy = yd - ys;
        auto adx = std::abs(dx);
        auto ady = std::abs(dy);
        if(xs == xd) {
            auto x = xs;
            auto y = ys;
            if(ady) {
                auto inc_y = dy / ady;
                for(position_t i = 0; i < ady - 1; ++i) {
                    y += inc_y;
                    auto piece = chessboard.getBoardPiece(x, y);
                    if(!piece.isEmpty())
                        return false;
                }
            }

            return true;
        } else if(ys == yd) {
            auto x = xs;
            auto y = ys;
            if(adx) {
                auto inc_x = dx / adx;
                for(position_t i = 0; i < adx - 1; ++i) {
                    x += inc_x;
                    auto piece = chessboard.getBoardPiece(x, y);
                    if(!piece.isEmpty())
                        return false;
                }
            }

            return true;
        }

        return false;
    }
}
    static std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>> isPawnMoveValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        auto color = piece.getColor();
        const auto pieceMove = chessboard.getPawnDirection(color);
        if(yd == ys + pieceMove) {
            if(xd == xs)
//                if(chessboard.getBoardPiece(xd, yd).isEmpty())
                    return { { piece, xs, ys, xd, yd } };
        } else if(!piece.hasMoved() && yd == ys + (pieceMove * 2)) {
            if(xd == xs) {
                if(detail::isRookMoveValid(xs, ys, xd, yd, color, chessboard))
                    return { { piece, xs, ys, xd, yd } };
            }
        }

        return {};
    }

    static std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>> isKnightMoveValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess&)
    {
        static const std::vector<std::pair<position_t, position_t>> KNIGHT_VALID_MOVES =
        {
            { -1, -2 },
            { 1, - 2 },
            { -1, 2 },
            { 1, 2 },
            { -2, 1 },
            { -2, -1 },
            { 2, 1 },
            { 2, -1 }
        };

        for(const auto& moves : KNIGHT_VALID_MOVES) {
            if(xd == xs + moves.first && yd == ys + moves.second)
                return { { piece, xs, ys, xd, yd } };
        }

        return {};
    }

    static std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>> isBishopMoveValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        if(detail::isBishopMoveValid(xs, ys, xd, yd, piece.getColor(), chessboard))
            return { { piece, xs, ys, xd, yd } };

        return {};
    }

    static std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>> isRookMoveValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        if(detail::isRookMoveValid(xs, ys, xd,yd, piece.getColor(), chessboard))
            return { { piece, xs, ys, xd, yd } };

        return {};
    }

    static std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>> isQueenMoveValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        auto color = piece.getColor();
        if(detail::isBishopMoveValid(xs, ys, xd, yd, color, chessboard) ||
           detail::isRookMoveValid(xs, ys, xd, yd, color, chessboard))
            return { { piece, xs, ys, xd, yd } };

        return {};
    }

    static std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>> isKingSpecialMoveValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        // castling:
        // - The king cannot be in check
        // - nor can the king pass through any square that is under attack by an enemy piece,
        // - or move to a square that would result in check.
        if(!piece.hasMoved()) {
            auto dx = xd - xs;
            auto adx = std::abs(dx);
            if(ys == yd && adx == 2) {
                if(!isCheck(Chess::PieceInformation(piece, xs, ys), xs, ys, chessboard).size()) {
                    auto xf = xs;
                    auto inc_x = dx / adx;
                    const auto IBOARD_WIDTH = static_cast<position_t>(BOARD_WIDTH);
                    while(xf >= 0 && xf < IBOARD_WIDTH) {
                        xf += inc_x;
                        auto otherPiece = chessboard.getBoardPiece(xf, ys);
                        if(!otherPiece.isEmpty()) {
                            if(otherPiece.getType() == Piece::Type::ROOK && piece.getColor() == otherPiece.getColor() && !otherPiece.hasMoved()) {
                                auto xm = xs + inc_x;
                                if(!isCheck(Chess::PieceInformation(piece, xs, ys), xm, ys, chessboard).size()) {
                                    return
                                    {
                                        { piece, xs, ys, xd, yd },
                                        { otherPiece, xf, ys, xd + (inc_x * - 1), yd }
                                    };
                                }
                            } else
                                return {};
                        }
                    }
                }
            }
        }

        return {};
    }

    static std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>> isKingMoveValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        auto dx = xd - xs;
        auto dy = yd - ys;
        auto adx = std::abs(dx);
        auto ady = std::abs(dy);
        if(adx <= 1 && ady <= 1) {
            auto oppositeColor = getOppositeColor(piece.getColor());
            const auto& otherKing = chessboard.getKing(oppositeColor);
            const auto& otherKingPosition = otherKing.getPosition();

            auto kdx = otherKingPosition.x - xd;
            auto kdy = otherKingPosition.y - yd;
            if(std::abs(kdx) > 1 || std::abs(kdy) > 1)
                return { { piece, xs, ys, xd, yd } };
        }

        return isKingSpecialMoveValid(piece, xs, ys, xd, yd, chessboard);
    }

    bool isSelfMove(position_t xs, position_t ys, position_t xd, position_t yd)
    {
        return xs == xd && ys == yd;
    }

    std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>> isMoveValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard)
    {
        using function_ptr_type = std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>>(*)(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard);

        static const std::unordered_map<Piece::Type, function_ptr_type> TYPE_TO_MOVE =
        {
            { Piece::Type::PAWN, &isPawnMoveValid },
            { Piece::Type::KNIGHT, &isKnightMoveValid },
            { Piece::Type::BISHOP, &isBishopMoveValid },
            { Piece::Type::ROOK, &isRookMoveValid },
            { Piece::Type::QUEEN, &isQueenMoveValid },
            { Piece::Type::KING, &isKingMoveValid }
        };

//        if(!piece.isEmpty()) {
        assert(!piece.isEmpty());
        if(isSelfMove(xs, ys, xd, yd))
            return {};

        assert(TYPE_TO_MOVE.find(piece.getType()) != TYPE_TO_MOVE.end()); // if this spits an error, we made a mistake in making the piece
        return (*TYPE_TO_MOVE.at(piece.getType()))(piece, xs, ys, xd, yd, chessboard);
//        }

//        return {};
    }
}
