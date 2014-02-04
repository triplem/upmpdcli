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

////////////////////// libupnpp UPnP explorer test program

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <iostream>
#include <vector>
#include <functional>

using namespace std;
using namespace std::placeholders;

#include "libupnpp/upnpplib.hxx"
#include "libupnpp/vdir.hxx"
#include "libupnpp/soaphelp.hxx"
#include "libupnpp/device.hxx"

#include "mpdcli.hxx"

static const string friendlyName("UpMpd");
static const string deviceDescription1 = 
"<?xml version=\"1.0\"?>\n"
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\n"
"  <specVersion>\n"
"    <major>1</major>\n"
"    <minor>0</minor>\n"
"  </specVersion>\n"
"  <device>\n"
"    <deviceType>urn:schemas-upnp-org:device:MediaRenderer:1</deviceType>\n"
"    <friendlyName>UpMPD</friendlyName>\n"
"    <manufacturer>JF Light Industries</manufacturer>\n"
"    <manufacturerURL>http://www.github.com/medoc92/upmpd</manufacturerURL>\n"
"    <modelDescription>UPnP front-end to MPD</modelDescription>\n"
"    <modelName>UpMPD</modelName>\n"
"    <modelNumber>1.0</modelNumber>\n"
"    <modelURL>http://www.github.com/medoc92/upmpd</modelURL>\n"
"    <serialNumber>72</serialNumber>\n"
"    <UDN>uuid:"
	;
static const string deviceDescription2 = 
"</UDN>\n"
"    <serviceList>\n"
"      <service>\n"
"        <serviceType>urn:schemas-upnp-org:service:RenderingControl:1</serviceType>\n"
"        <serviceId>urn:upnp-org:serviceId:RenderingControl</serviceId>\n"
"        <SCPDURL>/RenderingControl.xml</SCPDURL>\n"
"        <controlURL>/ctl/RenderingControl</controlURL>\n"
"        <eventSubURL>/evt/RenderingControl</eventSubURL>\n"
"      </service>\n"
"    </serviceList>\n"
"    <presentationURL>/presentation.html</presentationURL>\n"
"  </device>\n"
"</root>\n"
;

string scdp = 
"<?xml version=\"1.0\"?>\n"
"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">\n"
"  <specVersion>\n"
"    <major>1</major>\n"
"    <minor>0</minor>\n"
"  </specVersion>\n"
"  <actionList>\n"
"    <action>\n"
"      <name>ListPresets</name>\n"
"      <argumentList>\n"
"        <argument>\n"
"          <name>InstanceID</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>CurrentPresetNameList</name>\n"
"          <direction>out</direction>\n"
"          <relatedStateVariable>PresetNameList</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"    <action>\n"
"      <name>SelectPreset</name>\n"
"      <argumentList>\n"
"        <argument>\n"
"          <name>InstanceID</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>PresetName</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_PresetName</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"    <action>\n"
"      <name>GetMute</name>\n"
"      <argumentList>\n"
"        <argument>\n"
"          <name>InstanceID</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>Channel</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>CurrentMute</name>\n"
"          <direction>out</direction>\n"
"          <relatedStateVariable>Mute</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"    <action>\n"
"      <name>SetMute</name>\n"
"      <argumentList>\n"
"        <argument>\n"
"          <name>InstanceID</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>Channel</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>DesiredMute</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>Mute</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"    <action>\n"
"      <name>GetVolume</name>\n"
"      <argumentList>\n"
"        <argument>\n"
"          <name>InstanceID</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>Channel</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>CurrentVolume</name>\n"
"          <direction>out</direction>\n"
"          <relatedStateVariable>Volume</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"    <action>\n"
"      <name>SetVolume</name>\n"
"      <argumentList>\n"
"        <argument>\n"
"          <name>InstanceID</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>Channel</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>DesiredVolume</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>Volume</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"    <action>\n"
"      <name>GetVolumeDB</name>\n"
"      <argumentList>\n"
"        <argument>\n"
"          <name>InstanceID</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>Channel</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>CurrentVolume</name>\n"
"          <direction>out</direction>\n"
"          <relatedStateVariable>VolumeDB</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"    <action>\n"
"      <name>SetVolumeDB</name>\n"
"      <argumentList>\n"
"        <argument>\n"
"          <name>InstanceID</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>Channel</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>DesiredVolume</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>VolumeDB</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"    <action>\n"
"      <name>GetVolumeDBRange</name>\n"
"      <argumentList>\n"
"        <argument>\n"
"          <name>InstanceID</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>Channel</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>MinValue</name>\n"
"          <direction>out</direction>\n"
"          <relatedStateVariable>VolumeDB</relatedStateVariable>\n"
"        </argument>\n"
"        <argument>\n"
"          <name>MaxValue</name>\n"
"          <direction>out</direction>\n"
"          <relatedStateVariable>VolumeDB</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"  </actionList>\n"
"  <serviceStateTable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>PresetNameList</name>\n"
"      <dataType>string</dataType>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"yes\">\n"
"      <name>LastChange</name>\n"
"      <dataType>string</dataType>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>Mute</name>\n"
"      <dataType>boolean</dataType>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>Volume</name>\n"
"      <dataType>ui2</dataType>\n"
"      <allowedValueRange>\n"
"        <minimum>0</minimum>\n"
"        <maximum>100</maximum>\n"
"        <step>1</step>\n"
"      </allowedValueRange>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>VolumeDB</name>\n"
"      <dataType>i2</dataType>\n"
"      <allowedValueRange>\n"
"        <minimum>-10240</minimum>\n"
"        <maximum>0</maximum>\n"
"        <step>1</step>\n"
"      </allowedValueRange>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>A_ARG_TYPE_Channel</name>\n"
"      <dataType>string</dataType>\n"
"      <allowedValueList>\n"
"        <allowedValue>Master</allowedValue>\n"
"      </allowedValueList>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>A_ARG_TYPE_InstanceID</name>\n"
"      <dataType>ui4</dataType>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>A_ARG_TYPE_PresetName</name>\n"
"      <dataType>string</dataType>\n"
"      <allowedValueList>\n"
"        <allowedValue>FactoryDefaults</allowedValue>\n"
"      </allowedValueList>\n"
"    </stateVariable>\n"
"  </serviceStateTable>\n"
"</scpd>\n"
;

// We do db upnp-encoded values from -10240 (0%) to 0 (100%)
static int percentodbvalue(int value)
{
    int dbvalue;
    if (value == 0) {
        dbvalue = -10240;
    } else {
        float ratio = float(value)*value / 10000.0;
        float db = 10 * log10(ratio);
        dbvalue = int(256 * db);
    }
    return dbvalue;
}

static int dbvaluetopercent(int dbvalue)
{
    float db = float(dbvalue) / 256.0;
    float vol = exp10(db/10);
    int percent = floor(sqrt(vol * 10000.0));
	if (percent < 0)	percent = 0;
	if (percent > 100)	percent = 100;
    return percent;
}

class UpMpd : public UpnpDevice {
public:
	UpMpd(const string& deviceid, MPDCli *mpdcli);

	int setMute(const SoapArgs& sc, SoapData& data);
	int getMute(const SoapArgs& sc, SoapData& data);
	int setVolume(const SoapArgs& sc, SoapData& data, bool isDb);
	int getVolume(const SoapArgs& sc, SoapData& data, bool isDb);
	int listPresets(const SoapArgs& sc, SoapData& data);
	int selectPreset(const SoapArgs& sc, SoapData& data);
	int getVolumeDBRange(const SoapArgs& sc, SoapData& data);
    virtual bool getEventData(std::vector<std::string>& names, 
                              std::vector<std::string>& values);
	
private:
	void doNotifyEvent();
	MPDCli *m_mpdcli;
};


UpMpd::UpMpd(const string& deviceid, MPDCli *mpdcli)
	: UpnpDevice(deviceid), m_mpdcli(mpdcli)
{
	addServiceType("urn:upnp-org:serviceId:RenderingControl",
				   "urn:schemas-upnp-org:service:RenderingControl:1");

	{
		auto bound = bind(&UpMpd::setMute, this, _1, _2);
		addActionMapping("SetMute", bound);
	}
	{
		auto bound = bind(&UpMpd::getMute, this, _1, _2);
		addActionMapping("GetMute", bound);
	}
	{
		auto bound = bind(&UpMpd::setVolume, this, _1, _2, false);
		addActionMapping("SetVolume", bound);
	}
	{
		auto bound = bind(&UpMpd::setVolume, this, _1, _2, true);
		addActionMapping("SetVolumeDB", bound);
	}
	{
		auto bound = bind(&UpMpd::getVolume, this, _1, _2, false);
		addActionMapping("GetVolume", bound);
	}
	{
		auto bound = bind(&UpMpd::getVolume, this, _1, _2, true);
		addActionMapping("GetVolumeDB", bound);
	}
	{
		auto bound = bind(&UpMpd::listPresets, this, _1, _2);
		addActionMapping("ListPresets", bound);
	}
	{
		auto bound = bind(&UpMpd::selectPreset, this, _1, _2);
		addActionMapping("SelectPreset", bound);
	}
	{
		auto bound = bind(&UpMpd::getVolumeDBRange, this, _1, _2);
		addActionMapping("GetVolumeDBRange", bound);
	}
}

// LastChange contains all the variables that were changed since the last
// event. For us that's at most Mute, Volume, VolumeDB
// <Event xmlns=”urn:schemas-upnp-org:metadata-1-0/AVT_RCS">
//   <InstanceID val=”0”>
//     <Mute channel=”Master” val=”0”/>
//     <Volume channel=”Master” val=”24”/>
//     <VolumeDB channel=”Master” val=”24”/>
//   </InstanceID>
// </Event>
bool UpMpd::getEventData(std::vector<std::string>& names, 
						 std::vector<std::string>& values)
{
	names.push_back("LastChange");
	int volume = m_mpdcli->getVolume();
	char cvalue[40];
	string 
		chgdata("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT_RCS\">\n"
				"<InstanceID val=\"0\">\n"
				"<Mute channel=\"Master\" val=\"");
    chgdata += volume == 0 ? "1" : "0";
	chgdata += "\"/>\n<Volume channel=\"Master\" val=\"";
	sprintf(cvalue, "%d", volume);
	chgdata += cvalue;
	chgdata += "\"/>\n<VolumeDB channel=\"Master\" val=\"";
	sprintf(cvalue, "%d", percentodbvalue(volume));
	chgdata += cvalue;
	chgdata += "\"/>\n</InstanceID>\n</Event>";
	values.push_back(chgdata);
	cerr << "UpMpd::getEventData: " << chgdata << endl;
	return true;
}
	
void UpMpd::doNotifyEvent()
{
	std::vector<std::string> names;
	std::vector<std::string> values;
	getEventData(names, values);
	notifyEvent("urn:upnp-org:serviceId:RenderingControl", names, values);
}

int UpMpd::getVolumeDBRange(const SoapArgs& sc, SoapData& data)
{
	map<string, string>::const_iterator it;

	it = sc.args.find("Channel");
	if (it == sc.args.end() || it->second.compare("Master")) {
		return UPNP_E_INVALID_PARAM;
	}
	data.data.push_back(pair<string,string>("MinValue", "-10240"));
	data.data.push_back(pair<string,string>("MaxValue", "0"));

	return UPNP_E_SUCCESS;
}

int UpMpd::setMute(const SoapArgs& sc, SoapData& data)
{
	map<string, string>::const_iterator it;

	it = sc.args.find("Channel");
	if (it == sc.args.end() || it->second.compare("Master")) {
		return UPNP_E_INVALID_PARAM;
	}
		
	it = sc.args.find("DesiredMute");
	if (it == sc.args.end() || it->second.empty()) {
		return UPNP_E_INVALID_PARAM;
	}
	if (it->second[0] == 'F' || it->second[0] == '0') {
		// Relative set of 0 -> restore pre-mute
		m_mpdcli->setVolume(0, 1);
	} else if (it->second[0] == 'T' || it->second[0] == '1') {
		m_mpdcli->setVolume(0);
	} else {
		return UPNP_E_INVALID_PARAM;
	}
	doNotifyEvent();
	return UPNP_E_SUCCESS;
}

int UpMpd::getMute(const SoapArgs& sc, SoapData& data)
{
	map<string, string>::const_iterator it;

	it = sc.args.find("Channel");
	if (it == sc.args.end() || it->second.compare("Master")) {
		return UPNP_E_INVALID_PARAM;
	}
	int volume = m_mpdcli->getVolume();
	data.data.push_back(pair<string,string>("CurrentMute", 
											volume == 0 ? "1" : "0"));
	return UPNP_E_SUCCESS;
}

int UpMpd::setVolume(const SoapArgs& sc, SoapData& data, bool isDb)
{
	map<string, string>::const_iterator it;

	it = sc.args.find("Channel");
	if (it == sc.args.end() || it->second.compare("Master")) {
		return UPNP_E_INVALID_PARAM;
	}
		
	it = sc.args.find("DesiredVolume");
	if (it == sc.args.end() || it->second.empty()) {
		return UPNP_E_INVALID_PARAM;
	}
	int volume = atoi(it->second.c_str());
	if (isDb) {
		volume = dbvaluetopercent(volume);
	} 
	if (volume < 0 || volume > 100) {
		return UPNP_E_INVALID_PARAM;
	}
	m_mpdcli->setVolume(volume);
	doNotifyEvent();
	return UPNP_E_SUCCESS;
}

int UpMpd::getVolume(const SoapArgs& sc, SoapData& data, bool isDb)
{
	map<string, string>::const_iterator it;

	it = sc.args.find("Channel");
	if (it == sc.args.end() || it->second.compare("Master")) {
		return UPNP_E_INVALID_PARAM;
	}
		
	int volume = m_mpdcli->getVolume();
	if (isDb) {
		volume = percentodbvalue(volume);
	}
	char svolume[30];
	sprintf(svolume, "%d", volume);
	data.data.push_back(pair<string,string>("CurrentVolume", svolume));
	return UPNP_E_SUCCESS;
}

int UpMpd::listPresets(const SoapArgs& sc, SoapData& data)
{
	map<string, string>::const_iterator it;

	// The 2nd arg is a comma-separated list of preset names
	data.data.push_back(pair<string,string>("CurrentPresetNameList",
											"FactoryDefaults"));
	return UPNP_E_SUCCESS;
}

int UpMpd::selectPreset(const SoapArgs& sc, SoapData& data)
{
	map<string, string>::const_iterator it;
		
	it = sc.args.find("PresetName");
	if (it == sc.args.end() || it->second.empty()) {
		return UPNP_E_INVALID_PARAM;
	}
	if (it->second.compare("FactoryDefaults")) {
		return UPNP_E_INVALID_PARAM;
	}

	// Well there is only the volume actually...
	m_mpdcli->setVolume(50);

	doNotifyEvent();
	return UPNP_E_SUCCESS;
}

static char *thisprog;
static char usage [] =
			"  \n\n"
			;
static void
Usage(void)
{
	fprintf(stderr, "%s: usage:\n%s", thisprog, usage);
	exit(1);
}
static int	   op_flags;
#define OPT_MOINS 0x1

static string myDeviceUUID;

int main(int argc, char *argv[])
{
	thisprog = argv[0];
	argc--; argv++;

	if (argc != 0)
		Usage();

	LibUPnP *mylib = LibUPnP::getLibUPnP(true);
	if (!mylib) {
		cerr << " Can't get LibUPnP" << endl;
		return 1;
	}
	if (!mylib->ok()) {
		cerr << "Lib init failed: " <<
			mylib->errAsString("main", mylib->getInitError()) << endl;
		return 1;
	}
	mylib->setLogFileName("/tmp/clilibupnp.log");

	MPDCli mpdcli("hm1.dockes.com");
	if (!mpdcli.ok()) {
		cerr << "MPD connection failed" << endl;
		return 1;
	}

	myDeviceUUID = LibUPnP::makeDevUUID(friendlyName);
	cerr << "Generated UUID: [" << myDeviceUUID << "]" << endl;

	UpMpd device(string("uuid:") + myDeviceUUID, &mpdcli);

	VirtualDir* theVD = VirtualDir::getVirtualDir();
	if (theVD == 0) {
		cerr << "Can't get VirtualDir" << endl;
		return 1;
	}
	string description = deviceDescription1 + myDeviceUUID + deviceDescription2;
	theVD->addFile("/", "description.xml", description, "application/xml");
	theVD->addFile("/", "RenderingControl.xml", scdp, "application/xml");

	mylib->setupWebServer(description);
	while (true)
		sleep(1000);
	return 0;
}


/* Local Variables: */
/* mode: c++ */
/* c-basic-offset: 4 */
/* tab-width: 4 */
/* indent-tabs-mode: t */
/* End: */
