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
#ifndef GUARD_MAR_debug_debug_log_H
#define GUARD_MAR_debug_debug_log_H

#ifdef NDEBUG
#define DEBUG_LOG(...)
#define DEBUG_LOG_WITH_LEVEL(level, ...)
#else
// custom headers
#include "log.h"
#include "log_level.h"

#define DEBUG_LOG(...) mar::debug::write_line_to_terminal_console(mar::debug::elog_level::Debug, __VA_ARGS__)
#define DEBUG_LOG_WITH_LEVEL(level, ...) mar::debug::write_line_to_terminal_console(level, __VA_ARGS__)
#endif // NDEBUG

#endif // GUARD_MAR_debug_debug_log_H
