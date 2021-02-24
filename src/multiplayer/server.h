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
#ifndef SERVER_H
#define SERVER_H

#include <set>
#include <boost/asio.hpp>

namespace cchess_multiplayer
{
    class Client
    {
    public:
        Client(boost::asio::ip::tcp::socket socket);

    private:
        friend class ServerRoom;

        char                            m_message[255];
        std::size_t                     m_messageLength;
        boost::asio::ip::tcp::socket    m_socket;
    };

    class ServerRoom
    {
    public:
        ServerRoom(boost::asio::ip::tcp::acceptor& acceptor);

        void join(std::shared_ptr<Client> client);
        void leave(std::shared_ptr<Client> client);

        void sendToAll(std::string msg);

    private:
        void startReading(std::shared_ptr<Client> client);
        bool readOwnerCommands(const std::shared_ptr<Client>& client, const std::string& cmd);
        bool readPlayerCommands(const std::shared_ptr<Client>& client, const std::string& cmd);
        bool readClientCommands(const std::shared_ptr<Client>& client, const std::string& cmd);

        void writeCommand(std::shared_ptr<Client> client, std::string cmd);

        std::set<std::shared_ptr<Client>>   m_clients;

        std::shared_ptr<Client>             m_owner;        // first one to join the room becomse the owner
        std::shared_ptr<Client>             m_whitePlayer;
        std::shared_ptr<Client>             m_blackPlayer;

        boost::asio::ip::tcp::acceptor*     m_acceptor;
    };

    class ServerAcceptor
    {
    public:
        explicit ServerAcceptor(int port);

    private:
        void doAccept();

        boost::asio::io_context         m_context;
        boost::asio::ip::tcp::acceptor  m_acceptor;
        ServerRoom                      m_room;
    };
}

#endif // SERVER_H
