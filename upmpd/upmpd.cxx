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
#include <set>
using namespace std;
using namespace std::placeholders;

#include "libupnpp/upnpplib.hxx"
#include "libupnpp/soaphelp.hxx"
#include "libupnpp/device.hxx"
#include "libupnpp/log.hxx"

#include "mpdcli.hxx"
#include "upmpdutils.hxx"

static const string friendlyName("UpMpd");

// The UPnP MPD frontend device with its 2 services
class UpMpd : public UpnpDevice {
public:
	UpMpd(const string& deviceid, const unordered_map<string, string>& xmlfiles,
		  MPDCli *mpdcli);

	// RenderingControl
	int setMute(const SoapArgs& sc, SoapData& data);
	int getMute(const SoapArgs& sc, SoapData& data);
	int setVolume(const SoapArgs& sc, SoapData& data, bool isDb);
	int getVolume(const SoapArgs& sc, SoapData& data, bool isDb);
	int listPresets(const SoapArgs& sc, SoapData& data);
	int selectPreset(const SoapArgs& sc, SoapData& data);
	int getVolumeDBRange(const SoapArgs& sc, SoapData& data);
    virtual bool getEventDataRendering(bool all, 
									   std::vector<std::string>& names, 
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
    virtual bool getEventDataTransport(bool all, 
									   std::vector<std::string>& names, 
									   std::vector<std::string>& values);

	// Shared
    virtual bool getEventData(bool all, const string& serviceid, 
							  std::vector<std::string>& names, 
							  std::vector<std::string>& values);

private:
	MPDCli *m_mpdcli;
	// State variable storage
	unordered_map<string, string> m_rdupdates;
	unordered_map<string, string> m_tpstate;

	// My track identifiers (for cleaning up)
	set<int> m_songids;

	// The two services use different methods for recording changed
	// state: The RenderingControl actions record changes one at a
	// time while the AVTransport recomputes the whole state by
	// querying MPD and runs a diff with the previous state

	// Record RenderingControl state change
	void renderingUpdate(const string& nm, const string& val) {
		m_rdupdates[nm] = val;
	}

	// Translate MPD state to AVTransport state variables.
	bool tpstateMToU(unordered_map<string, string>& state);
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
	{	auto bound = bind(&UpMpd::seek, this, _1, _2);
		addActionMapping("Seek", bound);
	}
	{	auto bound = bind(&UpMpd::seqcontrol, this, _1, _2, 0);
		addActionMapping("Next", bound);
	}
	{	auto bound = bind(&UpMpd::seqcontrol, this, _1, _2, 1);
		addActionMapping("Previous", bound);
	}
}

// This is called at regular intervals by the polling loop to retrieve
// changed state variables for each of the services (the list of
// services was defined in the base class by the "addServiceTypes()"
// calls during construction). 
//
// We might add a method for triggering an event from the action
// methods after changing state, which would really act only if the
// interval with the previous event is long enough. But things seem to
// work ok with the systematic delay.
bool UpMpd::getEventData(bool all, const string& serviceid, 
						 std::vector<std::string>& names, 
						 std::vector<std::string>& values)
{
	if (!serviceid.compare(serviceIdRender)) {
		return getEventDataRendering(all, names, values);
	} else if (!serviceid.compare(serviceIdTransport)) {
		return getEventDataTransport(all, names, values);
	} else {
		LOGERR("UpMpd::getEventData: servid? [" << serviceid << "]" << endl);
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
bool UpMpd::getEventDataRendering(bool all, std::vector<std::string>& names, 
								  std::vector<std::string>& values)
{
	if (all) {
		// Record all values in the changes structure.
		int volume = m_mpdcli->getVolume();
		renderingUpdate("Mute", volume == 0 ? "1" : "0");
		char cvalue[40];
		sprintf(cvalue, "%d", volume);
		renderingUpdate("Volume", cvalue);
		sprintf(cvalue, "%d", percentodbvalue(volume));
		renderingUpdate("VolumeDB", cvalue);
		renderingUpdate("PresetNameList", "FactoryDefault");
	}

	if (m_rdupdates.empty())
		return true;

	names.push_back("LastChange");

	string 
		chgdata("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT_RCS\">\n"
				"<InstanceID val=\"0\">\n");
	for (unordered_map<string, string>::const_iterator it = m_rdupdates.begin();
		 it != m_rdupdates.end(); it++) {
		chgdata += "<";
		chgdata += it->first;
		chgdata += " channel=\"Master\" val=\"";
		chgdata += xmlquote(it->second);
		chgdata += "\"/>\n";
	}
	chgdata += "</InstanceID>\n</Event>\n";

	values.push_back(chgdata);
	m_rdupdates.clear();
	return true;
}

// Actions:
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


int UpMpd::getVolumeDBRange(const SoapArgs& sc, SoapData& data)
{
	map<string, string>::const_iterator it;

	it = sc.args.find("Channel");
	if (it == sc.args.end() || it->second.compare("Master")) {
		return UPNP_E_INVALID_PARAM;
	}
	data.addarg("MinValue", "-10240");
	data.addarg("MaxValue", "0");

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
	loopWakeup();
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
	data.addarg("CurrentMute", volume == 0 ? "1" : "0");
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

	loopWakeup();
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
	data.addarg("CurrentVolume", svolume);
	return UPNP_E_SUCCESS;
}

int UpMpd::listPresets(const SoapArgs& sc, SoapData& data)
{
	// The 2nd arg is a comma-separated list of preset names
	data.addarg("CurrentPresetNameList", "FactoryDefaults");
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

// Translate MPD mode flags to UPnP Play mode
static string mpdsToPlaymode(const struct MpdStatus& mpds)
{
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
	return playmode;
}

// AVTransport eventing
// 
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
//
// To be all bundled inside:    LastChange

// Translate MPD state to UPnP AVTRansport state variables
bool UpMpd::tpstateMToU(unordered_map<string, string>& status)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	//DEBOUT << "UpMpd::tpstateMToU: curpos: " << mpds.songpos <<
	//   " qlen " << mpds.qlen << endl;
	bool is_song = (mpds.state == MpdStatus::MPDS_PLAY) || 
		(mpds.state == MpdStatus::MPDS_PAUSE);

	string tstate("STOPPED");
	string tactions("Next,Previous");
	switch(mpds.state) {
	case MpdStatus::MPDS_PLAY: 
		tstate = "PLAYING"; 
		tactions += ",Pause,Stop,Seek";
		break;
	case MpdStatus::MPDS_PAUSE: 
		tstate = "PAUSED_PLAYBACK"; 
		tactions += ",Play,Stop,Seek";
		break;
	default:
		tactions += ",Play";
	}
	status["TransportState"] = tstate;
	status["CurrentTransportActions"] = tactions;
	status["TransportStatus"] = m_mpdcli->ok() ? "OK" : "ERROR_OCCURRED";
	status["TransportPlaySpeed"] = "1";

	const string& uri = mapget(mpds.currentsong, "uri");
	status["CurrentTrack"] = "1";
	status["CurrentTrackURI"] = uri;
	status["CurrentTrackMetaData"] = is_song?didlmake(mpds) : "";
	string playmedium("NONE");
	if (is_song)
		playmedium = uri.find("http://") == 0 ?	"HDD" : "NETWORK";
	status["NumberOfTracks"] = "1";
	status["CurrentMediaDuration"] = is_song?
		upnpduration(mpds.songlenms):"00:00:00";
	status["CurrentTrackDuration"] = is_song?
		upnpduration(mpds.songlenms):"00:00:00";
	status["AVTransportURI"] = uri;
	status["AVTransportURIMetaData"] = is_song?didlmake(mpds) : "";
	status["RelativeTimePosition"] = is_song?
		upnpduration(mpds.songelapsedms):"0:00:00";
	status["AbsoluteTimePosition"] = is_song?
		upnpduration(mpds.songelapsedms) : "0:00:00";

	status["NextAVTransportURI"] = mapget(mpds.nextsong, "uri");
	status["NextAVTransportURIMetaData"] = is_song?didlmake(mpds, true) : "";

	status["PlaybackStorageMedium"] = playmedium;
	status["PossiblePlaybackStorageMedium"] = "HDD,NETWORK";
	status["RecordStorageMedium"] = "NOT_IMPLEMENTED";
	status["RelativeCounterPosition"] = "0";
	status["AbsoluteCounterPosition"] = "0";
	status["CurrentPlayMode"] = mpdsToPlaymode(mpds);

	status["PossibleRecordStorageMedium"] = "NOT_IMPLEMENTED";
	status["RecordMediumWriteStatus"] = "NOT_IMPLEMENTED";
	status["CurrentRecordQualityMode"] = "NOT_IMPLEMENTED";
	status["PossibleRecordQualityModes"] = "NOT_IMPLEMENTED";
	return true;
}

bool UpMpd::getEventDataTransport(bool all, std::vector<std::string>& names, 
								  std::vector<std::string>& values)
{
	unordered_map<string, string> newtpstate;
	tpstateMToU(newtpstate);
	if (all)
		m_tpstate.clear();

	bool changefound = false;

	string 
		chgdata("<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT_RCS\">\n"
				"<InstanceID val=\"0\">\n");
	for (unordered_map<string, string>::const_iterator it = newtpstate.begin();
		 it != newtpstate.end(); it++) {

		const string& oldvalue = mapget(m_tpstate, it->first);
		if (!it->second.compare(oldvalue))
			continue;

		if (it->first.compare("RelativeTimePosition") && 
			it->first.compare("AbsoluteTimePosition")) {
			//DEBOUT << "Transport state update for " << it->first << 
			// " oldvalue [" << oldvalue << "] -> [" << it->second << endl;
			changefound = true;
		}

		chgdata += "<";
		chgdata += it->first;
		chgdata += " val=\"";
		chgdata += xmlquote(it->second);
		chgdata += "\"/>\n";
	}
	chgdata += "</InstanceID>\n</Event>\n";

	if (!changefound) {
		// DEBOUT << "UpMpd::getEventDataTransport: no updates" << endl;
		return true;
	}

	names.push_back("LastChange");
	values.push_back(chgdata);

	m_tpstate = newtpstate;
	// DEBOUT << "UpMpd::getEventDataTransport: " << chgdata << endl;
	return true;
}

// http://192.168.4.4:8200/MediaItems/246.mp3
int UpMpd::setAVTransportURI(const SoapArgs& sc, SoapData& data, bool setnext)
{
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
	LOGDEB("UpMpd::set" << (setnext?"Next":"") << 
		   "AVTransportURI: curpos: " <<
		   curpos << " is_song " << is_song << " qlen " << mpds.qlen << endl);

	// curpos == -1 means that the playlist was cleared or we just started. A
	// play will use position 0, so it's actually equivalent to curpos == 0
	if (curpos == -1)
		curpos = 0;

	if (mpds.qlen == 0 && setnext) {
		LOGDEB("setNextAVTRansportURI invoked but empty queue!" << endl);
		return UPNP_E_INVALID_PARAM;
	}
	int songid;
	if ((songid = m_mpdcli->insert(uri, setnext?curpos+1:curpos)) < 0) {
		return UPNP_E_INTERNAL_ERROR;
	}
	if (!setnext) {
		MpdStatus::State st = mpds.state;
		// Have to tell mpd which track to play, else it will keep on
		// the previous despite of the insertion. The UPnP docs say
		// that setAVTransportURI should not change the transport
		// state (pause/stop stay pause/stop) but it seems that some clients
		// expect that the track will start playing so we let it play. 
		// Needs to be revisited after seeing more clients. For now try to 
		// preserve state as per standard.
		// Audionet: issues a Play
		// BubbleUpnp: issues a Play
		// MediaHouse: no setnext, Play
		m_mpdcli->play(curpos);
#if 1 || defined(upmpd_do_restore_play_state_after_add)
		switch (st) {
		case MpdStatus::MPDS_PAUSE: m_mpdcli->togglePause(); break;
		case MpdStatus::MPDS_STOP: m_mpdcli->stop(); break;
		}
#endif
		// Clean up old song ids
		for (set<int>::iterator it = m_songids.begin();
			 it != m_songids.end(); it++) {
			// Can't just delete here. If the id does not exist, MPD 
			// gets into an apparently permanent error state, where even 
			// get_status does not work
			if (m_mpdcli->statId(*it)) {
				m_mpdcli->deleteId(*it);
			}
		}
		m_songids.clear();
	}

	m_songids.insert(songid);
	loopWakeup();
	return UPNP_E_SUCCESS;
}

int UpMpd::getPositionInfo(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	LOGDEB("UpMpd::getPositionInfo. State: " << mpds.state << endl);

	bool is_song = (mpds.state == MpdStatus::MPDS_PLAY) || 
		(mpds.state == MpdStatus::MPDS_PAUSE);

	if (is_song) {
		data.addarg("Track", "1");
	} else {
		data.addarg("Track", "0");
	}

	if (is_song) {
		data.addarg("TrackDuration", upnpduration(mpds.songlenms));
	} else {
		data.addarg("TrackDuration", "00:00:00");
	}

	if (is_song) {
		data.addarg("TrackMetaData", didlmake(mpds));
	} else {
		data.addarg("TrackMetaData", "");
	}

	const string& uri = mapget(mpds.currentsong, "uri");
	if (is_song && !uri.empty()) {
		data.addarg("TrackURI", xmlquote(uri));
	} else {
		data.addarg("TrackURI", "");
	}
	if (is_song) {
		data.addarg("RelTime", upnpduration(mpds.songelapsedms));
	} else {
		data.addarg("RelTime", "0:00:00");
	}

	if (is_song) {
		data.addarg("AbsTime", upnpduration(mpds.songelapsedms));
	} else {
		data.addarg("AbsTime", "0:00:00");
	}

	data.addarg("RelCount", "0");
	data.addarg("AbsCount", "0");
	return UPNP_E_SUCCESS;
}

int UpMpd::getTransportInfo(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	LOGDEB("UpMpd::getTransportInfo. State: " << mpds.state << endl);

	string tstate("STOPPED");
	switch(mpds.state) {
	case MpdStatus::MPDS_PLAY: tstate = "PLAYING"; break;
	case MpdStatus::MPDS_PAUSE: tstate = "PAUSED_PLAYBACK"; break;
	}
	data.addarg("CurrentTransportState", tstate);
	data.addarg("CurrentTransportStatus", m_mpdcli->ok() ? "OK" : 
				"ERROR_OCCURRED");
	data.addarg("CurrentSpeed", "1");
	return UPNP_E_SUCCESS;
}

int UpMpd::getDeviceCapabilities(const SoapArgs& sc, SoapData& data)
{
	data.addarg("PlayMedia", "NETWORK,HDD");
	data.addarg("RecMedia", "NOT_IMPLEMENTED");
	data.addarg("RecQualityModes", "NOT_IMPLEMENTED");
	return UPNP_E_SUCCESS;
}

int UpMpd::getMediaInfo(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	LOGDEB("UpMpd::getMediaInfo. State: " << mpds.state << endl);

	bool is_song = (mpds.state == MpdStatus::MPDS_PLAY) || 
		(mpds.state == MpdStatus::MPDS_PAUSE);

	data.addarg("NrTracks", "1");
	if (is_song) {
		data.addarg("MediaDuration", upnpduration(mpds.songlenms));
	} else {
		data.addarg("MediaDuration", "00:00:00");
	}

	const string& thisuri = mapget(mpds.currentsong, "uri");
	if (is_song && !thisuri.empty()) {
		data.addarg("CurrentURI", xmlquote(thisuri));
	} else {
		data.addarg("CurrentURI", "");
	}
	if (is_song) {
		data.addarg("CurrentURIMetaData", didlmake(mpds));
	} else {
		data.addarg("CurrentURIMetaData", "");
	}
	data.addarg("NextURI", "NOT_IMPLEMENTED");
	data.addarg("NextURIMetaData", "NOT_IMPLEMENTED");
	string playmedium("NONE");
	if (is_song)
		playmedium = thisuri.find("http://") == 0 ?	"HDD" : "NETWORK";
	data.addarg("PlayMedium", playmedium);

	data.addarg("RecordMedium", "NOT_IMPLEMENTED");
	data.addarg("WriteStatus", "NOT_IMPLEMENTED");
	return UPNP_E_SUCCESS;
}

int UpMpd::playcontrol(const SoapArgs& sc, SoapData& data, int what)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	LOGDEB("UpMpd::playcontrol State: " << mpds.state <<" what "<<what<< endl);

	if ((what & ~0x3)) {
		LOGERR("UpMPd::playcontrol: bad control " << what << endl);
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
	
	loopWakeup();
	return ok ? UPNP_E_SUCCESS : UPNP_E_INTERNAL_ERROR;
}

int UpMpd::seqcontrol(const SoapArgs& sc, SoapData& data, int what)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	LOGDEB("UpMpd::seqcontrol State: " << mpds.state << " what "<<what<< endl);

	if ((what & ~0x1)) {
		LOGERR("UpMPd::seqcontrol: bad control " << what << endl);
		return UPNP_E_INVALID_PARAM;
	}

	bool ok = true;
	switch (what) {
	case 0: ok = m_mpdcli->next();break;
	case 1: ok = m_mpdcli->previous();break;
	}

	sleepms(200);
	const struct MpdStatus &mpds1 = m_mpdcli->getStatus();

	string uri = mapget(mpds.currentsong, "uri");

	loopWakeup();
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
	loopWakeup();
	return ok ? UPNP_E_SUCCESS : UPNP_E_INTERNAL_ERROR;
}

int UpMpd::getTransportSettings(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	string playmode = mpdsToPlaymode(mpds);
	data.addarg("PlayMode", playmode);
	data.addarg("RecQualityMode", "NOT_IMPLEMENTED");
	return UPNP_E_SUCCESS;
}

int UpMpd::getCurrentTransportActions(const SoapArgs& sc, SoapData& data)
{
	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	string tactions("Next,Previous");
	switch(mpds.state) {
	case MpdStatus::MPDS_PLAY: 
		tactions += ",Pause,Stop,Seek";
		break;
	case MpdStatus::MPDS_PAUSE: 
		tactions += ",Play,Stop,Seek";
		break;
	default:
		tactions += ",Play";
	}
	data.addarg("CurrentTransportActions", tactions);
	return UPNP_E_SUCCESS;
}

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

	// LOGDEB("UpMpd::seek: unit " << unit << " target " << target);

	const struct MpdStatus &mpds = m_mpdcli->getStatus();
	int abs_seconds;
	if (!unit.compare("ABS_TIME")) {
		abs_seconds = upnpdurationtos(target);
	} else 	if (!unit.compare("REL_TIME")) {
		abs_seconds = mpds.songelapsedms / 1000;
		abs_seconds += upnpdurationtos(target);
//	} else 	if (!unit.compare("TRACK_NR")) {
	} else {
		return UPNP_E_INVALID_PARAM;
	}

	loopWakeup();
	return m_mpdcli->seek(abs_seconds) ? UPNP_E_SUCCESS : UPNP_E_INTERNAL_ERROR;
}


/////////////////////////////////////////////////////////////////////
// Main program
static char *thisprog;

static int op_flags;
#define OPT_MOINS 0x1
#define OPT_h	  0x2
#define OPT_p	  0x4
#define OPT_d	  0x8

static const char usage[] = 
"-h host    \t specify host MPD is running on\n"
"-i port     \t specify MPD port\n"
"-d logfilename\t debug messages to\n"
"  \n\n"
			;
static void
Usage(void)
{
	fprintf(stderr, "%s: usage:\n%s", thisprog, usage);
	exit(1);
}

static string myDeviceUUID;

static string datadir("upmpd/");

int main(int argc, char *argv[])
{
	string mpdhost("localhost");
	int mpdport = 6600;
	string upnplogfilename("/tmp/upmpd_libupnp.log");

	const char *cp;
	if (cp = getenv("UPMPD_HOST"))
		mpdhost = cp;
	if (cp = getenv("UPMPD_PORT"))
		mpdport = atoi(cp);

	thisprog = argv[0];
	argc--; argv++;
	while (argc > 0 && **argv == '-') {
		(*argv)++;
		if (!(**argv))
			Usage();
		while (**argv)
			switch (*(*argv)++) {
			case 'h':	op_flags |= OPT_h; if (argc < 2)  Usage();
				mpdhost = *(++argv); argc--;
				goto b1;
			case 'p':	op_flags |= OPT_p; if (argc < 2)  Usage();
				mpdport = atoi(*(++argv)); argc--;
				goto b1;
			default: Usage();	break;
			}
	b1: argc--; argv++;
	}

	if (argc != 0)
		Usage();

	// Initialize libupnpp, and check health
	LibUPnP *mylib = LibUPnP::getLibUPnP(true);
	if (!mylib) {
		LOGFAT(" Can't get LibUPnP" << endl);
		return 1;
	}
	if (!mylib->ok()) {
		LOGFAT("Lib init failed: " <<
			   mylib->errAsString("main", mylib->getInitError()) << endl);
		return 1;
	}
	// mylib->setLogFileName(upnplogfilename);

	// Initialize MPD client module
	MPDCli mpdcli(mpdhost, mpdport);
	if (!mpdcli.ok()) {
		LOGFAT("MPD connection failed" << endl);
		return 1;
	}
	
	// Create unique ID
	string UUID = LibUPnP::makeDevUUID(friendlyName);

	// Read our XML data.
	string reason;

	string description;
	string filename = datadir + "description.xml";
	if (!file_to_string(filename, description, &reason)) {
		LOGFAT("Failed reading " << filename << " : " << reason << endl);
		return 1;
	}
	// Update device description with UUID
    description = regsub1("@UUID@", description, UUID);

	string rdc_scdp;
	filename = datadir + "RenderingControl.xml";
	if (!file_to_string(filename, rdc_scdp, &reason)) {
		LOGFAT("Failed reading " << filename << " : " << reason << endl);
		return 1;
	}
	string avt_scdp;
	filename = datadir + "AVTransport.xml";
	if (!file_to_string(filename, avt_scdp, &reason)) {
		LOGFAT("Failed reading " << filename << " : " << reason << endl);
		return 1;
	}

	// List the XML files to be served through http (all will live in '/')
	unordered_map<string, string> xmlfiles;
	xmlfiles["description.xml"] = description;
	xmlfiles["RenderingControl.xml"] = rdc_scdp;
	xmlfiles["AVTransport.xml"] = avt_scdp;

	// Initialize the UPnP device object.
	UpMpd device(string("uuid:") + UUID, xmlfiles, &mpdcli);

	LOGDEB("Entering event loop" << endl);

	// And forever generate state change events.
	device.eventloop();

	return 0;
}

/* Local Variables: */
/* mode: c++ */
/* c-basic-offset: 4 */
/* tab-width: 4 */
/* indent-tabs-mode: t */
/* End: */
