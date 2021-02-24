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
#include <vector>
#include "../3rdparty/data/base_data.h"
#include "../piece/piece.h"
#include "../types.h"

namespace cchess
{
    class Chess;
    class BoardHistoryManager
    {
        static constexpr char DESC_MOVE = 'M';
        static constexpr char DESC_CAPTURE = 'C';
        static constexpr char DESC_PROMOTION = 'P';

        class MoveDescription
        {
        public:
            // move constructor
            MoveDescription(bool hasMoved, position_t xs, position_t ys, position_t xd, position_t yd);

            // capture constructor
            MoveDescription(Piece::eColor color);

            // promotion constructor
            MoveDescription(Piece pieceToPromoteFrom, position_t x, position_t y);

            // get description
            char getDescription() const;

        private:
            friend class BoardHistoryManager;

            mar::unique_base_data m_data;
        };

    public:
        BoardHistoryManager();

        void addMoveToHistory(const std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>>& moves);
        void addCaptureToHistory(Piece::eColor color, bool hasMoved, position_t xs, position_t ys, position_t xd, position_t yd);
        void addPromotionToHistory(Piece pieceToPromoteFrom, position_t x, position_t y);
        void addLastEnPassant(Piece::eColor color, position_t x, position_t y);

        void resetHistory();

        bool isThereHistoryToUndo() const { return m_moveHistory.size(); }
        bool undoLastMove(Chess& chessboard);

    private:
        std::vector<MoveDescription> convertMovesToMoveDescriptions(const std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>>& moves);

        void undoMove(Chess& chessboard, const MoveDescription& md);
        void undoCapture(Chess& chessboard, const MoveDescription& md);
        void undoPromotion(Chess& chessboard, const MoveDescription& md);

        std::vector<std::vector<MoveDescription>>                                   m_moveHistory;
        std::vector<std::pair<Piece::eColor, std::pair<position_t, position_t>>>    m_enPassant;
    };
}
