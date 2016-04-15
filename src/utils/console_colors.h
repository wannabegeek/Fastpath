/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef tfconsolecolors_h
#define tfconsolecolors_h

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>

namespace tf {
    namespace color {
        typedef enum {
            normal,
            black,
            red,
            green,
            yellow,
            blue,
            magenta,
            cyan,
            white
        } console_color;
    }

    namespace attr {
        typedef enum {
            normal,
            bright,
            dim,
            underline,
            blink,
            reverse,
            hidden
        } console_attr;
    }

    static const char *color_values[] = {
            ";0m",
            ";30m",
            ";31m",
            ";32m",
            ";33m",
            ";34m",
            ";35m",
            ";36m",
            ";37m"
    };

    static const char *console_escape = "\x1B[";

    class setcolor {
    private:
        const color::console_color m_color;
        const attr::console_attr m_attr;

    public:

        static bool supported_output_device() {
            bool supported = false;
            if (isatty(STDOUT_FILENO) != 0) {
                const char *const term = getenv("TERM");
                if (term != nullptr) {
                    supported = std::strcmp(term, "xterm") == 0 ||
                            std::strcmp(term, "xterm") == 0 ||
                            std::strcmp(term, "xterm-color") == 0 ||
                            std::strcmp(term, "xterm-256color") == 0 ||
                            std::strcmp(term, "screen") == 0 ||
                            std::strcmp(term, "screen-256color") == 0 ||
                            std::strcmp(term, "linux") == 0 ||
                            std::strcmp(term, "cygwin") == 0;
                }
            }
            return supported;
        }


        setcolor(const tf::color::console_color color, tf::attr::console_attr attr = tf::attr::normal) : m_color(color), m_attr(attr) {}

        friend std::ostream &operator<<(std::ostream &s, const setcolor &ref) {
            s << console_escape << ref.m_attr << color_values[ref.m_color];
            return s;
        }
    };
}

#endif
