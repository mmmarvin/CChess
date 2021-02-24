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
#include <iostream>
#include "../chess/chess.h"
#include "../define.h"
#include "displayserver.h"

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using namespace std;

namespace cchess_display
{
namespace
{
    tcp::socket createDisplayServer(boost::asio::io_context& ioc,
                                    int port)
    {
        tcp::acceptor a(ioc, tcp::endpoint(address::from_string("127.0.0.1"), port));
        a.listen();

        // block until display client connects
        return a.accept();
    }

    int charToInt(char c)
    {
        int ret;

        stringstream ss;
        ss.str(string(&c, 1));
        ss >> ret;

        return ret;
    }
}
    static const char HS_HELLO_MSG[3] = { 0x1A, 0x1D, '\0' };
    static const char HS_HI_MSG[3] = { 0x1C, 0x1A, '\0' };
    static const char HS_HI_ACCEPT_MSG[3] = { 0x21, 0x2D, '\0' };

    DisplayServer::DisplayServer(int port) :
        m_socket(createDisplayServer(m_ioc, port)),
        m_started(false)
    {
        initiateHandshake();
    }

    tuple<array<char, 255>, boost::system::error_code, size_t> DisplayServer::read()
    {
        std::array<char, 255> buffer;

        boost::system::error_code error;
        size_t length = m_socket.read_some(boost::asio::buffer(&buffer[0], 255), error);

        return make_tuple(buffer, error, length);
    }

    void DisplayServer::write(const char* buff, std::size_t length)
    {
        boost::asio::write(m_socket, boost::asio::buffer(buff, length));
    }

    void DisplayServer::run()
    {
        cchess::Chess chessBoard;
        chessBoard.resetBoard(cchess::Piece::eColor::white, true);
        cchess::Chess::PieceInformation selectedPiece = cchess::Chess::PieceInformation();

        while(m_started) {
            chessBoard.update();

            auto r = read();
            auto l = std::get<2>(r);
            auto b = string(&std::get<0>(r)[0], l);
            auto e = std::get<1>(r);

            cout << b << ' ' << b.size() << endl;

            if(e == boost::asio::error::eof)
                break;

            if(b.empty())
                continue;

            if(b[0] == 'e') {
                break;
            } else if(b[0] == 'u') {
                auto wt = static_cast<int32_t>(chessBoard.getTimeLeft(cchess::Piece::eColor::white));
                auto bt = static_cast<int32_t>(chessBoard.getTimeLeft(cchess::Piece::eColor::black));

                string t = to_string(wt);
                t += " ";
                t += to_string(bt);

                write(t.data(), t.size());
            } else if(b[0] == 'd') {
                auto boardStringPieces = chessBoard.getBoardStringPieces();
                write(boardStringPieces.data(), boardStringPieces.size());
            } else if(b[0] == 'r') {
                if(b == "rt") {
                    chessBoard.resetBoard(cchess::Piece::eColor::white, true);
                    write("ok", 2);
                } else if(b == "rn") {
                    chessBoard.resetBoard(cchess::Piece::eColor::white);
                    write("ok", 2);
                }
            } else if(b[0] == 'l') {
                auto whiteDead = chessBoard.getDeadPieces(cchess::Piece::eColor::white);
                auto blackDead = chessBoard.getDeadPieces(cchess::Piece::eColor::black);

                string whiteDeadStr = "s";
                for(const auto& white : whiteDead)
                    whiteDeadStr += static_cast<char>(white.getPiece().getType());

                string blackDeadStr = "s";
                for(const auto& black : blackDead)
                    blackDeadStr += tolower(static_cast<char>(black.getPiece().getType()));

                auto dead = whiteDeadStr + blackDeadStr;
                write(dead.data(), dead.size());
            } else if(b.size() == 4 && b[0] == 'p' && b[1] == 'r') {
                if(chessBoard.isWaitingForPromotion()) {
                    auto p = b[3];
                    if(p == 'Q' || p == 'R' || p == 'B' || p == 'N') {
                        chessBoard.setTypeToPromoteTo(static_cast<cchess::Piece::Type>(p));
                        write("ok", 2);
                    } else
                        write("no", 2);
                } else
                    write("np", 2);
            } else if(b.size() == 5 && b[0] == 's') {
                int x = charToInt(b[2]);
                int y = charToInt(b[4]);

                auto p = chessBoard.selectPiece(x, y);
                if(selectedPiece == p.first) {
                    selectedPiece = cchess::Chess::PieceInformation();
                    write("ca", 2);
                } else {
                    if(selectedPiece != cchess::Chess::PieceInformation()) {
                        auto from = selectedPiece.getPosition();

                        auto r = chessBoard.move(from.x, from.y, x, y);
                        switch(r) {
                        case cchess::Chess::EMoveResult::Move:
                            if(chessBoard.isWaitingForPromotion()) {
                                write("okw", 3);
                            } else
                                write("ok", 2);
                            chessBoard.resetTimer();
                            break;
                        case cchess::Chess::EMoveResult::Capture:
                            if(chessBoard.isWaitingForPromotion()) {
                                write("cow", 3);
                            } else
                                write("co", 2);
                            chessBoard.resetTimer();
                            break;
                        case cchess::Chess::EMoveResult::Invalid:
                            write("00", 2);
                            break;
                        }

                        selectedPiece = cchess::Chess::PieceInformation();
                    } else {
                        selectedPiece = p.first;
                        write("pm", 2);
                    }
                }
            }
        }
    }

    void DisplayServer::initiateHandshake()
    {
        // first, read a hello connection from client
        auto r = read();
        auto b = std::get<0>(r);
        auto e = std::get<1>(r);

        if(!e && !strncmp(&b[0], &HS_HELLO_MSG[0], 2)) {
            // second, send a hi connection to client
            write(&HS_HI_MSG[0], 3);

            // read the hi confirmation
            r = read();
            b = std::get<0>(r);
            e = std::get<1>(r);

            if(!e && !strncmp(&b[0], &HS_HI_ACCEPT_MSG[0], 2)) {
                string gameVersion = "";
                gameVersion += to_string(cchess::GAME_VERSION_MAJOR);
                gameVersion += ".";
                gameVersion += to_string(cchess::GAME_VERSION_MINOR);
                gameVersion += to_string(cchess::GAME_VERSION_PATCH);

                write(gameVersion.data(), gameVersion.size());
                m_started = true;
            }
        }
    }
}
