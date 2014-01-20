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

#include <iostream>
using namespace std;

#include "soaphelp.hxx"


/* Example Soap XML doc passed by libupnp is like: 
   <ns0:SetMute>
     <InstanceID>0</InstanceID>
     <Channel>Master</Channel>
     <DesiredMute>False</DesiredMute>
   </ns0:SetMute>
*/
bool decodeSoap(const char *callnm, IXML_Document *actReq, SoapCall *res)
{
    bool ret = false;
    IXML_NodeList* nl = 0;
    IXML_Node* topNode = 
        ixmlNode_getFirstChild((IXML_Node *)actReq);
    if (topNode == 0) {
        cerr << "decodeSoap: Empty Action request (no topNode) ??" << endl;
        return false;
    }
        
    nl = ixmlNode_getChildNodes(topNode);
    if (nl == 0) {
        cerr << "decodeSoap: empty Action request (no childs) ??" << endl;
        return false;
    }
    for (unsigned long i = 0; i <  ixmlNodeList_length(nl); i++) {
        IXML_Node *cld = ixmlNodeList_item(nl, i);
        if (cld == 0) {
            cerr << "decodeSoap: got null node  from nodelist??" << endl;
            goto out;
        }
        const char *name = ixmlNode_getNodeName(cld);
        if (cld == 0) {
            cerr << "decodeSoap: got null name ??" << endl;
            goto out;
        }
        IXML_Node *txtnode = ixmlNode_getFirstChild(cld);
        if (txtnode == 0) {
            cerr << "decodeSoap: got null name ??" << endl;
            goto out;
        }
        const char *value = ixmlNode_getNodeValue(txtnode);
        // Can we get an empty value here ?
        if (value == 0)
            value = "";
        res->args[name] = value;
    }
    res->name = callnm;
    ret = true;
out:
    if (nl)
        ixmlNodeList_free(nl);
    return ret;
}
