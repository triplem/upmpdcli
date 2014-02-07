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
    virtual bool getEventDataRendering(std::vector<std::string>& names, 
									   std::vector<std::string>& values);

	// AVTransport
	int setAVTransportURI(const SoapArgs& sc, SoapData& data, bool setnext);
	int getPositionInfo(const SoapArgs& sc, SoapData& data);
	int getTransportInfo(const SoapArgs& sc, SoapData& data);
	int getMediaInfo(const SoapArgs& sc, SoapData& data);
	int getDeviceCapabilities(const SoapArgs& sc, SoapData& data);
	int setPlayMode(const SoapArgs& sc, SoapData& data);
	int getTransportSettings(const SoapArgs& sc, SoapData& data);
	int getCurrentTransportActions(const SoapArgs& sc, SoapData& data);
	int playcontrol(const SoapArgs& sc, SoapData& data, int what);
	int seek(const SoapArgs& sc, SoapData& data);
	int seqcontrol(const SoapArgs& sc, SoapData& data, int what);
    virtual bool getEventDataTransport(std::vector<std::string>& names, 
									   std::vector<std::string>& values);

	// Shared
    virtual bool getEventData(const string& serviceid, 
							  std::vector<std::string>& names, 
							  std::vector<std::string>& values);

private:
	void doNotifyEventRendering();
	void doNotifyEventTransport();
	MPDCli *m_mpdcli;
	unordered_map<string, string> m_rdupdates;
	unordered_map<string, string> m_tpupdates;
	void renderingUpdate(const string& nm, const string& val)
		{
			m_rdupdates[nm] = val;
		}
	void transportUpdate(const string& nm, const string& val)
		{
			m_tpupdates[nm] = val;
		}
};

static const string serviceIdRender("urn:upnp-org:serviceId:RenderingControl");
static const string serviceIdTransport("urn:upnp-org:serviceId:AVTransport");

UpMpd::UpMpd(const string& deviceid, 
			 const unordered_map<string, string>& xmlfiles,
			 MPDCli *mpdcli)
	: UpnpDevice(deviceid, xmlfiles), m_mpdcli(mpdcli)
{
	addServiceType(serviceIdRender,
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

	addServiceType(serviceIdTransport,
				   "urn:schemas-upnp-org:service:AVTransport:1");

	{	auto bound = bind(&UpMpd::setAVTransportURI, this, _1, _2, false);
		addActionMapping("SetAVTransportURI", bound);
	}
	{	auto bound = bind(&UpMpd::setAVTransportURI, this, _1, _2, true);
		addActionMapping("SetNextAVTransportURI", bound);
	}
	{	auto bound = bind(&UpMpd::getPositionInfo, this, _1, _2);
		addActionMapping("GetPositionInfo", bound);
	}
	{	auto bound = bind(&UpMpd::getTransportInfo, this, _1, _2);
		addActionMapping("GetTransportInfo", bound);
	}
	{	auto bound = bind(&UpMpd::getMediaInfo, this, _1, _2);
		addActionMapping("GetMediaInfo", bound);
	}
	{	auto bound = bind(&UpMpd::getDeviceCapabilities, this, _1, _2);
		addActionMapping("GetDeviceCapabilities", bound);
	}
	{	auto bound = bind(&UpMpd::setPlayMode, this, _1, _2);
		addActionMapping("SetPlayMode", bound);
	}
	{	auto bound = bind(&UpMpd::getTransportSettings, this, _1, _2);
		addActionMapping("GetTransportSettings", bound);
	}
	{	auto bound = bind(&UpMpd::getCurrentTransportActions, this, _1, _2);
		addActionMapping("GetCurrentTransportActions", bound);
	}
	{	auto bound = bind(&UpMpd::playcontrol, this, _1, _2, 0);
		addActionMapping("Stop", bound);
	}
	{	auto bound = bind(&UpMpd::playcontrol, this, _1, _2, 1);
		addActionMapping("Play", bound);
	}
	{	auto bound = bind(&UpMpd::playcontrol, this, _1, _2, 2);
		addActionMapping("Pause", bound);
	}
//	{	auto bound = bind(&UpMpd::seek, this, _1, _2);
//		addActionMapping("Seek", bound);
//	}
	{	auto bound = bind(&UpMpd::seqcontrol, this, _1, _2, 0);
		addActionMapping("Next", bound);
	}
	{	auto bound = bind(&UpMpd::seqcontrol, this, _1, _2, 1);
		addActionMapping("Previous", bound);
	}
}


bool UpMpd::getEventData(const string& serviceid, 
						 std::vector<std::string>& names, 
						 std::vector<std::string>& values)
{
	if (!serviceid.compare(serviceIdRender)) {
		return getEventDataRendering(names, values);
	} else if (!serviceid.compare(serviceIdTransport)) {
		return getEventDataTransport(names, values);
	} else {
		cerr << "UpMpd::getEventData: servid? [" << serviceid << "]" << endl;
		return UPNP_E_INVALID_PARAM;
	}
}

////////////////////////////////////////////////////
/// RenderingControl methods

// State variables for the RenderingControl. All evented through LastChange
//  PresetNameList
//  Mute
//  Volume
//  VolumeDB
// LastChange contains all the variables that were changed since the last
// event. For us that's at most Mute, Volume, VolumeDB
// <Event xmlns=”urn:schemas-upnp-org:metadata-1-0/AVT_RCS">
//   <InstanceID val=”0”>
//     <Mute channel=”Master” val=”0”/>
//     <Volume channel=”Master” val=”24”/>
//     <VolumeDB channel=”Master” val=”24”/>
//   </InstanceID>
// </Event>
bool UpMpd::getEventDataRendering(std::vector<std::string>& names, 
								  std::vector<std::string>& values)
{
	names.push_back("LastChange");

	int volume = m_mpdcli->getVolume();
	char cvalue[40];
	string 
		chgdata("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT_RCS\">\n"
				"<InstanceID val=\"0\">\n");

	chgdata  += "<Mute channel=\"Master\" val=\"";
    chgdata += volume == 0 ? "1" : "0";
	chgdata += "\"/>\n";

	chgdata += "<Volume channel=\"Master\" val=\"";
	sprintf(cvalue, "%d", volume);
	chgdata += cvalue;
	chgdata += "\"/>\n";

	chgdata += "<VolumeDB channel=\"Master\" val=\"";
	sprintf(cvalue, "%d", percentodbvalue(volume));
	chgdata += cvalue;
	chgdata += "\"/>\n";

	chgdata += "<PresetNameList val=\"FactoryDefault\"/>\n";

	chgdata += "</InstanceID>\n</Event>\n";

	values.push_back(chgdata);

	cerr << "UpMpd::getEventDataRendering: " << chgdata << endl;
	return true;
}

void UpMpd::doNotifyEventRendering()
{
	std::vector<std::string> names;
	std::vector<std::string> values;
	getEventDataRendering(names, values);
	notifyEvent(serviceIdRender, names, values);
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
		renderingUpdate("Mute", "0");
	} else if (it->second[0] == 'T' || it->second[0] == '1') {
		m_mpdcli->setVolume(0);
		renderingUpdate("Mute", "1");
	} else {
		return UPNP_E_INVALID_PARAM;
	}
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

	char cvalue[30];
	sprintf(cvalue, "%d", volume);
	renderingUpdate("Volume", cvalue);
	sprintf(cvalue, "%d", percentodbvalue(volume));
	renderingUpdate("VolumeDB", cvalue);

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
	int volume = 50;
	m_mpdcli->setVolume(volume);
	char cvalue[30];
	sprintf(cvalue, "%d", volume);
	renderingUpdate("Volume", cvalue);
	sprintf(cvalue, "%d", percentodbvalue(volume));
	renderingUpdate("VolumeDB", cvalue);

	return UPNP_E_SUCCESS;
}

///////////////// AVTransport methods

bool UpMpd::getEventDataTransport(std::vector<std::string>& names, 
								  std::vector<std::string>& values)
{
	return true;
}

// Some state variables do not generate events and must be polled by
// the control point: RelativeTimePosition AbsoluteTimePosition
// RelativeCounterPosition AbsoluteCounterPosition.
// This leaves us with:
//    TransportState
//    TransportStatus
//    PlaybackStorageMedium
//    PossiblePlaybackStorageMedia
//    RecordStorageMedium
//    PossibleRecordStorageMedia
//    CurrentPlayMode
//    TransportPlaySpeed
//    RecordMediumWriteStatus
//    CurrentRecordQualityMode
//    PossibleRecordQualityModes
//    NumberOfTracks
//    CurrentTrack
//    CurrentTrackDuration
//    CurrentMediaDuration
//    CurrentTrackMetaData
//    CurrentTrackURI
//    AVTransportURI
//    AVTransportURIMetaData
//    NextAVTransportURI
//    NextAVTransportURIMetaData
//    RelativeTimePosition
//    AbsoluteTimePosition
//    RelativeCounterPosition
//    AbsoluteCounterPosition
//    CurrentTransportActions
//    LastChange

void UpMpd::doNotifyEventTransport()
{
	std::vector<std::string> names;
	std::vector<std::string> values;
	getEventDataTransport(names, values);
	notifyEvent(serviceIdTransport, names, values);
}

// http://192.168.4.4:8200/MediaItems/246.mp3
int UpMpd::setAVTransportURI(const SoapArgs& sc, SoapData& data, bool setnext)
{
	cerr << "UpMpd::setAVTransportURI(" << setnext << ") " << endl;

	map<string, string>::const_iterator it;
		
	it = setnext? sc.args.find("NextURI") : sc.args.find("CurrentURI");
	if (it == sc.args.end() || it->second.empty()) {
		return UPNP_E_INVALID_PARAM;
	}
	string uri = it->second;
	string metadata;
	it = setnext? sc.args.find("NextURIMetaData") : 
		sc.args.find("CurrentURIMetaData");
	if (it != sc.args.end())
		metadata = it->second;

	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	bool is_song = (mpds.state == MpdStatus::MPDS_PLAY) || 
		(mpds.state == MpdStatus::MPDS_PAUSE);
	int curpos = mpds.songpos;

	if (curpos == -1 && setnext) {
		cerr << "setNextAVTRansportURI invoked but no current!" << endl;
		return UPNP_E_INVALID_PARAM;
	}

	if (m_mpdcli->insert(uri, curpos+1) < 0) {
		return UPNP_E_INTERNAL_ERROR;
	}

	if (!setnext) {
		MpdStatus::State st = mpds.state;
		m_mpdcli->next();
		switch (st) {
		case MpdStatus::MPDS_PAUSE: m_mpdcli->togglePause();break;
		case MpdStatus::MPDS_STOP: m_mpdcli->stop();break;
		}
	}

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
		data.data.push_back(pair<string,string>("TrackDuration", "00:00:00"));
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
		data.data.push_back(pair<string,string>("TrackURI", 
												xmlquote(it->second)));
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

int UpMpd::getTransportInfo(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	cerr << "UpMpd::getTransportInfo. State: " << mpds.state << endl;

	string tstate("STOPPED");
	switch(mpds.state) {
	case MpdStatus::MPDS_PLAY: tstate = "PLAYING"; break;
	case MpdStatus::MPDS_PAUSE: tstate = "PAUSED_PLAYBACK"; break;
	}
	data.data.push_back(pair<string,string>("CurrentTransportState", tstate));
	data.data.push_back(pair<string,string>("CurrentTransportStatus", 
											m_mpdcli->ok() ? 
											"OK" : "ERROR_OCCURRED"));
	data.data.push_back(pair<string,string>("CurrentSpeed", "1"));
	return UPNP_E_SUCCESS;
}

int UpMpd::getDeviceCapabilities(const SoapArgs& sc, SoapData& data)
{
	data.data.push_back(pair<string,string>("PlayMedia", "NETWORK,HDD"));
	data.data.push_back(pair<string,string>("RecMedia", "NOT_IMPLEMENTED"));
	data.data.push_back(pair<string,string>("RecQualityModes", 
											"NOT_IMPLEMENTED"));
	return UPNP_E_SUCCESS;
}


int UpMpd::getMediaInfo(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	cerr << "UpMpd::getMediaInfo. State: " << mpds.state << endl;

	bool is_song = (mpds.state == MpdStatus::MPDS_PLAY) || 
		(mpds.state == MpdStatus::MPDS_PAUSE);

	data.data.push_back(pair<string,string>("NrTracks", "1"));
	if (is_song) {
		data.data.push_back(pair<string,string>("MediaDuration", 
												upnpduration(mpds.songlenms)));
	} else {
		data.data.push_back(pair<string,string>("MediaDuration", "00:00:00"));
	}
	map<string, string>::const_iterator it = mpds.currentsong.find("uri");
	string thisuri;
	if (is_song && it != mpds.currentsong.end()) {
		thisuri = it->second;
		data.data.push_back(pair<string,string>("CurrentURI", 
												xmlquote(thisuri)));
	} else {
		data.data.push_back(pair<string,string>("CurrentURI", ""));
	}
	if (is_song) {
		data.data.push_back(pair<string,string>("CurrentURIMetaData", 
												didlmake(mpds)));
	} else {
		data.data.push_back(pair<string,string>("CurrentURIMetaData", ""));
	}
	data.data.push_back(pair<string,string>("NextURI", "NOT_IMPLEMENTED"));
	data.data.push_back(pair<string,string>("NextURIMetaData", 
											"NOT_IMPLEMENTED"));
	string playmedium("NONE");
	if (is_song)
		playmedium = thisuri.find("http://") == 0 ?	"HDD" : "NETWORK";
	data.data.push_back(pair<string,string>("PlayMedium", playmedium));

	data.data.push_back(pair<string,string>("RecordMedium", "NOT_IMPLEMENTED"));
	data.data.push_back(pair<string,string>("WriteStatus", "NOT_IMPLEMENTED"));
	return UPNP_E_SUCCESS;
}


int UpMpd::playcontrol(const SoapArgs& sc, SoapData& data, int what)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	cerr << "UpMpd::playcontrol State: " << mpds.state << " what "<<what<< endl;
	if ((what & ~0x3)) {
		cerr << "UpMPd::playcontrol: bad control " << what << endl;
		return UPNP_E_INVALID_PARAM;
	}

	bool ok = true;
	switch (mpds.state) {
	case MpdStatus::MPDS_PLAY: 
		switch (what) {
		case 0:	ok = m_mpdcli->stop(); break;
		case 1:break;
		case 2: ok = m_mpdcli->togglePause();break;
		}
		break;
	case MpdStatus::MPDS_PAUSE:
		switch (what) {
		case 0:	ok = m_mpdcli->stop(); break;
		case 1: ok = m_mpdcli->togglePause();break;
		case 2: break;
		}
		break;
	case MpdStatus::MPDS_STOP:
	default:
		switch (what) {
		case 0:	break;
		case 1: ok = m_mpdcli->play();break;
		case 2: break;
		}
		break;
	}
	
	return ok ? UPNP_E_SUCCESS : UPNP_E_INTERNAL_ERROR;
}


int UpMpd::seqcontrol(const SoapArgs& sc, SoapData& data, int what)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	cerr << "UpMpd::seqcontrol State: " << mpds.state << " what "<<what<< endl;
	if ((what & ~0x1)) {
		cerr << "UpMPd::seqcontrol: bad control " << what << endl;
		return UPNP_E_INVALID_PARAM;
	}

	bool ok = true;
	switch (what) {
	case 0: ok = m_mpdcli->next();break;
	case 1: ok = m_mpdcli->previous();break;
	}
	return ok ? UPNP_E_SUCCESS : UPNP_E_INTERNAL_ERROR;
}
	
int UpMpd::setPlayMode(const SoapArgs& sc, SoapData& data)
{
	map<string, string>::const_iterator it;
		
	it = sc.args.find("NewPlayMode");
	if (it == sc.args.end() || it->second.empty()) {
		return UPNP_E_INVALID_PARAM;
	}
	string playmode(it->second);
	bool ok;
	if (!playmode.compare("NORMAL")) {
		ok = m_mpdcli->repeat(false) && m_mpdcli->random(false) &&
			m_mpdcli->single(false);
	} else if (!playmode.compare("SHUFFLE")) {
		ok = m_mpdcli->repeat(false) && m_mpdcli->random(true) &&
			m_mpdcli->single(false);
	} else if (!playmode.compare("REPEAT_ONE")) {
		ok = m_mpdcli->repeat(true) && m_mpdcli->random(false) &&
			m_mpdcli->single(true);
	} else if (!playmode.compare("REPEAT_ALL")) {
		ok = m_mpdcli->repeat(true) && m_mpdcli->random(false) &&
			m_mpdcli->single(false);
	} else if (!playmode.compare("RANDOM")) {
		ok = m_mpdcli->repeat(true) && m_mpdcli->random(true) &&
			m_mpdcli->single(false);
	} else if (!playmode.compare("DIRECT_1")) {
		ok = m_mpdcli->repeat(false) && m_mpdcli->random(false) &&
			m_mpdcli->single(true);
	} else {
		return UPNP_E_INVALID_PARAM;
	}
	return ok ? UPNP_E_SUCCESS : UPNP_E_INTERNAL_ERROR;
}

int UpMpd::getTransportSettings(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	string playmode = "NORMAL";
    if (!mpds.rept && mpds.random && !mpds.single)
		playmode = "SHUFFLE";
	else if (mpds.rept && !mpds.random && mpds.single)
		playmode = "REPEAT_ONE";
	else if (mpds.rept && !mpds.random && !mpds.single)
		playmode = "REPEAT_ALL";
	else if (mpds.rept && mpds.random && !mpds.single)
		playmode = "RANDOM";
	else if (!mpds.rept && !mpds.random && mpds.single)
		playmode = "DIRECT_1";

	data.data.push_back(pair<string,string>("PlayMode", playmode.c_str()));
	data.data.push_back(pair<string,string>("RecQualityMode",
											"NOT_IMPLEMENTED"));
	return UPNP_E_SUCCESS;
}

int UpMpd::getCurrentTransportActions(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	// 
	data.data.push_back(pair<string,string>("CurrentTransportActions", 
										"Play,Stop,Pause,Seek,Next,Previous"));
	return UPNP_E_SUCCESS;
}

// Not implemented for now
int UpMpd::seek(const SoapArgs& sc, SoapData& data)
{
	map<string, string>::const_iterator it;
		
	it = sc.args.find("Unit");
	if (it == sc.args.end() || it->second.empty()) {
		return UPNP_E_INVALID_PARAM;
	}
	string unit(it->second);

	it = sc.args.find("Target");
	if (it == sc.args.end() || it->second.empty()) {
		return UPNP_E_INVALID_PARAM;
	}
	string target(it->second);

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
