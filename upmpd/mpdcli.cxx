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
#include <memory.h>

#include <iostream>

#include <mpd/connection.h>
#include <mpd/password.h>
#include <mpd/mixer.h>
#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/tag.h>
#include <mpd/player.h>
#include <mpd/queue.h>

#include "mpdcli.hxx"

using namespace std;

#define M_CONN ((struct mpd_connection *)m_conn)

MPDCli::MPDCli(const string& host, int port, const string& pass)
    : m_conn(0), m_premutevolume(0), 
      m_host(host), m_port(port), m_password(pass)
{
    cerr << "MPDCli::MPDCli" << endl;
    if (!openconn()) {
        return;
    }
    m_ok = true;
    updStatus();
}

MPDCli::~MPDCli()
{
    if (m_conn) 
        mpd_connection_free(M_CONN);
}

bool MPDCli::openconn()
{
    if (m_conn) {
        mpd_connection_free(M_CONN);
        m_conn = 0;
    }
    m_conn = mpd_connection_new(m_host.c_str(), m_port, 0);
    if (m_conn == NULL) {
        cerr << "mpd_connection_new failed. No memory?" << endl;
        return false;
    }

    if (mpd_connection_get_error(M_CONN) != MPD_ERROR_SUCCESS) {
        showError("MPDCli::openconn");
        return false;
    }

    if(!m_password.empty()) {
        if (!mpd_run_password(M_CONN, m_password.c_str())) {
            cerr << "Password wrong" << endl;
            return false;
        }
    }
    return true;
}

bool MPDCli::showError(const string& who)
{
    if (!ok()) {
        cerr << "MPDCli::showError: bad state" << endl;
        return false;
    }

    cerr << who << " failed: " <<  mpd_connection_get_error_message(M_CONN);
    int error = mpd_connection_get_error(M_CONN);
    if (error == MPD_ERROR_SERVER) {
        cerr << " server error: " << mpd_connection_get_server_error(M_CONN) ;
    }
    cerr << endl;
    if (error == MPD_ERROR_CLOSED)
        if (openconn())
            return true;
    return false;
}

bool MPDCli::updStatus()
{
    if (!ok()) {
        cerr << "MPDCli::updStatus: bad state" << endl;
        return false;
    }

    mpd_status *mpds = 0;
    for (int i = 0; i < 2; i++) {
        if (mpds = mpd_run_status(M_CONN))
            break;
        if (i == 1 || !showError("MPDCli::updStatus"))
            return false;
    }

    m_stat.volume = mpd_status_get_volume(mpds);
    m_stat.rept = mpd_status_get_repeat(mpds);
    m_stat.random = mpd_status_get_random(mpds);
    m_stat.single = mpd_status_get_single(mpds);
    m_stat.consume = mpd_status_get_consume(mpds);
    m_stat.qlen = mpd_status_get_queue_length(mpds);
    m_stat.qvers = mpd_status_get_queue_version(mpds);
    switch (mpd_status_get_state(mpds)) {
    case MPD_STATE_UNKNOWN: m_stat.state = MpdStatus::MPDS_UNK;break;
    case MPD_STATE_STOP: m_stat.state = MpdStatus::MPDS_STOP;break;
    case MPD_STATE_PLAY: m_stat.state = MpdStatus::MPDS_PLAY;break;
    case MPD_STATE_PAUSE: m_stat.state = MpdStatus::MPDS_PAUSE;break;
    }


    m_stat.crossfade = mpd_status_get_crossfade(mpds);
    m_stat.mixrampdb = mpd_status_get_mixrampdb(mpds);
    m_stat.mixrampdelay = mpd_status_get_mixrampdelay(mpds);
    m_stat.songpos = mpd_status_get_song_pos(mpds);
    m_stat.songid = mpd_status_get_song_id(mpds);
    if (m_stat.songpos >= 0) {
        updSong(m_stat.currentsong);
        updSong(m_stat.nextsong, m_stat.songpos + 1);
    }

    m_stat.songelapsedms = mpd_status_get_elapsed_ms(mpds);
    m_stat.songlenms = mpd_status_get_total_time(mpds) * 1000;
    m_stat.kbrate = mpd_status_get_kbit_rate(mpds);

    const char *err = mpd_status_get_error(mpds);
    if (err != 0)
        m_stat.errormessage.assign(err);

    mpd_status_free(mpds);
    return true;
}

bool MPDCli::updSong(map<string, string>& tsong, int pos)
{
    // cerr << "MPDCli::updSong" << endl;
    tsong.clear();
    if (!ok())
        return false;

    struct mpd_song *song = pos == -1 ? mpd_run_current_song(M_CONN) :
        mpd_run_get_queue_song_pos(M_CONN, (unsigned int)pos);
        
    if (song == 0) {
        cerr << "mpd_run_current_song failed" << endl;
        return false;
    }

    const char *cp;
    cp = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
    if (cp != 0)
        tsong["upnp:artist"] = cp;

    cp = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
    if (cp != 0)
        tsong["upnp:album"] = cp;

    cp = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
    if (cp != 0) {
        tsong["dc:title"] = cp;
    }

    cp = mpd_song_get_tag(song, MPD_TAG_TRACK, 0);
    if (cp != 0)
        tsong["upnp:originalTrackNumber"] = cp;

    cp = mpd_song_get_tag(song, MPD_TAG_GENRE, 0);
    if (cp != 0)
        tsong["upnp:genre"] = cp;
    
    cp = mpd_song_get_uri(song);
    if (cp != 0)
        tsong["uri"] = cp;

    mpd_song_free(song);
    return true;
}

bool MPDCli::setVolume(int volume, bool relative)
{
    if (!ok()) {
        return false;
    }
    cerr << "setVolume: vol " << volume << " relative " << relative << endl;
    if (volume == 0) {
        if (relative) {
            // Restore premute volume
            m_stat.volume = m_premutevolume;
            cerr << "Restoring premute" << endl;
        } else {
            updStatus();
            if (m_stat.volume != 0) {
                cerr << "Saving premute: " << m_stat.volume << endl;
                m_premutevolume = m_stat.volume;
            }
        }
    }
        
    if (relative)
        volume += m_stat.volume;

    if (volume < 0)
        volume = 0;
    else if (volume > 100)
        volume = 100;
    
    for (int i = 0; i < 2; i++) {
        if (mpd_run_set_volume(M_CONN, volume))
            break;
        if (i == 1 || !showError("MPDCli::updStatus"))
            return false;
    }

    return updStatus();
}

#define RETRY_CMD(CMD) {                                \
    for (int i = 0; i < 2; i++) {                       \
        if (CMD)                                        \
            break;                                      \
        if (i == 1 || !showError(#CMD))                 \
            return false;                               \
    }                                                   \
    }

int MPDCli::getVolume()
{
    if (!updStatus())
        return -1;
    return m_stat.volume == -1 ? 0: m_stat.volume;
}

bool MPDCli::togglePause()
{
    cerr << "MPDCli::togglePause" << endl;
    if (!ok())
        return false;
    RETRY_CMD(mpd_run_toggle_pause(M_CONN));
    return true;
}

bool MPDCli::play()
{
    cerr << "MPDCli::play" << endl;
    if (!ok())
        return false;
    RETRY_CMD(mpd_run_play(M_CONN));
    return true;
}
bool MPDCli::stop()
{
    cerr << "MPDCli::stop" << endl;
    if (!ok())
        return false;
    RETRY_CMD(mpd_run_stop(M_CONN));
    return true;
}
bool MPDCli::next()
{
    cerr << "MPDCli::next" << endl;
    if (!ok())
        return false;
    RETRY_CMD(mpd_run_next(M_CONN));
    return true;
}
bool MPDCli::previous()
{
    cerr << "MPDCli::previous" << endl;
    if (!ok())
        return false;
    RETRY_CMD(mpd_run_previous(M_CONN));
    return true;
}
bool MPDCli::repeat(bool on)
{
    cerr << "MPDCli::repeat:" << on << endl;
    if (!ok())
        return false;
    RETRY_CMD(mpd_run_repeat(M_CONN, on));
    return true;
}
bool MPDCli::random(bool on)
{
    cerr << "MPDCli::random:" << on << endl;
    if (!ok())
        return false;
    RETRY_CMD(mpd_run_random(M_CONN, on));
    return true;
}
bool MPDCli::single(bool on)
{
    cerr << "MPDCli::single:" << on << endl;
    if (!ok())
        return false;
    RETRY_CMD(mpd_run_single(M_CONN, on));
    return true;
}

int MPDCli::insert(const string& uri, int pos)
{
    cerr << "MPDCli::insert at :" << pos << " uri " << uri << endl;
    if (!ok())
        return -1;

    if (!updStatus())
        return -1;

    int id = mpd_run_add_id_to(M_CONN, uri.c_str(), (unsigned)pos);

    if (id < 0) {
        showError("MPDCli::run_add_id");
        return -1;
    }
    return id;
}
int MPDCli::curpos()
{
    if (!updStatus())
        return -1;
    cerr << "MPDCli::curpos: pos: " << m_stat.songpos << " id " 
         << m_stat.songid << endl;
    return m_stat.songpos;
}
