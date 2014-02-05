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

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <functional>
//#include <regex>

using namespace std;
using namespace std::placeholders;

#include "libupnpp/upnpplib.hxx"
#include "libupnpp/soaphelp.hxx"
#include "libupnpp/device.hxx"

#include "mpdcli.hxx"
#include "upmpdutils.hxx"

static const string friendlyName("UpMpd");

static const string deviceDescription;

string rdc_scdp;

string avt_scdp;


class UpMpd : public UpnpDevice {
public:
	UpMpd(const string& deviceid, 
		  const unordered_map<string, string>& xmlfiles,
		  MPDCli *mpdcli);

	// RenderingControl
	int setMute(const SoapArgs& sc, SoapData& data);
	int getMute(const SoapArgs& sc, SoapData& data);
	int setVolume(const SoapArgs& sc, SoapData& data, bool isDb);
	int getVolume(const SoapArgs& sc, SoapData& data, bool isDb);
	int listPresets(const SoapArgs& sc, SoapData& data);
	int selectPreset(const SoapArgs& sc, SoapData& data);
	int getVolumeDBRange(const SoapArgs& sc, SoapData& data);

	// AVTransport
	int setAVTransportURI(const SoapArgs& sc, SoapData& data);
	int getPositionInfo(const SoapArgs& sc, SoapData& data);

	// Shared
    virtual bool getEventData(std::vector<std::string>& names, 
                              std::vector<std::string>& values);
	
private:
	void doNotifyEvent();
	MPDCli *m_mpdcli;
};


UpMpd::UpMpd(const string& deviceid, 
			 const unordered_map<string, string>& xmlfiles,
			 MPDCli *mpdcli)
	: UpnpDevice(deviceid, xmlfiles), m_mpdcli(mpdcli)
{
	addServiceType("urn:upnp-org:serviceId:RenderingControl",
				   "urn:schemas-upnp-org:service:RenderingControl:1");
	{	auto bound = bind(&UpMpd::setMute, this, _1, _2);
		addActionMapping("SetMute", bound);
	}
	{	auto bound = bind(&UpMpd::getMute, this, _1, _2);
		addActionMapping("GetMute", bound);
	}
	{	auto bound = bind(&UpMpd::setVolume, this, _1, _2, false);
		addActionMapping("SetVolume", bound);
	}
	{	auto bound = bind(&UpMpd::setVolume, this, _1, _2, true);
		addActionMapping("SetVolumeDB", bound);
	}
	{	auto bound = bind(&UpMpd::getVolume, this, _1, _2, false);
		addActionMapping("GetVolume", bound);
	}
	{	auto bound = bind(&UpMpd::getVolume, this, _1, _2, true);
		addActionMapping("GetVolumeDB", bound);
	}
	{	auto bound = bind(&UpMpd::listPresets, this, _1, _2);
		addActionMapping("ListPresets", bound);
	}
	{	auto bound = bind(&UpMpd::selectPreset, this, _1, _2);
		addActionMapping("SelectPreset", bound);
	}
	{	auto bound = bind(&UpMpd::getVolumeDBRange, this, _1, _2);
		addActionMapping("GetVolumeDBRange", bound);
	}

	addServiceType("urn:upnp-org:serviceId:AVTransport",
				   "urn:schemas-upnp-org:service:AVTransport:1");

	{	auto bound = bind(&UpMpd::setAVTransportURI, this, _1, _2);
		addActionMapping("SetAVTransportURI", bound);
	}
	{	auto bound = bind(&UpMpd::getPositionInfo, this, _1, _2);
		addActionMapping("GetPositionInfo", bound);
	}
}


///////// TOBEDONE: this needs a serviceid parameter
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

////////////////////////////////////////////////////
/// RenderingControl methods

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

///////////////// AVTransport methods

int UpMpd::setAVTransportURI(const SoapArgs& sc, SoapData& data)
{
	map<string, string>::const_iterator it;
		
	it = sc.args.find("CurrentURI");
	if (it == sc.args.end() || it->second.empty()) {
		return UPNP_E_INVALID_PARAM;
	}
	string uri = it->second;
	string metadata;
	it = sc.args.find("CurrentURIMetaData");
	if (it != sc.args.end())
		metadata = it->second;
	return UPNP_E_SUCCESS;
}

// Note: we need to return all out arguments defined by the SOAP call even if
// they don't make sense (because there is no song playing). Ref upnp arch p.51:
//
//   argumentName: Required if and only if action has out
//   arguments. Value returned from action. Repeat once for each out
//   argument. If action has an argument marked as retval, this
//   argument must be the first element. (Element name not qualified
//   by a namespace; element nesting context is sufficient.) Case
//   sensitive. Single data type as defined by UPnP service
//   description. Every “out” argument in the definition of the action
//   in the service description must be included, in the same order as
//   specified in the service description (SCPD) available from the
//   device.

int UpMpd::getPositionInfo(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	cerr << "UpMpd::getPositionInfo. State: " << mpds.state << endl;

	bool is_song = (mpds.state == MpdStatus::MPDS_PLAY) || 
		(mpds.state == MpdStatus::MPDS_PAUSE);

	if (is_song) {
		data.data.push_back(pair<string,string>("Track", "1"));
	} else {
		data.data.push_back(pair<string,string>("Track", "0"));
	}

	if (is_song) {
		data.data.push_back(pair<string,string>("TrackDuration", 
												upnpduration(mpds.songlenms)));
	} else {
		data.data.push_back(pair<string,string>("Track", "0"));
	}

	if (is_song) {
		data.data.push_back(pair<string,string>("TrackMetaData", 
												didlmake(mpds)));
	} else {
		data.data.push_back(pair<string,string>("TrackMetaData", ""));
	}

	cerr << "before find" << endl;
	map<string, string>::const_iterator it = 
		mpds.currentsong.find("uri");
//	unordered_map<string, string>::const_iterator it = mpds.currentsong.end();
	cerr << "after find" << endl;
	if (is_song && it != mpds.currentsong.end()) {
		data.data.push_back(pair<string,string>("TrackURI", it->second));
	} else {
		data.data.push_back(pair<string,string>("TrackURI", ""));
	}
	if (is_song) {
		data.data.push_back(pair<string,string>("RelTime", 
											upnpduration(mpds.songelapsedms)));
	} else {
		data.data.push_back(pair<string,string>("RelTime", "0:00:00"));
	}

	if (is_song) {
		data.data.push_back(pair<string,string>("AbsTime", 
											upnpduration(mpds.songelapsedms)));
	} else {
		data.data.push_back(pair<string,string>("AbsTime", "0:00:00"));
	}

	data.data.push_back(pair<string,string>("RelCount", "0"));
	data.data.push_back(pair<string,string>("AbsCount", "0"));
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

static string datadir("upmpd/");
int main(int argc, char *argv[])
{
	thisprog = argv[0];
	argc--; argv++;

	if (argc != 0)
		Usage();

	// Initialize libupnpp, and check health
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

	// Initialize MPD client module
	MPDCli mpdcli("hm1.dockes.com");
	if (!mpdcli.ok()) {
		cerr << "MPD connection failed" << endl;
		return 1;
	}
	
	// Create unique ID
	string UUID = LibUPnP::makeDevUUID(friendlyName);

	// Read our XML data.
	
	string reason;

	string description;
	string filename = datadir + "description.xml";
	if (!file_to_string(filename, description, &reason)) {
		cerr << "Failed reading " << filename << " : " << reason << endl;
		return 1;
	}
	// Update device description with UUID
    description = regsub1("@UUID@", description, UUID);
    cerr << "Description: [" << description << "]" << endl;

	string rdc_scdp;
	filename = datadir + "RenderingControl.xml";
	if (!file_to_string(filename, rdc_scdp, &reason)) {
		cerr << "Failed reading " << filename << " : " << reason << endl;
		return 1;
	}
	string avt_scdp;
	filename = datadir + "AVTransport.xml";
	if (!file_to_string(filename, avt_scdp, &reason)) {
		cerr << "Failed reading " << filename << " : " << reason << endl;
		return 1;
	}

	// List the XML files to be served through http (all will live in '/')
	unordered_map<string, string> xmlfiles;
	xmlfiles["description.xml"] = description;
	xmlfiles["RenderingControl.xml"] = rdc_scdp;
	xmlfiles["AVTransport.xml"] = avt_scdp;

	// Initialize the UPnP device object.

	UpMpd device(string("uuid:") + UUID, xmlfiles, &mpdcli);

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
