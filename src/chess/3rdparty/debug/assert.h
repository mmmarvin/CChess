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
#ifndef GUARD_MAR_debug_assert_H
#define GUARD_MAR_debug_assert_H

#ifndef NDEBUG
#include <cassert>
#define mar_assert(condition, message) assert((condition) && message)
#else
#define mar_assert(condition, message)
#endif //NDEBUG

#endif // GUARD_MAR_debug_assert_H
