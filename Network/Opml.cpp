/*
 * $Id: Opml.cpp 74 2007-10-20 10:46:39Z soarchin $

 * Copyright 2007 surfchen

 * This file is part of eReader2.

 * eReader2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * eReader2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Opml.h"
opml::opml(string feed) {
    this->feed=feed;
    this->feed_position=0;
}
bool opml::next(FeedInfo* feed_info) {
    if (feeds.size()<=0) {
        TiXmlDocument doc(feed);
        doc.LoadFile();
        this->parse(&doc,"");
    }
    if (this->feed_position>=feeds.size()) return false;
    *feed_info=this->feeds.at(this->feed_position);
    this->feed_position++;
    return true;
}
bool opml::rewind() {
    this->feed_position=0;
}
bool opml::parse(TiXmlNode* pnode,string cat) {
    int t = pnode->Type();
    TiXmlNode* cnode;
    FeedInfo fi;
    if (t==TiXmlNode::ELEMENT && pnode->ValueStr()=="outline") {
        TiXmlElement* pelement;
        pelement=pnode->ToElement();
        if (!pelement->Attribute("xmlUrl")) {
            cat=pelement->Attribute("text");
        } else {
            fi.url=pelement->Attribute("xmlUrl");
            fi.title=pelement->Attribute("text");
            fi.category=cat;
            feeds.push_back(fi);
            cat="";
        }
    }
    cnode = 0;
    //for (cnode=pnode->FirstChild();cnode!=0;cnode->NextSibling()) {
    while( cnode = pnode->IterateChildren( cnode ) ) {
        this->parse(cnode,cat);
    }
}
