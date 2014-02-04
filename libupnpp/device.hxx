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
#ifndef _DEVICE_H_X_INCLUDED_
#define _DEVICE_H_X_INCLUDED_

#include <unordered_map>
#include <functional>


#include "soaphelp.hxx"

class UpnpDevice;

//typedef int (*soapfun)(const SoapArgs&, void *, SoapData&) ;

typedef function<int (const SoapArgs&, SoapData&)> soapfun;

/** Define a virtual interface to link libupnp operations to a device 
 * implementation 
 */
class UpnpDevice {
public:
    UpnpDevice(const std::string& deviceId);
    void addServiceType(const std::string& serviceId, 
                        const std::string& serviceType);
    void addActionMapping(const std::string& actName, soapfun fun);

    /** Called by the library when a control point subscribes, to
        retrieve eventable data. Return name/value pairs in the data array 
        To be overriden by the derived class.
    */
    virtual bool getEventData(std::vector<std::string>& names, 
                              std::vector<std::string>& values)
        {
            return true;
        }

    /** To be called by the device layer when data changes and an
        event should happen */
    void notifyEvent(const std::string& serviceId,
                     const std::vector<std::string>& names, 
                     const std::vector<std::string>& values);
    bool ok() {return m_lib != 0;}
private:
    const std::string& serviceType(const std::string& serviceId);
    std::string serviceKey(const std::string& UDN, const std::string& servId)
        {
            return UDN + "|" + servId;
        }
            
    LibUPnP *m_lib;
    std::string m_deviceId;
    std::unordered_map<std::string, std::string> m_serviceTypes;
    std::unordered_map<std::string, soapfun> m_calls;

    static unordered_map<std::string, UpnpDevice *> o_devices;
    static int sCallBack(Upnp_EventType et, void* evp, void*);
    int callBack(Upnp_EventType et, void* evp);
};


#endif /* _DEVICE_H_X_INCLUDED_ */
