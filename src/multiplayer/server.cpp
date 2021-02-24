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
#include "server.h"
using boost::asio::ip::tcp;
using boost::asio::ip::address_v4;
using namespace std;

namespace cchess_multiplayer
{
    Client::Client(boost::asio::ip::tcp::socket socket) :
        m_socket(std::move(socket))
    {
    }

    ServerRoom::ServerRoom(boost::asio::ip::tcp::acceptor& acceptor) :
        m_owner(nullptr),
        m_whitePlayer(nullptr),
        m_blackPlayer(nullptr),
        m_acceptor(&acceptor)
    {
    }

    void ServerRoom::join(std::shared_ptr<Client> client)
    {
        m_clients.emplace(client);

        if(!m_owner)
            m_owner = client;

        startReading(client);
    }

    void ServerRoom::leave(std::shared_ptr<Client> client)
    {
        m_clients.erase(client);
        if(m_owner == client) {
            m_clients.clear();

            if(m_acceptor->is_open())
                m_acceptor->close();
        } else if(m_clients.empty()) {
            if(m_acceptor->is_open())
                m_acceptor->close();
        }
    }

    void ServerRoom::sendToAll(std::string msg)
    {
        for(auto& client : m_clients)
            this->writeCommand(client, msg);
    }

    void ServerRoom::startReading(std::shared_ptr<Client> client)
    {
        boost::asio::async_read(client->m_socket, boost::asio::buffer(&client->m_message[0], 255), [this, client]
        (boost::system::error_code error, std::size_t length) {
            if(!error) {
                string cmd = string(&client->m_message[0], length);

                if(client == m_owner) {
                    if(!readOwnerCommands(client, cmd)) {
                        if(client == m_whitePlayer ||
                           client == m_blackPlayer)
                            readPlayerCommands(client, cmd);
                        else
                            readClientCommands(client, cmd);
                    }
                } else if(client == m_whitePlayer || client == m_blackPlayer) {
                    if(!readPlayerCommands(client, cmd))
                        readClientCommands(client, cmd);
                } else
                    readClientCommands(client, cmd);
            }

            this->startReading(client);
        });
    }

    bool ServerRoom::readOwnerCommands(const std::shared_ptr<Client>& client, const std::string& cmd)
    {
        if(cmd == "start") {
            return true;
        }

        return false;
    }

    bool ServerRoom::readPlayerCommands(const std::shared_ptr<Client>& client, const std::string& cmd)
    {
        if(cmd.size() >= 4) {
            if(cmd.substr(0, 7) == "select ") {
                auto x = cmd[7];
                auto y = cmd[9];
            }
        }

        return false;
    }

    bool ServerRoom::readClientCommands(const std::shared_ptr<Client>& client, const std::string& cmd)
    {
        if(cmd.size() > 5) {
            if(auto subCmd = cmd.substr(0, 5); subCmd == "join ") {
                auto teamCmd = cmd.substr(5, cmd.size() - 5);
                if(teamCmd == "white") {
                    if(!m_whitePlayer)
                        m_whitePlayer = client;
                    else
                        writeCommand(client, "no_join_white");

                    return true;
                } else if(teamCmd == "black") {
                    if(!m_blackPlayer)
                        m_blackPlayer = client;
                    else
                        writeCommand(client, "no_join_black");

                    return true;
                }
            } else if(cmd.size() > 8) {
                if(cmd.substr(0, 8) == "message ") {
                    this->sendToAll(cmd);
                }
            }
        }

        return false;
    }

    void ServerRoom::writeCommand(std::shared_ptr<Client> client, std::string cmd)
    {
        boost::asio::async_write(client->m_socket, boost::asio::buffer(cmd.data(), cmd.size()), [this, client]
        (boost::system::error_code error, std::size_t) {
            if(error)
                this->leave(client);
        });
    }

    ServerAcceptor::ServerAcceptor(int port) :
        m_acceptor(m_context, tcp::endpoint(tcp::v4(), port)),
        m_room(m_acceptor)
    {
    }

    void ServerAcceptor::doAccept()
    {
        m_acceptor.async_accept([this]
        (boost::system::error_code error, tcp::socket socket) {
            if(!error)
                m_room.join(std::make_shared<Client>(std::move(socket)));

            this->doAccept();
        });
    }
}
