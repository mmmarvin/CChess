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
#include <cinttypes>
#include <vector>
#include "3rdparty/container/fixed_sized_array.h"
#include "3rdparty/high_resolution_clock.h"
#include "piece/piece.h"
#include "snapshot/boardHistory.h"
#include "snapshot/boardStateManager.h"
#include "define.h"
#include "types.h"

namespace cchess
{
    /// Todo:
    /// Fix problem where promoted pieces don't have unique ids
    class Chess
    {
    public:
        enum class EMoveResult : unsigned char
        {
            Invalid,
            Move,
            Capture
        };

        static constexpr position_t RANK_1 = 0;
        static constexpr position_t RANK_8 = BOARD_HEIGHT - 1;

        struct Position
        {
            Position() {}
            Position(position_t x_, position_t y_) : x(x_), y(y_) {}

            position_t x;
            position_t y;
        };

        class PieceInformation
        {
        public:
            PieceInformation() : m_position(-1, -1) {}
            PieceInformation(Piece piece, position_t x, position_t y) : m_piece(piece), m_position(x, y) {}

            bool isEmpty() const { return m_piece.isEmpty(); }

            Piece getPiece() const { return m_piece; }
            const Position& getPosition() const { return m_position; }

        private:
            friend class Chess;
            friend class BoardHistoryManager;

            friend bool operator==(const Chess::PieceInformation& lhs, const Chess::PieceInformation& rhs);

            Piece       m_piece;
            Position    m_position;
        };

        using pieces_information_container_type = std::vector<PieceInformation>;
        using select_piece_return_type = std::pair<PieceInformation, std::vector<Position>>;

        Chess();

        void update();
        void resetBoard(Piece::eColor bottomColor, bool hasTurnClock = false, bool disableOppositeColorHint = true);

        int getTimeLeft(Piece::eColor color);
        void resetTimer();

        select_piece_return_type selectPiece(position_t x, position_t y) const;
        select_piece_return_type selectPiece(const std::string& pos) const;
        EMoveResult move(position_t xs, position_t ys, position_t xd, position_t yd);
        EMoveResult move(const std::string& from, const std::string& to);
        bool undo();

        bool isOnCheck(Piece::eColor color) const;
        bool isCheckMate(Piece::eColor color) const;
        bool isStaleMate(Piece::eColor color) const;
        bool isThereThreefoldRepetition() const { return m_boardStateManager.isThereThreefoldRepetition(); }

        void setTypeToPromoteTo(Piece::Type type);
        bool isWaitingForPromotion() const { return m_isWaitingForPromotion; }

        position_t getPawnDirection(Piece::eColor color) const { return color == m_bottomColor ? -1 : 1; }
        Piece getBoardPiece(position_t x, position_t y) const;
        const pieces_information_container_type& getAlivePieces(Piece::eColor color) const { return m_alivePieces[getColorIndex(color)]; }
        const pieces_information_container_type& getDeadPieces(Piece::eColor color) const { return m_deadPieces[getColorIndex(color)]; }
        const Position& getEnPassantPosition(Piece::eColor color) const { return m_enPassant[getColorIndex(color)]; }
        const PieceInformation& getKing(Piece::eColor color) const { return m_king[getColorIndex(color)]; }
        std::vector<Position> getValidMoves(position_t x, position_t y) const;

        std::string getBoardString() const;
        std::string getBoardStringPieces() const;

    private:
        friend class BoardHistoryManager;

        using board_container_type = mar::container::fixed_sized_array<Piece, BOARD_WIDTH * BOARD_HEIGHT>;

        bool isPositionValid(position_t pos) const;

        void initPieces(Piece::eColor color);
        void addPiece(Piece::Type type, unsigned char id, Piece::eColor color, position_t x, position_t y, bool isKing = false);
        void updatePositionIfKing(Piece piece, Piece::eColor color, position_t x, position_t y) const;
        void updateKingPosition(Piece::eColor color, position_t x, position_t y) const;

        bool movePawn(Piece piece, Piece::eColor fromColor, Piece::color_index_type fromColorIndex, position_t xs, position_t ys, position_t xd, position_t yd);
        void checkPawnPromotion(position_t x, position_t y);

        std::vector<PieceInformation> copyAlivePiecesExcept(Piece piece, Piece::eColor color) const;
        Piece simulateMove(position_t fromPos, position_t toPos, position_t xd, position_t yd) const;
        void reverseMove(position_t fromPos, position_t toPos, position_t xs, position_t ys) const;
        void finalizeMove(const PieceInformation& pieceInformation, Piece::eColor color);
        std::pair<Piece, Piece> simulateCapture(position_t fromPos, position_t toPos, position_t capturePos, position_t xd, position_t yd) const;
        void reverseCapture(position_t fromPos, position_t toPos, position_t capturePos, position_t xs, position_t ys, Piece& temporary) const;
        void removePieceFormAlivePieces(Piece piece);

        bool tryMove(const std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>>&  moves);
        std::pair<bool, Piece> tryCapture(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, position_t xc, position_t yc);
        bool tryTemporaryMove(const std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>>& moves) const;
        bool tryTemporaryCapture(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, position_t xc, position_t yc) const;

        std::vector<Position> getValidMoves(Piece piece, position_t x, position_t y) const;

        mutable board_container_type            m_board;
        BoardStateManager                       m_boardStateManager;
        BoardHistoryManager                     m_boardHistoryManager;
        Piece::eColor                           m_bottomColor;

        Piece::eColor                           m_currentColorsTurn;
        bool                                    m_disableOppositeColorHint;

        bool                                    m_isWaitingForPromotion;
        Piece::eColor                           m_colorWaitingForPromotion;
        Position                                m_positionForPromotion;

        Position                                m_enPassant[Piece::NUMBER_OF_COLOR];
        pieces_information_container_type       m_alivePieces[Piece::NUMBER_OF_COLOR];
        pieces_information_container_type       m_deadPieces[Piece::NUMBER_OF_COLOR];
        mutable PieceInformation                m_king[Piece::NUMBER_OF_COLOR];

        mar::high_resolution_clock              m_turnClock[Piece::NUMBER_OF_COLOR];
        int                                     m_turnClockTimeLeft[Piece::NUMBER_OF_COLOR];
        bool                                    m_hasTurnClock;
    };

    bool operator==(const Chess::PieceInformation& lhs, const Chess::PieceInformation& rhs);
    bool operator!=(const Chess::PieceInformation& lhs, const Chess::PieceInformation& rhs);
    bool operator==(const Chess::Position& lhs, const Chess::Position& rhs);
    bool operator!=(const Chess::Position& lhs, const Chess::Position& rhs);

    position_t convert2Dto1DPosition(position_t x, position_t y, position_t width);
    std::pair<position_t, position_t> convertStringPositionToInt(const std::string& pos);
    std::vector<Chess::PieceInformation> isCheck(const Chess::PieceInformation& king, position_t xd, position_t yd, const Chess& chessboard);
    std::vector<Chess::PieceInformation> isCheck(position_t xd, position_t yd, const Chess& chessboard, const std::vector<Chess::PieceInformation>& alivePieces);
}
