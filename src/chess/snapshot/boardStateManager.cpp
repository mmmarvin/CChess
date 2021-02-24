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
#include "boardStateManager.h"

namespace cchess
{
    BoardStateManager::BoardStateManager(const Chess& chessboard)
    {
        resetStates(chessboard);
    }

    void BoardStateManager::resetStates(const Chess& chessboard)
    {
        m_currentBoardState = 0;
        m_threefoldRepetition = false;

        // initialize the state of the board
        for(auto x = 0; x < BOARD_WIDTH; ++x) {
            for(auto y = 0; y < BOARD_HEIGHT; ++y) {
                auto piece = chessboard.getBoardPiece(x, y);

                if(!piece.isEmpty())
                    m_currentBoardState = m_zobristTable.toggleState(m_currentBoardState, x, y, getPieceState(piece));
            }
        }

        m_boardStates.clear();
        m_boardStatesRepetitions.clear();
    }

    void BoardStateManager::updateStateOnMove(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd)
    {
        m_currentBoardState = m_zobristTable.toggleState(m_currentBoardState, xs, ys, getPieceState(piece));
        m_currentBoardState = m_zobristTable.toggleState(m_currentBoardState, xd, yd, getPieceState(piece));

        m_boardStates.push_back(m_currentBoardState);
        ++m_boardStatesRepetitions[m_currentBoardState];

        if(m_boardStatesRepetitions[m_currentBoardState] >= 3)
            m_threefoldRepetition = true;
    }

    void BoardStateManager::updateStateOnCapture(Piece piece, Piece capturedPiece, position_t xs, position_t ys, position_t xd, position_t yd, position_t xc, position_t yc)
    {
        m_currentBoardState = m_zobristTable.toggleState(m_currentBoardState, xc, yc, getPieceState(capturedPiece));
        updateStateOnMove(piece, xs, ys, xd, yd);
    }

    bool BoardStateManager::undoLastState()
    {
        if(m_boardStates.size()) {
            auto lastState = m_boardStates.back();
            --m_boardStatesRepetitions.at(lastState);
            m_boardStates.pop_back();

            // search the container if there are still threefold repetitions
            m_threefoldRepetition = false;
            for(const auto& p : m_boardStatesRepetitions) {
                if(p.second >= 3) {
                    m_threefoldRepetition = true;
                    break;
                }
            }
            return true;
        }

        return false;
    }

    std::size_t BoardStateManager::getPieceState(Piece piece)
    {
        static const std::unordered_map<Piece::Type, std::size_t> PIECE_TO_INDEX =
        {
            { Piece::Type::PAWN, 0 },
            { Piece::Type::KNIGHT, 2 },
            { Piece::Type::BISHOP, 6 },
            { Piece::Type::ROOK, 10 },
            { Piece::Type::QUEEN, 14 },
            { Piece::Type::KING, 16 },
        };

        // how to visualize colorIndex, idIndex, pieceIndex:
        // ex: knight
        // wk0 wk1 bk0 bk1
        //
        // piece + color + id = table index
        // 2 + (0) + 0 = 2
        // 2 + (0) + 1 = 3
        // 2 + (2) + 0 = 4
        // 2 + (2) + 1 = 5

        auto pieceType = piece.getType();
        bool isNotIdedTypes = (pieceType == Piece::Type::PAWN ||
                               pieceType == Piece::Type::QUEEN ||
                               pieceType == Piece::Type::KING);

        auto colorIndex = piece.getColor() == Piece::eColor::white ? 0 : 1;
        colorIndex = isNotIdedTypes ? colorIndex : colorIndex * 2;
        auto idIndex = isNotIdedTypes ? 0 : piece.getId();
        auto pieceIndex = PIECE_TO_INDEX.at(pieceType);

        return pieceIndex + colorIndex + idIndex;
    }
}
