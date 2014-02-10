/* Copyright (C) 2014 J.F.Dockes
 *	 This program is free software; you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	 GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program; if not, write to the
 *	 Free Software Foundation, Inc.,
 *	 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _UPMPDUTILS_H_X_INCLUDED_
#define _UPMPDUTILS_H_X_INCLUDED_

#include <string>
#include <unordered_map>

/**
 * Read file into string.
 * @return true for ok, false else
 */
extern bool file_to_string(const std::string &filename, std::string &data, 
                           std::string *reason = 0);

// Escape XML special chars.
extern std::string xmlquote(const std::string& in);

// Convert between db value to percent values (Get/Set Volume and VolumeDb)
extern int percentodbvalue(int value);
extern int dbvaluetopercent(int dbvalue);

extern bool sleepms(int ms);

// Format milliseconds into H+:MM:SS
extern std::string upnpduration(int ms);
// hms to seconds
extern int upnpdurationtos(const string& dur);

// Return mapvalue or null strings, for maps where absent entry and
// null data are equivalent
extern const std::string& mapget(
    const std::unordered_map<std::string, std::string>& im, 
    const std::string& k);

// Format a didl fragment from MPD status data
class MpdStatus;
extern std::string didlmake(const MpdStatus& mpds, bool next = false);

// Replace the first occurrence of regexp. cxx11 regex does not work
// that well yet...
extern std::string regsub1(const std::string& sexp, const std::string& input, 
                           const std::string& repl);

#endif /* _UPMPDUTILS_H_X_INCLUDED_ */
