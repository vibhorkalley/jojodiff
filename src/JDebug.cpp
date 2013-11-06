/*
 * JDebug.cpp
 *
 * Debugging routines and definitions, typically empty when compiling without the -Ddebug option.
 *
 * Copyright (C) 2002-2011 Joris Heirbaut
 *
 * This file is part of JojoDiff.
 *
 * JojoDiff is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "JDebug.h"

FILE *JDebug::stddbg = stderr ;

#if debug
#warning DEBUG code included !
int JDebug::gbDbg[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif
