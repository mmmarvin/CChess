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
// custom headers
#include "high_resolution_clock.h"

namespace mar
{
namespace chrono
{
    using std::chrono::high_resolution_clock;
}
    high_resolution_clock::high_resolution_clock() :
        m_start(chrono::high_resolution_clock::now()),
        m_isrunning(false)
    {
    }

    void high_resolution_clock::start()
    {
        if(!m_isrunning) {
            restart_impl();
            m_isrunning = true;
        }
    }

    void high_resolution_clock::stop() noexcept
    {
        m_isrunning = false;
    }

    time high_resolution_clock::get_elapsed() const
    {
        return get_elapsed_impl().first;
    }

    time high_resolution_clock::restart()
    {
        if(!m_isrunning)
            m_isrunning = true;

        return restart_impl();
    }

    std::pair<time, chrono::high_resolution_clock::time_point> high_resolution_clock::get_elapsed_impl() const
    {
        chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
        chrono::high_resolution_clock::time_point prev = m_start;

        time elapsed;
        elapsed += time(time::nanoseconds(std::chrono::duration_cast<std::chrono::nanoseconds>(now - prev).count()));

        return { elapsed, now };
    }

    time high_resolution_clock::restart_impl()
    {
        auto current_time = get_elapsed_impl();
        m_start = current_time.second;

        return current_time.first;
    }
}
