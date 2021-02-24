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
#include "debug/debug_log.h"
#include "piece/piecesCaptureMoveset.h"
#include "piece/piecesMoveset.h"
#include "chess.h"

namespace cchess
{
    Chess::Chess() :
        m_boardStateManager(*this),
        m_hasTurnClock(false)
    {
        resetBoard(Piece::eColor::white);
    }

    void Chess::update()
    {
        if(m_hasTurnClock) {
            if(m_turnClockTimeLeft[getColorIndex(Piece::eColor::white)] < 1 ||
               m_turnClockTimeLeft[getColorIndex(Piece::eColor::black)] < 1 ) {
                return;
            }

            auto colorTurn = getColorIndex(m_currentColorsTurn);
            auto& turnClock = m_turnClock[colorTurn];
            auto elapsed = turnClock.get_elapsed().as_seconds();

            if(elapsed >= 1.0) {
                auto& turnClockElapsed = m_turnClockTimeLeft[colorTurn];
                turnClockElapsed -= static_cast<int>(floor(elapsed));
                turnClockElapsed = std::max(turnClockElapsed, 0);
                turnClock.restart();
            }
        }
    }

    void Chess::resetBoard(Piece::eColor color, bool hasTurnClock, bool disableOppositeColorHint)
    {
        m_board = board_container_type(Piece());
        m_bottomColor = color;
        m_currentColorsTurn = Piece::eColor::white;
        m_disableOppositeColorHint = disableOppositeColorHint;

        m_hasTurnClock = hasTurnClock;
        m_turnClockTimeLeft[getColorIndex(Piece::eColor::white)] = 5 * 60;
        m_turnClockTimeLeft[getColorIndex(Piece::eColor::black)] = 5 * 60;
        m_turnClock[getColorIndex(Piece::eColor::white)].restart();
        m_turnClock[getColorIndex(Piece::eColor::black)].restart();

        auto whiteColorIndex = getColorIndex(Piece::eColor::white);
        auto blackColorIndex = getColorIndex(Piece::eColor::black);
        m_alivePieces[whiteColorIndex].clear();
        m_alivePieces[blackColorIndex].clear();
        m_deadPieces[whiteColorIndex].clear();
        m_deadPieces[blackColorIndex].clear();

        initPieces(Piece::eColor::white);
        initPieces(Piece::eColor::black);

        m_enPassant[whiteColorIndex] = Position(-1, -1);
        m_enPassant[blackColorIndex] = Position(-1, -1);

        m_boardStateManager.resetStates(*this);
        m_boardHistoryManager.resetHistory();
    }

    int Chess::getTimeLeft(Piece::eColor color)
    {
        return m_turnClockTimeLeft[getColorIndex(color)];
    }

    void Chess::resetTimer()
    {
        m_turnClock[getColorIndex(m_currentColorsTurn)].restart();
    }

    Chess::select_piece_return_type Chess::selectPiece(position_t x, position_t y) const
    {
        auto piece = getBoardPiece(x, y);
        if(!piece.isEmpty()) {
            if(m_disableOppositeColorHint) {
                if(piece.getColor() == m_currentColorsTurn) {
                    std::vector<Position> ret = getValidMoves(piece, x, y);
                    return select_piece_return_type(PieceInformation(piece, x, y), ret);
                }
            } else {
                std::vector<Position> ret = getValidMoves(piece, x, y);
                return select_piece_return_type(PieceInformation(piece, x, y), ret);
            }
        }

        return select_piece_return_type(PieceInformation(), std::vector<Position>());
    }

    Chess::select_piece_return_type Chess::selectPiece(const std::string& pos) const
    {
        auto ipos = convertStringPositionToInt(pos);
        auto x = ipos.first;
        auto y = ipos.second;
        if(x >= 0 && y >= 0)
            return selectPiece(x, y);

        return std::make_pair(PieceInformation(), std::vector<Position>());
    }

    Chess::EMoveResult Chess::move(position_t xs, position_t ys, position_t xd, position_t yd)
    {
        if(!isWaitingForPromotion()) {
            // can only move if there is no promotion waiting
            const auto fromPos = convert2Dto1DPosition(xs, ys, BOARD_WIDTH);
            const auto toPos = convert2Dto1DPosition(xd, yd, BOARD_WIDTH);
            if(isPositionValid(fromPos) && isPositionValid(toPos) && (xs != xd || ys != yd)) {
                const auto fromPiece = m_board[fromPos];
                const auto toPiece = m_board[toPos];

                if(!fromPiece.isEmpty() && fromPiece.getColor() == m_currentColorsTurn) {
                    const auto fromColor = fromPiece.getColor();
                    const auto fromColorIndex = getColorIndex(fromColor);
                    const auto& lastEnPassantPosition = m_enPassant[fromColorIndex];

                    if(toPiece.isEmpty()) {
                        if(fromPiece.getType() == Piece::Type::PAWN) {
                            if(movePawn(fromPiece, fromColor, fromColorIndex, xs, ys, xd, yd))
                                return EMoveResult::Move;
                        } else {
                            const auto moves = isMoveValid(fromPiece, xs, ys, xd, yd, *this);
                            if(tryMove(moves)) {
                                for(const auto& t : moves) {
                                    auto tp = std::get<0>(t);
                                    auto txs = std::get<1>(t);
                                    auto tys = std::get<2>(t);
                                    auto txd = std::get<3>(t);
                                    auto tyd = std::get<4>(t);
                                    m_boardStateManager.updateStateOnMove(tp, txs, tys, txd, tyd);
                                }
                                m_boardHistoryManager.addMoveToHistory(moves);
                                m_boardHistoryManager.addLastEnPassant(fromColor, lastEnPassantPosition.x, lastEnPassantPosition.y);
                                m_enPassant[fromColorIndex] = Position(-1, -1);
                                m_currentColorsTurn = getOppositeColor(m_currentColorsTurn);
                                return EMoveResult::Move;
                            }
                        }
                    } else {
                        const auto toColor = toPiece.getColor();
                        const auto toColorIndex = getColorIndex(toColor);

                        if(fromColor != toColor) {
                            const auto captures = isCaptureValid(fromPiece, xs, ys, xd, yd, *this);
                            if(std::get<0>(captures)) {
                                auto xc = std::get<1>(captures);
                                auto yc = std::get<2>(captures);
                                auto captureResult = tryCapture(fromPiece, xs, ys, xd, yd, xc, yc);
                                if(captureResult.first) {
                                    auto pieceCaptured = captureResult.second;
                                    m_deadPieces[toColorIndex].push_back(PieceInformation(pieceCaptured, xc, yc));
                                    m_boardStateManager.updateStateOnCapture(fromPiece, pieceCaptured, xs, ys, xd, yd, xc, yc);
                                    m_boardHistoryManager.addCaptureToHistory(toColor, fromPiece.hasMoved(), xs, ys, xd, yd);
                                    m_boardHistoryManager.addLastEnPassant(fromColor, lastEnPassantPosition.x, lastEnPassantPosition.y);
                                    m_enPassant[fromColorIndex] = Position(-1, -1);

                                    if(fromPiece.getType() == Piece::Type::PAWN)
                                        checkPawnPromotion(xd, yd);

                                    m_currentColorsTurn = getOppositeColor(m_currentColorsTurn);
                                    return EMoveResult::Capture;
                                }
                            }
                        }
                    }
                }
            }
        }

        return EMoveResult::Invalid;
    }

    Chess::EMoveResult Chess::move(const std::string& from, const std::string& to)
    {
        auto fpos = convertStringPositionToInt(from);
        auto xs = fpos.first;
        auto ys = fpos.second;

        if(to.size() > 0) {
            auto tpos = convertStringPositionToInt(to);
            auto xd = tpos.first;
            auto yd = tpos.second;
            if(xs >= 0 && ys >= 0 && xd >= 0 && yd >= 0)
                return move(xs, ys, xd, yd);
        } else {
            auto piece = getBoardPiece(xs, ys);
            if(piece.getType() == Piece::Type::PAWN) {
                auto pawnMoveDirection = getPawnDirection(piece.getColor());
                return move(xs, ys, xs, ys + pawnMoveDirection);
            }
        }

        return EMoveResult::Invalid;
    }

    bool Chess::undo()
    {
        assert((m_boardStateManager.isThereStateToUndo() && m_boardHistoryManager.isThereHistoryToUndo()) ||
               (!m_boardStateManager.isThereStateToUndo() && !m_boardHistoryManager.isThereHistoryToUndo()));
        if(!isWaitingForPromotion()) {
            if(m_boardStateManager.isThereStateToUndo() && m_boardHistoryManager.isThereHistoryToUndo()) {
                m_boardStateManager.undoLastState();
                m_boardHistoryManager.undoLastMove(*this);
                m_currentColorsTurn = getOppositeColor(m_currentColorsTurn);

                return true;
            }
        }

        return false;
    }

    bool Chess::isOnCheck(Piece::eColor color) const
    {
        const auto& kingInformation = getKing(color);
        const auto& kingPosition = kingInformation.getPosition();
        return isCheck(kingInformation, kingPosition.x, kingPosition.y, *this).size();
    }

    bool Chess::isCheckMate(Piece::eColor color) const
    {
        // a checkmate occurs when the player's king is on check,
        // and the player's pieces have no valid moves
        if(isOnCheck(color)) {
            for(const auto& pieceInformation : getAlivePieces(color)) {
                const auto& piecePosition = pieceInformation.getPosition();
                if(getValidMoves(pieceInformation.getPiece(), piecePosition.x, piecePosition.y).size())
                    return false;
            }
            const auto& king = getKing(color);
            const auto& kingPosition = king.getPosition();
            if(getValidMoves(king.getPiece(), kingPosition.x, kingPosition.y).size())
                return false;

            return true;
        }

        return false;
    }

    bool Chess::isStaleMate(Piece::eColor color) const
    {
        // stalemate occurs when the player only has a king left, and player's king is not on check
        // but the player's king have 0 available move
        if(!isOnCheck(color) && !getAlivePieces(color).size()) {
            const auto& king = getKing(color);
            const auto& kingPosition = king.getPosition();
            if(!getValidMoves(king.getPiece(), kingPosition.x, kingPosition.y).size())
                return true;
        }

        return false;
    }

    void Chess::setTypeToPromoteTo(Piece::Type type)
    {
        if(isWaitingForPromotion()) {
            const auto& promotionPosition = m_positionForPromotion;
            auto promPos = convert2Dto1DPosition(promotionPosition.x, promotionPosition.y, BOARD_WIDTH);
            // set the pieces
            auto pieceToPromoteFrom = m_board[promPos];
            auto pieceToPromoteTo = Piece(type, 9, m_colorWaitingForPromotion); // TODO: add ids for promotable types

            // find the piece from alive pieces, and update it's piece information
            for(auto& pieceInformation : m_alivePieces[getColorIndex(m_colorWaitingForPromotion)]) {
                if(pieceInformation.getPiece() == pieceToPromoteFrom) {
                    pieceInformation.m_piece = pieceToPromoteTo;
                }
            }

            // update the piece to promoted type
            m_board[promPos] = pieceToPromoteTo;
            m_isWaitingForPromotion = false;
        }
    }

    Piece Chess::getBoardPiece(position_t x, position_t y) const
    {
        auto pos = convert2Dto1DPosition(x, y, BOARD_WIDTH);
        if(isPositionValid(pos))
            return m_board[pos];

        return Piece();
    }

    std::vector<Chess::Position> Chess::getValidMoves(position_t x, position_t y) const
    {
        auto piece = getBoardPiece(x, y);
        if(!piece.isEmpty())
            return getValidMoves(piece, x, y);

        return {};
    }

    std::string Chess::getBoardString() const
    {
        static const char INT_TO_CHAR[] =
        {
            'a',
            'b',
            'c',
            'd',
            'e',
            'f',
            'g',
            'h'
        };

        static const std::string Y_BORDER = "  " + std::string((BOARD_WIDTH * 2) - 1, '-') + '\n';
        const auto iwidth = static_cast<position_t>(BOARD_WIDTH);
        const auto iheight = static_cast<position_t>(BOARD_HEIGHT);
        std::string ret = "  ";

        // add the letters
        for(position_t i = 0; i < iwidth; ++i) {
            ret += INT_TO_CHAR[i];
            ret += ' ';
        }
        ret += '\n';
        ret += Y_BORDER;

        for(position_t y = 0; y < iheight; ++y) {
            std::string rankString;
            if(m_bottomColor == Piece::eColor::white)
                rankString = std::to_string((iheight - y));
            else
                rankString = std::to_string(y + 1);
            ret += rankString + "|";
            for(position_t x = 0; x < iwidth; ++x) {
                auto piece = getBoardPiece(x, y);
                char pieceType = piece.getColor() == Piece::eColor::white ?
                            static_cast<char>(toupper(getCharacterOfPiece(piece))) :
                            static_cast<char>(tolower(getCharacterOfPiece(piece)));
                ret += pieceType;
                ret += '|';
            }
            ret += '\n';
            ret += Y_BORDER;
        }

        return ret;
    }

    std::string Chess::getBoardStringPieces() const
    {
        const auto iwidth = static_cast<position_t>(BOARD_WIDTH);
        const auto iheight = static_cast<position_t>(BOARD_HEIGHT);

        std::string ret;
        for(position_t y = 0; y < iheight; ++y) {
            for(position_t x = 0; x < iwidth; ++x) {
                auto piece = getBoardPiece(x, y);
                char pieceType = piece.getColor() == Piece::eColor::white ?
                            static_cast<char>(toupper(getCharacterOfPiece(piece))) :
                            static_cast<char>(tolower(getCharacterOfPiece(piece)));

                ret += pieceType;
            }
        }

        return ret;
    }

    bool Chess::isPositionValid(position_t pos) const
    {
        return pos >= 0 && pos < static_cast<position_t>(m_board.capacity());
    }

    void Chess::initPieces(Piece::eColor color)
    {
        static constexpr position_t PAWN_STARTING_Y_POS[] = { 6, 1 };
        static constexpr position_t OTHER_STARTING_Y_POS[] = { 7, 0 };
        static const std::pair<position_t, position_t> QUEEN_KING_X_POS[] = { { 3, 4 }, { 4, 3 } };

        // determine which pieces are on the bottom
        std::size_t positionIndex = (color == m_bottomColor ? 0 : 1);

        auto pawnStartingYPos = PAWN_STARTING_Y_POS[positionIndex];
        for(auto i = 0; i < static_cast<position_t>(BOARD_WIDTH); ++i)
            addPiece(Piece::Type::PAWN, i, color, i, pawnStartingYPos);

        auto otherStartingYPos = OTHER_STARTING_Y_POS[positionIndex];
        addPiece(Piece::Type::KNIGHT, 0, color, 1, otherStartingYPos);
        addPiece(Piece::Type::KNIGHT, 1, color, 6, otherStartingYPos);
        addPiece(Piece::Type::BISHOP, 0, color, 2, otherStartingYPos);
        addPiece(Piece::Type::BISHOP, 1, color, 5, otherStartingYPos);
        addPiece(Piece::Type::ROOK, 0, color, 0, otherStartingYPos);
        addPiece(Piece::Type::ROOK, 1, color, 7, otherStartingYPos);

        const auto& queenKingXPos = QUEEN_KING_X_POS[getColorIndex(m_bottomColor)];
        addPiece(Piece::Type::QUEEN, 0, color, queenKingXPos.first, otherStartingYPos);
        addPiece(Piece::Type::KING, 0, color, queenKingXPos.second, otherStartingYPos, true);
    }

    void Chess::addPiece(Piece::Type type, unsigned char id, Piece::eColor color, position_t x, position_t y, bool isKing)
    {
        assert(type == Piece::Type::PAWN ||
               type == Piece::Type::KNIGHT ||
               type == Piece::Type::BISHOP ||
               type == Piece::Type::ROOK ||
               type == Piece::Type::QUEEN ||
               type == Piece::Type::KING);

        auto pos = convert2Dto1DPosition(x, y, BOARD_WIDTH);
        assert(isPositionValid(pos));
        if(isPositionValid(pos)) {
            m_board[pos] = Piece(type, id, color);

            if(isKing)
                m_king[getColorIndex(color)] = PieceInformation(Piece(type, id, color), x, y);
            else
                m_alivePieces[getColorIndex(color)].emplace_back(Piece(type, id, color), x, y);
        }
    }

    void Chess::updatePositionIfKing(Piece piece, Piece::eColor color, position_t x, position_t y) const
    {
        if(piece.getType() == Piece::Type::KING)
            updateKingPosition(color, x, y);
    }

    void Chess::updateKingPosition(Piece::eColor color, position_t x, position_t y) const
    {
        m_king[getColorIndex(color)].m_position = Position(x, y);
    }

    bool Chess::movePawn(Piece piece, Piece::eColor fromColor, Piece::color_index_type fromColorIndex, position_t xs, position_t ys, position_t xd, position_t yd)
    {
        const auto& lastEnPassantPosition = m_enPassant[fromColorIndex];
        if(yd == ys + 2 || yd == ys - 2) {
            const auto moves = isMoveValid(piece, xs, ys, xd, yd, *this);
            if(tryMove(moves)) {
                for(const auto& t : moves) {
                    auto tp = std::get<0>(t);
                    auto txs = std::get<1>(t);
                    auto tys = std::get<2>(t);
                    auto txd = std::get<3>(t);
                    auto tyd = std::get<4>(t);
                    m_boardStateManager.updateStateOnMove(tp, txs, tys, txd, tyd);
                }
                m_boardHistoryManager.addMoveToHistory(moves);
                m_boardHistoryManager.addLastEnPassant(fromColor, lastEnPassantPosition.x, lastEnPassantPosition.y);
                m_enPassant[fromColorIndex] = Position(xd, yd);

                checkPawnPromotion(xd, yd);
                m_currentColorsTurn = getOppositeColor(m_currentColorsTurn);
                return true;
            }
        } else {
            const auto moves = isMoveValid(piece, xs, ys, xd, yd, *this);
            if(tryMove(moves)) {
                for(const auto& t : moves) {
                    auto tp = std::get<0>(t);
                    auto txs = std::get<1>(t);
                    auto tys = std::get<2>(t);
                    auto txd = std::get<3>(t);
                    auto tyd = std::get<4>(t);
                    m_boardStateManager.updateStateOnMove(tp, txs, tys, txd, tyd);
                }
                m_boardHistoryManager.addMoveToHistory(moves);
                m_boardHistoryManager.addLastEnPassant(fromColor, lastEnPassantPosition.x, lastEnPassantPosition.y);
                m_enPassant[fromColorIndex] = Position(-1, -1);

                checkPawnPromotion(xd, yd);
                m_currentColorsTurn = getOppositeColor(m_currentColorsTurn);
                return true;
            } else {
                const auto captures = isPawnSpecialCaptureValid(piece, xs, ys, xd, yd, *this);
                if(std::get<0>(captures)) {
                    auto xc = std::get<1>(captures);
                    auto yc = std::get<2>(captures);
                    auto captureResult = tryCapture(piece, xs, ys, xd, yd, xc, yc);
                    if(captureResult.first) {
                        auto pieceCaptured = captureResult.second;
                        auto toColor = pieceCaptured.getColor();
                        m_deadPieces[getColorIndex(toColor)].push_back(PieceInformation(pieceCaptured, xc, yc));
                        m_boardStateManager.updateStateOnCapture(piece, pieceCaptured, xs, ys, xd, yd, xc, yc);
                        m_boardHistoryManager.addCaptureToHistory(toColor, piece.hasMoved(), xs, ys, xd, yd);
                        m_boardHistoryManager.addLastEnPassant(fromColor, lastEnPassantPosition.x, lastEnPassantPosition.y);
                        m_enPassant[fromColorIndex] = Position(-1, -1);

                        checkPawnPromotion(xd, yd);
                        m_currentColorsTurn = getOppositeColor(m_currentColorsTurn);
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void Chess::checkPawnPromotion(position_t x, position_t y)
    {
        if(y == RANK_1 || y == RANK_8) {
            auto pieceToPromote = m_board[convert2Dto1DPosition(x, y, BOARD_WIDTH)];

            m_isWaitingForPromotion = true;
            m_colorWaitingForPromotion = pieceToPromote.getColor();
            m_positionForPromotion = Position(x, y);

            m_boardHistoryManager.addPromotionToHistory(pieceToPromote, x, y);
        }
    }

    std::vector<Chess::PieceInformation> Chess::copyAlivePiecesExcept(Piece piece, Piece::eColor color) const
    {
        std::vector<PieceInformation> ret;
        for(const auto& alivePieceInformation : getAlivePieces(color)) {
            if(piece != alivePieceInformation.getPiece())
                ret.push_back(alivePieceInformation);
        }

        return ret;
    }

    Piece Chess::simulateMove(position_t fromPos, position_t toPos, position_t xd, position_t yd) const
    {
        std::swap(m_board[fromPos], m_board[toPos]);
        assert(m_board[fromPos].isEmpty());

        return m_board[toPos];
    }

    void Chess::reverseMove(position_t fromPos, position_t toPos, position_t xs, position_t ys) const
    {
        std::swap(m_board[toPos], m_board[fromPos]);
        assert(m_board[toPos].isEmpty());
    }

    void Chess::finalizeMove(const PieceInformation& movedPieceInformation, Piece::eColor color)
    {
        // set the moved flag on board
        const auto& movedPiecePosition = movedPieceInformation.getPosition();
        auto pos = convert2Dto1DPosition(movedPiecePosition.x, movedPiecePosition.y, BOARD_WIDTH);
        m_board[pos].setMovedFlag();

        // update position on alive pieces, and update the moved flag
        if(movedPieceInformation.getPiece().getType() == Piece::Type::KING) {
            auto& king = m_king[getColorIndex(color)];
//            king = movedPieceInformation;
            king.m_piece.setMovedFlag();
        } else {
            for(auto& pieceInformation : m_alivePieces[getColorIndex(color)]) {
                if(movedPieceInformation.getPiece() == pieceInformation.getPiece()) {
                    pieceInformation = movedPieceInformation;
                    pieceInformation.m_piece.setMovedFlag();
                    break;
                }
            }
        }
    }

    // returns
    // first - piece that moved
    // second - piece that has been captured
    std::pair<Piece, Piece> Chess::simulateCapture(position_t fromPos, position_t toPos, position_t capturePos, position_t xd, position_t yd) const
    {
        std::pair<Piece, Piece> ret;
        assert(!m_board[capturePos].isEmpty());
        assert(!m_board[fromPos].isEmpty());

        std::swap(ret.second, m_board[capturePos]);
        std::swap(m_board[fromPos], m_board[toPos]);
        assert(m_board[fromPos].isEmpty());
        ret.first = m_board[toPos];

        return ret;
    }

    void Chess::reverseCapture(position_t fromPos, position_t toPos, position_t capturePos, position_t xs, position_t ys, Piece& temporary) const
    {
        std::swap(m_board[toPos], m_board[fromPos]);
        std::swap(m_board[capturePos], temporary);
    }

    void Chess::removePieceFormAlivePieces(Piece piece)
    {
        auto color = piece.getColor();
        auto& alivePieces = m_alivePieces[getColorIndex(color)];
        for(std::size_t  i = 0, i_size = alivePieces.size(); i < i_size; ++i) {
            const auto& alivePiece = alivePieces[i];
            if(alivePiece.getPiece() == piece) {
                std::swap(alivePieces[i], alivePieces.back());
                alivePieces.pop_back();
                i_size = alivePieces.size();
                break;
            }
        }
    }

    bool Chess::tryMove(const std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>>&  moves)
    {
        if(moves.size()) {
            auto color = std::get<0>(moves[0]).getColor();
            std::vector<PieceInformation> piecesMoved;

            for(const auto& t : moves) {
                auto piece = std::get<0>(t);
                auto xs = std::get<1>(t);
                auto ys = std::get<2>(t);
                auto xd = std::get<3>(t);
                auto yd = std::get<4>(t);

                auto fromPos = convert2Dto1DPosition(xs, ys, BOARD_WIDTH);
                auto toPos = convert2Dto1DPosition(xd, yd, BOARD_WIDTH);

                auto pieceMoved = simulateMove(fromPos, toPos, xd, yd);
//                if(!pieceMoved.isEmpty())
                assert(!pieceMoved.isEmpty());
                piecesMoved.emplace_back(pieceMoved, xd, yd);
                updatePositionIfKing(piece, color, xd, yd);
            }

            const auto& king = getKing(color);
            const auto& kingPosition = king.getPosition();
            auto piecesThatCheck = isCheck(king, kingPosition.x, kingPosition.y, *this);
            if(piecesThatCheck.size()) {
                for(auto it = moves.rbegin(), it_end = moves.rend(); it != it_end; ++it) {
                    const auto& t = *it;
                    auto piece = std::get<0>(t);
                    auto xs = std::get<1>(t);
                    auto ys = std::get<2>(t);
                    auto xd = std::get<3>(t);
                    auto yd = std::get<4>(t);

                    auto fromPos = convert2Dto1DPosition(xs, ys, BOARD_WIDTH);
                    auto toPos = convert2Dto1DPosition(xd, yd, BOARD_WIDTH);

                    reverseMove(fromPos, toPos, xs, ys);
                    updatePositionIfKing(piece, color, xs, ys);
                }

                return false;
            }

            for(const auto& movedPieceInformation : piecesMoved) {
                finalizeMove(movedPieceInformation, color);
            }

            return true;
        }

        return false;
    }

    std::pair<bool, Piece> Chess::tryCapture(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, position_t xc, position_t yc)
    {
        auto color = piece.getColor();
        auto oppositeColor = getOppositeColor(color);
        auto fromPos = convert2Dto1DPosition(xs, ys, BOARD_WIDTH);
        auto toPos = convert2Dto1DPosition(xd, yd, BOARD_WIDTH);
        auto capturePos = convert2Dto1DPosition(xc, yc, BOARD_WIDTH);

        assert(!m_board[capturePos].isEmpty());
        assert(!m_board[fromPos].isEmpty());

        auto temporaryToRemove = m_board[capturePos];
        auto temporaryAlivePieces = copyAlivePiecesExcept(temporaryToRemove, oppositeColor);
        updatePositionIfKing(piece, color, xd, yd);

        auto simulatedCaptureResult = simulateCapture(fromPos, toPos, capturePos, xd, yd);
        const auto& king = getKing(color);
        const auto& kingPosition = king.getPosition();
        auto piecesThatCheck = isCheck(kingPosition.x, kingPosition.y, *this, temporaryAlivePieces);
        if(piecesThatCheck.size()) {
            reverseCapture(fromPos, toPos, capturePos, xs, ys, simulatedCaptureResult.second);
            updatePositionIfKing(piece, color, xs, ys);

            return { false, simulatedCaptureResult.second };
        }

        // remove the capture piece from the alive pieces
        removePieceFormAlivePieces(simulatedCaptureResult.second);

        // update the moved piece
        finalizeMove(PieceInformation(simulatedCaptureResult.first, xd, yd), color);

        return { true, simulatedCaptureResult.second };
    }

    bool Chess::tryTemporaryMove(const std::vector<std::tuple<Piece, position_t, position_t, position_t, position_t>>& moves) const
    {
        bool ret = true;
        if(moves.size()) {
            auto color = std::get<0>(moves[0]).getColor();
            for(const auto& t : moves) {
                auto piece = std::get<0>(t);
                auto xs = std::get<1>(t);
                auto ys = std::get<2>(t);
                auto xd = std::get<3>(t);
                auto yd = std::get<4>(t);

                auto fromPos = convert2Dto1DPosition(xs, ys, BOARD_WIDTH);
                auto toPos = convert2Dto1DPosition(xd, yd, BOARD_WIDTH);

                simulateMove(fromPos, toPos, xd, yd);
                updatePositionIfKing(piece, color, xd, yd);
            }

            if(isOnCheck(color))
                ret = false;

            for(auto it = moves.rbegin(), it_end = moves.rend(); it != it_end; ++it) {
                const auto& t = *it;
                auto piece = std::get<0>(t);
                auto xs = std::get<1>(t);
                auto ys = std::get<2>(t);
                auto xd = std::get<3>(t);
                auto yd = std::get<4>(t);

                auto fromPos = convert2Dto1DPosition(xs, ys, BOARD_WIDTH);
                auto toPos = convert2Dto1DPosition(xd, yd, BOARD_WIDTH);

                reverseMove(fromPos, toPos, xs, ys);
                updatePositionIfKing(piece, color, xs, ys);
            }

            return ret;
        }

        return false;
    }

    bool Chess::tryTemporaryCapture(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, position_t xc, position_t yc) const
    {
        bool ret = true;
        auto color = piece.getColor();
        auto oppositeColor = getOppositeColor(color);
        auto fromPos = convert2Dto1DPosition(xs, ys, BOARD_WIDTH);
        auto toPos = convert2Dto1DPosition(xd, yd, BOARD_WIDTH);
        auto capturePos = convert2Dto1DPosition(xc, yc, BOARD_WIDTH);

        assert(!m_board[capturePos].isEmpty());
        assert(!m_board[fromPos].isEmpty());
        auto temporaryToRemove = m_board[capturePos];
        auto temporaryAlivePieces = copyAlivePiecesExcept(temporaryToRemove, oppositeColor);
        updatePositionIfKing(piece, color, xd, yd);

        // simulate the capture
        auto simulatedCaptureResult = simulateCapture(fromPos, toPos, capturePos, xd, yd);
        const auto& king = getKing(color);
        const auto& kingPosition = king.getPosition();
        if(isCheck(kingPosition.x, kingPosition.y, *this, temporaryAlivePieces).size())
            ret = false;

        reverseCapture(fromPos, toPos, capturePos, xs, ys, simulatedCaptureResult.second);
        updatePositionIfKing(piece, color, xs, ys);

        return ret;
    }

    std::vector<Chess::Position> Chess::getValidMoves(Piece piece, position_t x, position_t y) const
    {
        std::vector<Chess::Position> ret;
        ret.reserve(100);

        for(position_t j = 0; j < static_cast<position_t>(BOARD_HEIGHT); ++j) {
            for(position_t i = 0; i < static_cast<position_t>(BOARD_WIDTH); ++i) {
                if(x != i || y != j) {
                    Piece dstPiece = getBoardPiece(i, j);
                    if(dstPiece.isEmpty()) {
                        auto moves = isMoveValid(piece, x, y, i, j, *this);
                        if(tryTemporaryMove(moves))
                            ret.emplace_back(i, j);

                        if(piece.getType() == Piece::Type::PAWN) {
                            auto captureResult = isPawnSpecialCaptureValid(piece, x, y, i, j, *this);
                            if(std::get<0>(captureResult))
                                ret.emplace_back(i, j);
                        }
                    } else {
                        if(piece.getColor() != dstPiece.getColor()) {
                            auto captures = isCaptureValid(piece, x, y, i, j, *this);
                            if(std::get<0>(captures) && tryTemporaryCapture(piece, x, y, i, j, std::get<1>(captures), std::get<2>(captures)))
                                ret.emplace_back(i, j);
                        }
                    }
                }
            }
        }

        return ret;
    }

    bool operator==(const Chess::PieceInformation& lhs, const Chess::PieceInformation& rhs)
    {
        return lhs.m_piece == rhs.m_piece && lhs.m_position == rhs.m_position;
    }

    bool operator!=(const Chess::PieceInformation& lhs, const Chess::PieceInformation& rhs)
    {
        return !(lhs == rhs);
    }

    bool operator==(const Chess::Position& lhs, const Chess::Position& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    bool operator!=(const Chess::Position& lhs, const Chess::Position& rhs)
    {
        return !(lhs == rhs);
    }

    position_t convert2Dto1DPosition(position_t x, position_t y, position_t width)
    {
        return x * width + y;
    }

    std::pair<position_t, position_t> convertStringPositionToInt(const std::string& pos)
    {
        static const std::unordered_map<char, position_t> CHAR_TO_INT =
        {
            { 'a', 0 },
            { 'b', 1 },
            { 'c', 2 },
            { 'd', 3 },
            { 'e', 4 },
            { 'f', 5 },
            { 'g', 6 },
            { 'h', 7 }
        };

        static const std::unordered_map<char, position_t> INT_TO_INT =
        {
            { '1', 7 },
            { '2', 6 },
            { '3', 5 },
            { '4', 4 },
            { '5', 3 },
            { '6', 2 },
            { '7', 1 },
            { '8', 0 },
        };

        assert(pos.size() == 2);
        if(pos.size() == 2) {
            auto cit = CHAR_TO_INT.find(pos[0]);
            auto iit = INT_TO_INT.find(pos[1]);

            if(cit != CHAR_TO_INT.end() && iit != INT_TO_INT.end())
                return std::make_pair((*cit).second, (*iit).second);

            return std::make_pair(-1, -1);
        }

        return std::make_pair(-1, -1);
    }

    std::vector<Chess::PieceInformation> isCheck(const Chess::PieceInformation& king, position_t xd, position_t yd, const Chess& chessboard)
    {
        std::vector<Chess::PieceInformation> ret;
        assert(king.getPiece().getType() == Piece::Type::KING);
        auto oppositeColor = getOppositeColor(king.getPiece().getColor());
        for(const auto& pieceInformation : chessboard.getAlivePieces(oppositeColor)) {
            auto piece = pieceInformation.getPiece();
            const auto& piecePosition = pieceInformation.getPosition();
            if(std::get<0>(isCaptureValid(piece, piecePosition.x, piecePosition.y, xd, yd, chessboard)))
                ret.push_back(pieceInformation);
        }

        return ret;
    }

    std::vector<Chess::PieceInformation> isCheck(position_t xd, position_t yd, const Chess& chessboard, const std::vector<Chess::PieceInformation>& alivePieces)
    {
        std::vector<Chess::PieceInformation> ret;

        for(const auto& pieceInformation : alivePieces) {
            auto piece = pieceInformation.getPiece();
            const auto& piecePosition = pieceInformation.getPosition();
            if(std::get<0>(isCaptureValid(piece, piecePosition.x, piecePosition.y, xd, yd, chessboard)))
                ret.push_back(pieceInformation);
        }

        return ret;
    }
}
