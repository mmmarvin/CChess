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

#include <boost/asio.hpp>

namespace cchess_display
{
    class DisplayServer
    {
    public:
        explicit DisplayServer(int port);

        std::tuple<std::array<char, 255>, boost::system::error_code, std::size_t> read();
        void write(const char* buff, std::size_t length);

        void run();

    private:
        void initiateHandshake();

        boost::asio::io_context         m_ioc;
        boost::asio::ip::tcp::socket    m_socket;
        bool                            m_started;
    };
}
