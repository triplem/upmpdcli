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
#ifndef _MPDCLI_H_X_INCLUDED_
#define _MPDCLI_H_X_INCLUDED_

#include <unordered_map>
#include <string>
#include <map>

struct MpdStatus {
    enum State {MPDS_UNK, MPDS_STOP, MPDS_PLAY, MPDS_PAUSE};
    int volume;
    bool rept;
    bool random;
    bool single;
    bool consume;
    int qlen;
    int qvers;
    State state;
    unsigned int crossfade;
    float mixrampdb;
    float mixrampdelay;
    int songpos;
    int songid;
    unsigned int songelapsedms; //current ms
    unsigned int songlenms; // song millis
    unsigned int kbrate;
    std::string errormessage;
    // Current song info. The keys are didl-lite names (which can be
    // attribute or element names
    std::map<std::string, std::string> currentsong;
    std::map<std::string, std::string> nextsong;
};

class MPDCli {
public:
    MPDCli(const std::string& host, int port = 6600, 
           const std::string& pass="");
    ~MPDCli();
    bool ok() {return m_ok && m_conn;}
    bool setVolume(int ivol, bool relative = false);
    int  getVolume();
    bool togglePause();
    bool play();
    bool stop();
    bool next();
    bool previous();
    bool repeat(bool on);
    bool random(bool on);
    bool single(bool on);
    int insert(const std::string& uri, int pos);
    int curpos();
    const struct MpdStatus& getStatus()
    {
        updStatus();
        return m_stat;
    }

private:
    void *m_conn;
    bool m_ok;
    MpdStatus m_stat;
    int m_premutevolume;
    std::string m_host;
    int m_port;
    std::string m_password;

    bool openconn();
    bool updStatus();
    bool updSong(std::map<std::string, std::string>& status, int pos = -1);
    bool showError(const std::string& who);
};


#endif /* _MPDCLI_H_X_INCLUDED_ */
