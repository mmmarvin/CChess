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
#include <cstring>
#include <sstream>

#include <boost/asio.hpp>

#include "src/chess/chess.h"
#include "src/displayserver/displayserver.h"
using namespace std;

using boost::asio::ip::tcp;
using boost::asio::ip::address_v4;

namespace
{
    bool isDigit(char c)
    {
        return isdigit(c);
    }

    bool isStringNumber(const string& numberString)
    {
        return find_if_not(numberString.begin(), numberString.end(), &isDigit) == numberString.end();
    }

    int getStringNumber(const string& numberString)
    {
        int ret;

        stringstream ss;
        ss.str(numberString);
        ss >> ret;

        return ret;
    }

    void runDisplayServer(string port)
    {
        if(isStringNumber(port)) {
            auto portNumber = getStringNumber(port);

            try {
                cchess_display::DisplayServer displayServer(portNumber);
                displayServer.run();
            } catch(...) {}
        }
    }
}

int main(int argc, char** argv)
{
    if(argc == 3) {
        if(string(argv[1]) == "-ds")
            runDisplayServer(string(argv[2]));
    }

    return 0;
}
