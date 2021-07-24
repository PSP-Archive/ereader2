/*
 * $Id: Feed.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#include "Feed.h"
bool feedparser::process(string feed) 
{
    TiXmlDocument doc(feed);
    doc.LoadFile();
    TiXmlNode* cnode=0;
    string node_value;
    while (cnode=doc.IterateChildren(cnode)) {
        if (cnode->Type()==TiXmlNode::ELEMENT) {
            node_value=cnode->ValueStr();
            if (node_value=="feed") {//ATOM
                this->feed_struct.item_name="entry";
                this->feed_struct.title="title";
                this->feed_struct.content="description";
            } else if (node_value=="rss") {//rss2.0 && 1.0
                this->feed_struct.parents.push_back("channel");
                this->feed_struct.item_name="item";
                this->feed_struct.title="title";
                this->feed_struct.content="description";
            } else {
                continue;
            }
            break;
        }
    }
    this->findItemParentNode(cnode,-1);
}
bool feedparser::findItemParentNode(TiXmlNode *node,int struct_step)
{
    if (struct_step<=(this->feed_struct.parents.size()-1)) {
        return this->findItemNode(node);
    }
    struct_step++;
    TiXmlNode *cnode;
    cnode=0;
    while (cnode=node->IterateChildren(cnode)) {
        if (cnode->Type()==TiXmlNode::ELEMENT && cnode->ValueStr()==this->feed_struct.parents.at(struct_step)) {
            return this->findItemParentNode(cnode,struct_step);
        }
    }
    return false;
}
bool feedparser::findItemNode(TiXmlNode *node) {
    TiXmlNode *cnode;
    cnode=0;
    while (cnode=node->IterateChildren(cnode)) {
        if (cnode->Type()==TiXmlNode::ELEMENT && cnode->ValueStr()==this->feed_struct.item_name) {
            this->processItemNode(cnode);
        }
    }
    return true;
}
bool feedparser::processItemNode(TiXmlNode* pnode)
{
    TiXmlNode* cnode;
    FeedItemInfo item;
    cnode=0;
    int get=0;
    int t = 0;
    while( cnode = pnode->IterateChildren( cnode ) ) {
        t=cnode->Type();
        if (t==TiXmlNode::ELEMENT && cnode->ValueStr()==this->feed_struct.title) {
            item.title = this->getNodeText(cnode);
            get++;
        }
        if (t==TiXmlNode::ELEMENT && cnode->ValueStr()==this->feed_struct.content) {
            item.content = this->getNodeText(cnode);
            get++;
        }
        if (get>=2) break;
    }
    return this->handle(&item);
}
string feedparser::getNodeText(TiXmlNode* node)
{
    string node_str="";
    int type;
    TiXmlNode *cnode = node->FirstChild();
    for(; cnode; cnode = node->IterateChildren(cnode)) {
        type=cnode->Type();
        if (type==TiXmlNode::TEXT) node_str+=cnode->Value();
        this->getNodeText(cnode);
    }
    return node_str;
}
