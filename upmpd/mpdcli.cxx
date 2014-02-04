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

#include "mpdcli.hxx"

using namespace std;

#define M_CONN ((struct mpd_connection *)m_conn)

MPDCli::MPDCli(const string& host, int port, const string& pass)
    : m_premutevolume(0)
{
    cerr << "MPDCli::MPDCli" << endl;
    m_conn = mpd_connection_new(host.c_str(), port, 0);
    if (m_conn == NULL) {
        cerr << "mpd_connection_new failed. No memory?" << endl;
        return;
    }

    if (mpd_connection_get_error(M_CONN) != MPD_ERROR_SUCCESS) {
        cerr << "Mpd connection error: " << 
            mpd_connection_get_error_message(M_CONN) << endl;
        return;
    }

    if(!pass.empty()) {
        if (!mpd_run_password(M_CONN, pass.c_str())) {
            cerr << "Password wrong" << endl;
            return;
        }
    }
    m_ok = true;
    updStatus();
}

MPDCli::~MPDCli()
{
    if (m_conn) 
        mpd_connection_free(M_CONN);
}

bool MPDCli::setVolume(int volume, bool relative)
{
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
    if (!mpd_run_set_volume(M_CONN, volume)) {
        cerr << "Mpd set_volume error: " << 
            mpd_connection_get_error_message(M_CONN) << endl;
        return false;
    }
    return true;
}

int MPDCli::getVolume()
{
    updStatus();
    return m_stat.volume == -1 ? 0: m_stat.volume;
}

bool MPDCli::updStatus()
{
    //cerr << "MPDCli::updStatus" << endl;
    if (!m_ok) {
        cerr << "MPDCli::updStatus: bad state" << endl;
        return false;
    }
    memset(&m_stat, 0, sizeof(m_stat));
    mpd_status *mpds = mpd_run_status(M_CONN);
    if (mpds == 0) {
        cerr << "MPDCli::updStatus: status failed. " <<
            "connec error: " <<  mpd_connection_get_error_message(M_CONN) <<
            "server error: " << mpd_connection_get_server_error(M_CONN) <<
             endl;
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
    m_stat.songms = mpd_status_get_elapsed_ms(mpds);
    m_stat.songlen = mpd_status_get_total_time(mpds);
    m_stat.kbrate = mpd_status_get_kbit_rate(mpds);
    const char *err = mpd_status_get_error(mpds);
    if (err != 0)
        m_stat.errormessage = err;

    mpd_status_free(mpds);
    return true;
}
