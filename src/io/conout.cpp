/*
 * ____ DAPHNE COPYRIGHT NOTICE ____
 *
 * Copyright (C) 2001 Matt Ownby
 *
 * This file is part of DAPHNE, a laserdisc arcade game emulator
 *
 * DAPHNE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * DAPHNE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// CONOUT
// Provides console functionality in a window (replacement for printf, cout,
// etc)

#include "config.h"

#include <string>
#include <iostream>
#include <list>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <g3log/g3log.hpp>
#include "conout.h"
#include "../hypseus.h"
#include "../video/video.h"
#include "../timer/timer.h"
#include "input.h"
#include "mpo_fileio.h"
#include "homedir.h"

using namespace std;

// false = write log entries to g_lsPendingLog, true = write to hypseus_log.txt
// This should be false until the command line has finished parsing.
bool g_log_enabled = false;

// used so log can store text until we've been given the green light to write to
// the log file
// (using a list because it is faster than a vector)
list<string> g_lsPendingLog;

// Prints a single character to the screen
// Returns 1 on success, 0 on failure
void outchr(const char ch)
{
    char s[2] = {0};

    // the SDL_console output library uses vsprintf so % signs mess it up
    if (ch != '%') {
        s[0] = ch;
        s[1] = 0;
    }

    printf(s);
}

void printline(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LOGF(INFO, fmt, args);
    va_end(args);
}

// flood-safe printline
// it only prints every second and anything else is thrown out
void noflood_printline(char *s)
{
    static unsigned int old_timer = 0;

    if (elapsed_ms_time(old_timer) > 1000) {
        printline(s);
        old_timer = refresh_ms_time();
    }
}

// This is a safe version of ITOA in that prevents buffer overflow, but if your
// buffer size is too small,
// you will get an incorrect result.
void safe_itoa(int num, char *a, int sizeof_a)
{
    int i = 0, j = 0, n = 0;
    char c = 0;

    // safety check : size needs to be at least 3 to accomodate one number, a
    // minus sign and a null character
    if (sizeof_a > 2) {
        n = num;

        // if number is negative, reverse its sign to make the math consistent
        // with positive numbers
        if (num < 0) {
            n = -n;
        }

        do {
            a[i++] = n % 10 + '0';
            n = n / 10;
        } while ((n > 0) && (i < (sizeof_a - 2)));
        // sizeof_a - 2 to leave room for a null terminating character and a
        // possible minus sign

        // if number was negative, then add a minus sign
        if (num < 0) a[i++] = '-';

        a[i] = 0; // terminate string

        // reverse string (we had to go in reverse order to do the calculation)
        for (i = 0, j = strlen(a) - 1; i < j; i++, j--) {
            c    = a[i];
            a[i] = a[j];
            a[j] = c;
        }
    }
}

void set_log_enabled(bool val) { g_log_enabled = val; }
