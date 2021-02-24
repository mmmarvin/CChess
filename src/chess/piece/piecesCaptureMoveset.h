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

// headers
#include <tuple>
#include "../types.h"
#include "piece.h"

namespace cchess
{
    class Chess;
    std::tuple<bool, position_t, position_t> isPawnSpecialCaptureValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard);
    std::tuple<bool, position_t, position_t> isCaptureValid(Piece piece, position_t xs, position_t ys, position_t xd, position_t yd, const Chess& chessboard);
}
