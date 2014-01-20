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

#include <string>
#include <iostream>
#include <vector>
using namespace std;

#include "libupnpp/upnpplib.hxx"
#include "libupnpp/vdir.hxx"
#include "libupnpp/soaphelp.hxx"

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
"    <UDN>uuid:\n"
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
"    <action>\n"
"      <name>GetLoudness</name>\n"
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
"          <name>CurrentLoudness</name>\n"
"          <direction>out</direction>\n"
"          <relatedStateVariable>Loudness</relatedStateVariable>\n"
"        </argument>\n"
"      </argumentList>\n"
"    </action>\n"
"    <action>\n"
"      <name>SetLoudness</name>\n"
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
"          <name>DesiredLoudness</name>\n"
"          <direction>in</direction>\n"
"          <relatedStateVariable>Loudness</relatedStateVariable>\n"
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
"        <maximum>Vendor defined</maximum>\n"
"        <step>1</step>\n"
"      </allowedValueRange>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>VolumeDB</name>\n"
"      <dataType>i2</dataType>\n"
"      <allowedValueRange>\n"
"        <minimum>Vendor defined</minimum>\n"
"        <maximum>Vendor defined</maximum>\n"
"        <step>Vendor defined</step>\n"
"      </allowedValueRange>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>Loudness</name>\n"
"      <dataType>boolean</dataType>\n"
"    </stateVariable>\n"
"    <stateVariable sendEvents=\"no\">\n"
"      <name>A_ARG_TYPE_Channel</name>\n"
"      <dataType>string</dataType>\n"
"      <allowedValueList>\n"
"        <allowedValue>Master</allowedValue>\n"
"        <allowedValue>LF</allowedValue>\n"
"        <allowedValue>RF</allowedValue>\n"
"        <allowedValue>CF</allowedValue>\n"
"        <allowedValue>LFE</allowedValue>\n"
"        <allowedValue>LS</allowedValue>\n"
"        <allowedValue>RS</allowedValue>\n"
"        <allowedValue>LFC</allowedValue>\n"
"        <allowedValue>RFC</allowedValue>\n"
"        <allowedValue>SD</allowedValue>\n"
"        <allowedValue>SL</allowedValue>\n"
"        <allowedValue>SR </allowedValue>\n"
"        <allowedValue>T</allowedValue>\n"
"        <allowedValue>B</allowedValue>\n"
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
"        <allowedValue>InstallationDefaults</allowedValue>\n"
"        <allowedValue>Vendor defined</allowedValue>\n"
"      </allowedValueList>\n"
"    </stateVariable>\n"
"  </serviceStateTable>\n"
"</scpd>\n"
;


typedef int (*soapfun)(const SoapCall&, MPDCli *) ;
static map<string, soapfun> soapcalls;

int setMute(const SoapCall& sc, MPDCli *mpdcli)
{
	map<string, string>::const_iterator it = sc.args.find("DesiredMute");
	if (it == sc.args.end() || it->second.empty()) {
		return UPNP_E_INVALID_PARAM;
	}
	if (it->second[0] == 'F') {
		// Relative set of 0 -> restore pre-mute
		mpdcli->setVolume(0, 1);
	} else if (it->second[0] == 'T') {
		mpdcli->setVolume(0);
	} else {
		return UPNP_E_INVALID_PARAM;
	}
}

static PTMutexInit cblock;
static int cluCallBack(Upnp_EventType et, void* evp, void* token)
{
	PTMutexLocker lock(cblock);
	MPDCli *mpdcli = (MPDCli*)token;

	cerr << "cluCallBack: evt type:" << 
		LibUPnP::evTypeAsString(et).c_str() << endl;

	switch (et) {
	case UPNP_CONTROL_ACTION_REQUEST:
	{
		struct Upnp_Action_Request *act = (struct Upnp_Action_Request *)evp;
		cerr << "UPNP_CONTROL_ACTION_REQUEST: " << act->ActionName <<
			" Params: " << ixmlPrintDocument(act->ActionRequest) << endl;
		SoapCall sc;
		if (!decodeSoap(act->ActionName, act->ActionRequest, &sc)) {
			return UPNP_E_INVALID_PARAM;
		}
		map<string, soapfun>::iterator it = soapcalls.find(sc.name);
		if (it != soapcalls.end())
			return it->second(sc, mpdcli);
		else
			return UPNP_E_INVALID_PARAM;
	}
	break;
	case UPNP_CONTROL_GET_VAR_REQUEST:
	{
		struct Upnp_State_Var_Request *act = 
			(struct Upnp_State_Var_Request *)evp;
		cerr << "UPNP_CONTROL_ACTION_REQUEST: " << act->StateVarName << endl;
	}
	break;
	case UPNP_EVENT_SUBSCRIPTION_REQUEST:
	{
		struct Upnp_Subscription_Request *act = 
			(struct  Upnp_Subscription_Request*)evp;
	}
	break;
	default:
		// Ignore other events for now
		break;
	}

	return UPNP_E_INVALID_PARAM;
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

	soapcalls["SetMute"] = setMute;

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

	MPDCli mpdcli("rasp1.dockes.com");
	if (!mpdcli.ok()) {
		cerr << "MPD connection failed" << endl;
		return 1;
	}
	mylib->registerHandler(UPNP_CONTROL_ACTION_REQUEST, cluCallBack, &mpdcli);
	mylib->registerHandler(UPNP_CONTROL_GET_VAR_REQUEST, cluCallBack, &mpdcli);
	mylib->registerHandler(UPNP_EVENT_SUBSCRIPTION_REQUEST, cluCallBack, &mpdcli);

	myDeviceUUID = LibUPnP::makeDevUUID(friendlyName);
	string description = deviceDescription1 + myDeviceUUID + deviceDescription2;
	cerr << myDeviceUUID << endl;
	VirtualDir* theVD = VirtualDir::getVirtualDir();
	if (theVD == 0) {
		cerr << "Can't get VirtualDir" << endl;
		return 1;
	}
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
