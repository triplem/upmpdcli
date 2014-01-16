/* Copyright (C) 2013 J.F.Dockes
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
#ifndef _LIBUPNP_H_X_INCLUDED_
#define _LIBUPNP_H_X_INCLUDED_

#include <string>
#include <map>

#include <upnp/upnp.h>
#include <upnp/upnptools.h>

#include "ptmutex.hxx"

/** Our link to libupnp. Initialize and keep the handle around */
class LibUPnP {
public:
	~LibUPnP();

	/** Retrieve the singleton LibUPnP object 
	 *
	 * On the first call, this initializes libupnp and registers us as a 
	 * client, but does not perform any other action (does not start
	 * discovery)
	 * @return 0 for failure.
	 */
	static LibUPnP* getLibUPnP();

	/** Set log file name and activate logging.
	 *
	 * @param fn file name to use. Use empty string to turn logging off
	 */
	bool setLogFileName(const std::string& fn);

	/** Set max library buffer size for reading content from servers.
	 * The default is 200k and should be ok */
	void setMaxContentLength(int bytes);

	/** Check state after initialization */
	bool ok() const
	{
		return m_ok;
	}

	/** Retrieve init error if state not ok */
	int getInitError() const
	{
		return m_init_error;
	}

	/** Specify function to be called on given UPnP event. This will happen
	 * in the libupnp thread context.
	 */
	void registerHandler(Upnp_EventType et, Upnp_FunPtr handler, void *cookie);

	UpnpClient_Handle getclh()
	{
		return m_clh;
	}

	/** Translate integer error code (UPNP_E_XXX) to string */
	static std::string errAsString(const std::string& who, int code);

	static std::string evTypeAsString(Upnp_EventType);

private:

	// A Handler object records the data from registerHandler.
	class Handler {
	public:
		Handler()
			: handler(0), cookie(0) {}
		Handler(Upnp_FunPtr h, void *c)
			: handler(h), cookie(c) {}
		Upnp_FunPtr handler;
		void *cookie;
	};

	LibUPnP();
	LibUPnP(const LibUPnP &);
	LibUPnP& operator=(const LibUPnP &);

	static int o_callback(Upnp_EventType, void *, void *);

	bool m_ok;
	int	 m_init_error;
	UpnpClient_Handle m_clh;
	PTMutexInit m_mutex;
	std::map<Upnp_EventType, Handler> m_handlers;
};

#endif /* _LIBUPNP.H_X_INCLUDED_ */
/* Local Variables: */
/* mode: c++ */
/* c-basic-offset: 4 */
/* tab-width: 4 */
/* indent-tabs-mode: t */
/* End: */
