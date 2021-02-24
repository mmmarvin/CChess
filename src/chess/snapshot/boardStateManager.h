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
#include <unordered_map>
#include <vector>
#include "../piece/piece.h"
#include "../define.h"
#include "../types.h"
#include "zobristKeyTable.h"

namespace cchess
{
    class Chess;

    /// TODO:
    /// Add state for en passant pawns
    /// Add state for kings that can castle
    class BoardStateManager
    {
        static constexpr std::size_t NUMBER_OF_PIECE_STATES = 18;
        using zobrist_table_type = ZobristKeyTable<BOARD_WIDTH, BOARD_HEIGHT, NUMBER_OF_PIECE_STATES>;
        using state_type = zobrist_table_type::state_type;

    public:
        BoardStateManager(const Chess& chessboard);

        void resetStates(const Chess& chessboard);

        void updateStateOnMove(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd);
        void updateStateOnCapture(Piece piece, Piece capturedPiece, position_t xs, position_t ys, position_t xd, position_t yd, position_t xc, position_t yc);

        bool isThereStateToUndo() const { return m_boardStates.size(); }
        bool undoLastState();

        bool isThereThreefoldRepetition() const { return m_threefoldRepetition; }
        state_type getCurrentState() const { return m_currentBoardState; }

    private:
        std::size_t getPieceState(Piece piece);

        zobrist_table_type                              m_zobristTable;
        state_type                                      m_currentBoardState;

        std::vector<state_type>                         m_boardStates;
        std::unordered_map<state_type, unsigned int>    m_boardStatesRepetitions;

        bool                                            m_threefoldRepetition;
    };
}
