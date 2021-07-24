/*
 * $Id: Network.h 74 2007-10-20 10:46:39Z soarchin $

 * Copyright 2007 aeolusc

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

#pragma once

#include "Thread.h"
#include "Datatype.h"

class NetworkConnState
{
public:
	virtual VOID Timed(INT msec) {}
	virtual VOID State(INT state) {}
	virtual ~NetworkConnState() {}
};

class Network
{
private:
	Thread * m_thread;
	NetworkConnState * m_state;
public:
	Network();
	~Network();
	VOID Init();
//	BOOL ConnectTo(INT index);
	BOOL ConnectToAP(INT index);
	VOID Disconnect();
	VOID SetConnStateClass(NetworkConnState * state);
	DWORD ResolveName(CONST CHAR * name);
};

extern Network g_Network;

class Socket
{
private:
	INT m_sock;
public:
	Socket();
	~Socket();
	BOOL Bind(WORD port);
	BOOL Listen(DWORD backlog);
	BOOL Connect(CONST CHAR * name, WORD port);
	BOOL Connect(DWORD ip, WORD port);
	INT Send(CONST CHAR * buf, INT len);
	INT Send(DWORD data);
	INT Send(WORD data);
	INT Send(BYTE data);
	INT Send(CONST CHAR * str);
	INT Recv(CHAR * buf, INT len);
	VOID Close();
};

class HttpGet
{
private:
	typedef struct {
		HttpGet * getclass;
		VOID * stream;
	} HttpGetParam;
	HttpGetParam m_param;
	STATIC UINT m_WriteData(VOID *ptr, UINT size, UINT nmemb, VOID *stream);
protected:
	virtual BOOL OnStart(CONST CHAR * url) = 0;
	virtual BOOL OnEnd(CONST CHAR * url) = 0;
	virtual UINT WriteData(VOID *ptr, UINT size, UINT nmemb, VOID *stream) = 0;
	VOID SetWriteStream(VOID * stream);
	VOID * GetWriteStream();
public:
	HttpGet();
	virtual ~HttpGet();
	BOOL Get(CONST CHAR * url);
};
