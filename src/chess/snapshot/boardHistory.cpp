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
#include <iostream>
#include "../chess.h"
#include "boardHistory.h"

namespace cchess
{
namespace detail
{
    struct component_move_description : public mar::data_component
    {
        explicit component_move_description(char desc_) : desc(desc_) {}

        char desc;
    };

    struct component_move_components : public mar::data_component
    {
        component_move_components(bool hasMoved_, position_t xs_, position_t ys_, position_t xd_, position_t yd_) : hasMoved(hasMoved_), xs(xs_), ys(ys_), xd(xd_), yd(yd_) {}

        bool        hasMoved;
        position_t  xs;
        position_t  ys;
        position_t  xd;
        position_t  yd;
    };

    struct component_capture_components : public mar::data_component
    {
        explicit component_capture_components(Piece::eColor color_) : color(color_) {}

        Piece::eColor color;
    };

    struct component_promotion_components : public mar::data_component
    {
        explicit component_promotion_components(Piece pieceToPromoteFrom_, position_t x_, position_t y_) : pieceToPromoteFrom(pieceToPromoteFrom_), x(x_), y(y_) {}

        Piece       pieceToPromoteFrom;
        position_t  x;
        position_t  y;
    };
}
    BoardHistoryManager::MoveDescription::MoveDescription(bool hasMoved, position_t xs, position_t ys, position_t xd, position_t yd)
    {
        m_data.add_component<detail::component_move_description>(char(DESC_MOVE));
        m_data.add_component<detail::component_move_components>(hasMoved, xs, ys, xd, yd);
    }

    BoardHistoryManager::MoveDescription::MoveDescription(Piece::eColor color)
    {
        m_data.add_component<detail::component_move_description>(char(DESC_CAPTURE));
        m_data.add_component<detail::component_capture_components>(color);
    }

    BoardHistoryManager::MoveDescription::MoveDescription(Piece pieceToPromoteFrom, position_t x, position_t y)
    {
        m_data.add_component<detail::component_move_description>(char(DESC_PROMOTION));
        m_data.add_component<detail::component_promotion_components>(pieceToPromoteFrom, x, y);
    }

    char BoardHistoryManager::MoveDescription::getDescription() const
    {
        return m_data.get_component<detail::component_move_description>().desc;
    }

    BoardHistoryManager::BoardHistoryManager()
    {
        m_moveHistory.reserve(50);
        m_enPassant.reserve(50);
    }

    void BoardHistoryManager::addMoveToHistory(const std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>>& moves)
    {
        auto moveDescriptions = convertMovesToMoveDescriptions(moves);
        m_moveHistory.push_back(std::move(moveDescriptions));
    }

    void BoardHistoryManager::addCaptureToHistory(Piece::eColor color, bool hasMoved, position_t xs, position_t ys, position_t xd, position_t yd)
    {
        std::vector<MoveDescription> moveDescriptions;

        moveDescriptions.reserve(2);
        moveDescriptions.emplace_back(color);                       // add the capture first
        moveDescriptions.emplace_back(hasMoved, xs, ys, xd, yd);    // add the move second
        m_moveHistory.push_back(std::move(moveDescriptions));
    }

    void BoardHistoryManager::addLastEnPassant(Piece::eColor color, position_t x, position_t y)
    {
        m_enPassant.emplace_back(color, std::make_pair(x, y));
    }

    void BoardHistoryManager::addPromotionToHistory(Piece pieceToPromoteFrom, position_t x, position_t y)
    {
        std::vector<MoveDescription> moveDescriptions;

        moveDescriptions.reserve(1);
        moveDescriptions.emplace_back(pieceToPromoteFrom, x, y);
        m_moveHistory.push_back(std::move(moveDescriptions));
    }

    void BoardHistoryManager::resetHistory()
    {
        m_moveHistory.clear();
        m_enPassant.clear();
    }

    bool BoardHistoryManager::undoLastMove(Chess& chessboard)
    {
//        assert(m_moveHistory.size() == m_enPassant.size());

        if(m_moveHistory.size()) {
            const auto& moveDescriptions = m_moveHistory.back();

            // undo the move in reverse order
            bool isPromotion = false;
            for(auto it = moveDescriptions.rbegin(), it_end = moveDescriptions.rend(); it != it_end && !isPromotion; ++it) {
                const auto& md = *it;
                auto desc = md.getDescription();
                switch(desc) {
                case DESC_MOVE:
                    undoMove(chessboard, md);
                    break;
                case DESC_CAPTURE:
                    undoCapture(chessboard, md);
                    break;
                case DESC_PROMOTION:
                    undoPromotion(chessboard, md);
                    isPromotion = true; // promotions are only 1 move, so exit the loop
                    break;
                default:
                    break;
                }
            }

            // grab the last en passant
            const auto& lastEnPassant = m_enPassant.back();
            const auto& lastEnPassantPosition = lastEnPassant.second;

            // and set the en passant back
            chessboard.m_enPassant[getColorIndex(lastEnPassant.first)] = Chess::Position(lastEnPassantPosition.first, lastEnPassantPosition.second);

            // remove the last element on the vectors
            m_moveHistory.pop_back();
            m_enPassant.pop_back();

            return true;
        }

        return false;
    }

    void BoardHistoryManager::undoMove(Chess& chessboard, const MoveDescription& md)
    {
        const auto& c_move_components = md.m_data.get_component<detail::component_move_components>();
        auto fromPos = convert2Dto1DPosition(c_move_components.xs, c_move_components.ys, BOARD_WIDTH);
        auto toPos = convert2Dto1DPosition(c_move_components.xd, c_move_components.yd, BOARD_WIDTH);
        auto hasMoved = c_move_components.hasMoved;
        auto piece = chessboard.m_board[toPos];

        std::cout << "Type moving back: " << getCharacterOfPiece(piece) << std::endl;
        std::cout << "Type has moved?: " << hasMoved << std::endl;

        // reverse the move
        std::swap(chessboard.m_board[toPos], chessboard.m_board[fromPos]);
        if(!hasMoved)
            chessboard.m_board[fromPos].resetMovedFlag();

        // find the piece on the alive pieces
        for(auto& pieceInformation : chessboard.m_alivePieces[getColorIndex(piece.getColor())]) {
            if(pieceInformation.getPiece() == piece) {
                pieceInformation.m_position = Chess::Position(c_move_components.xs, c_move_components.ys);
                if(!hasMoved)
                    pieceInformation.m_piece.resetMovedFlag();
                break;
            }
        }
    }

    void BoardHistoryManager::undoCapture(Chess& chessboard, const MoveDescription& md)
    {
        const auto& c_capture_components = md.m_data.get_component<detail::component_capture_components>();
        auto colorIndex = getColorIndex(c_capture_components.color);
        auto& deadPieces = chessboard.m_deadPieces[colorIndex];

        const auto& lastCapturedPiece = deadPieces.back();
        const auto& lastCapturedPiecePosition = lastCapturedPiece.getPosition();
        auto piece = lastCapturedPiece.getPiece();
        auto xc = lastCapturedPiecePosition.x;
        auto yc = lastCapturedPiecePosition.y;
        auto capturePos = convert2Dto1DPosition(xc, yc, BOARD_WIDTH);

        // add back to board
        chessboard.m_board[capturePos] = piece;

        // add back to alive pieces
        chessboard.m_alivePieces[colorIndex].push_back(Chess::PieceInformation(piece, xc, yc));

        // remove from dead pieces
        deadPieces.pop_back();
    }

    void BoardHistoryManager::undoPromotion(Chess& chessboard, const MoveDescription& md)
    {
        const auto& c_promotion_components = md.m_data.get_component<detail::component_promotion_components>();
        auto pos =  convert2Dto1DPosition(c_promotion_components.x, c_promotion_components.y, BOARD_WIDTH);

        auto pieceToDemote = chessboard.m_board[pos];
        auto pieceToDemoteTo = c_promotion_components.pieceToPromoteFrom;

        // find the piece on alive pieces, and demote
        for(auto& pieceInformation : chessboard.m_alivePieces[getColorIndex(pieceToDemote.getColor())]) {
            if(pieceInformation.getPiece() == pieceToDemote)
                pieceInformation.m_piece = pieceToDemoteTo;
        }

        // demote the piece on the board
        chessboard.m_board[pos] = pieceToDemoteTo;

        // since a promotion only happens after a move, grab the last move and undo that too
        assert(m_moveHistory.size());
        const auto& moveDescriptions = *(++m_moveHistory.rbegin());
        for(auto it = moveDescriptions.rbegin(), it_end = moveDescriptions.rend(); it != it_end; ++it) {
            const auto& bmd = *it;
            auto desc = bmd.getDescription();
            switch(desc) {
            case DESC_MOVE:
                undoMove(chessboard, bmd);
                break;
            case DESC_CAPTURE:
                undoCapture(chessboard, bmd);
                break;
            default:
                break;
            }
        }

        // remove the promotion from the move history, and the main undoLastMove function
        // will remove the move function
        m_moveHistory.pop_back();
    }

    std::vector<BoardHistoryManager::MoveDescription> BoardHistoryManager::convertMovesToMoveDescriptions(const std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>>& moves)
    {
        std::vector<MoveDescription> ret;
        ret.reserve(moves.size());

        for(const auto& t : moves) {
            auto piece = std::get<0>(t);
            auto xs = std::get<1>(t);
            auto ys = std::get<2>(t);
            auto xd = std::get<3>(t);
            auto yd = std::get<4>(t);

            ret.emplace_back(piece.hasMoved(), xs, ys, xd, yd);
        }

        return ret;
    }
}
