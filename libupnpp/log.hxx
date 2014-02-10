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
#ifndef _LOG_H_X_INCLUDED_
#define _LOG_H_X_INCLUDED_

namespace upnppdebug {
    typedef enum loglevels {LLDEB, LLINF, LLERR, LLFAT, LLNON} loglevels;

    extern int loglevel;
}

#define DEBOUT cerr

#define LOGDEB(X) {                                                     \
        if (upnppdebug::loglevel >= upnppdebug::LLDEB)                  \
        {                                                               \
            DEBOUT << __FILE__ << ":" << __LINE__<< "::"; DEBOUT << X;  \
        }                                                               \
    }

#define LOGINF(X) {                                                     \
        if (upnppdebug::loglevel >= upnppdebug::LLINF)                  \
        {                                                               \
            DEBOUT << __FILE__ << ":" << __LINE__<< "::"; DEBOUT << X;  \
        }                                                               \
    }                                                                   

#define LOGERR(X) {                                                     \
        if (upnppdebug::loglevel >= upnppdebug::LLERR)                  \
        {                                                               \
            DEBOUT << __FILE__ << ":" << __LINE__<< "::"; DEBOUT << X;  \
        }                                                               \
    }

#define LOGFAT(X) {                                                     \
        if (upnppdebug::loglevel >= upnppdebug::LLFAT)                  \
        {                                                               \
            DEBOUT << __FILE__ << ":" << __LINE__<< "::"; DEBOUT << X;  \
        }                                                               \
    }


#endif /* _LOG_H_X_INCLUDED_ */
