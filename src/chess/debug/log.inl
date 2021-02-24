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
// standard headers
#include <iostream>

// custom headers
#include "log.h"

namespace mar
{
namespace debug
{
    template<class... Args>
    void write_to_terminal_console(elog_level level, Args&&... args)
    {
        write_to_stream(std::cout, level, std::forward<Args>(args)...);
    }

    template<class... Args>
    void write_line_to_terminal_console(elog_level level, Args&&... args)
    {
        write_to_stream(std::cout, level, std::forward<Args>(args)...);
        std::cout << std::endl;
    }

    template<class... Args>
    void write_to_stream(std::ostream& stream, elog_level level, Args&&... args)
    {
        using expander = int[];

        static constexpr auto RED = "\033[31m";
        static constexpr auto GREEN = "\033[32m";
        static constexpr auto YELLOW = "\033[33m";
        static constexpr auto BLUE = "\033[34m";
        static constexpr auto DEFAULT = "\033[0m";

        switch(level) {
        case elog_level::DeveloperDebug:
            stream << "[ " << BLUE << "DEVELOPER DEBUG" << DEFAULT << " ] ";
            break;
        case elog_level::Debug:
            stream << "[ " << BLUE << "DEBUG" << DEFAULT << " ] ";
            break;
        case elog_level::Info:
            stream << "[ " << GREEN << "INFO" << DEFAULT << " ] ";
            break;
        case elog_level::Warning:
            stream << "[ " << YELLOW << "WARNING" << DEFAULT << " ] ";
            break;
        case elog_level::Error:
            stream << "[ " << RED << "ERROR" << DEFAULT << " ] ";
            break;
        case elog_level::None:
            break;
        default:
            break;
        }

        (void) expander { 0, ((stream << std::forward<Args>(args)), void(), 0)... };
    }
}
}
