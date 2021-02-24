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
#ifndef NDEBUG
#ifndef GUARD_MAR_debug_log_H
#define GUARD_MAR_debug_log_H

// standard headers
#include <iosfwd>

// custom headers
#include "log_level.h"

namespace mar
{
namespace debug
{
    // terminal logger
    template<class... Args> void write_to_terminal_console(elog_level level, Args&&... args);
    template<class... Args> void write_line_to_terminal_console(elog_level level, Args&&... args);
    template<class... Args> void write_to_stream(std::ostream& stream, elog_level level, Args&&... args);
}
}

// definitions
#include "log.inl"

#endif // GUARD_MAR_debug_log_H
#endif // NDEBUG
