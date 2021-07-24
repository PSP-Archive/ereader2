/*
 * $Id: Feed.h 74 2007-10-20 10:46:39Z soarchin $

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

#include <string>
#include <vector>
#include "lib/tinyxml/tinyxml.h"
using namespace std; 
struct FeedItemInfo 
{
    string title;
    string content;
};
struct FeedStructInfo
{
    vector<string> parents;
    string item_name;
    string title;
    string content;
};
class feedparser
{
public:
    bool process(string feed);
private:
    FeedStructInfo feed_struct;
    bool findItemParentNode(TiXmlNode *node,int struct_step);
    bool findItemNode(TiXmlNode *node);
    bool processItemNode(TiXmlNode* pnode);
    bool handle(FeedItemInfo *item);
    string getNodeText(TiXmlNode *node);
};
