/*
 * $Id: Network.cpp 74 2007-10-20 10:46:39Z soarchin $

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

#include "Network.h"
#include <string.h>
#include <unistd.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psputility_netmodules.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "curl.h"

Network g_Network;

Network::Network()
{
	m_thread = NULL;
	m_state = NULL;
}

Network::~Network()
{
	Disconnect();
}

VOID Network::Init()
{
#if _PSP_FW_VERSION >= 200
	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
#else
	pspSdkLoadInetModules();
#endif
}

BOOL Network::ConnectToAP(INT index)
{
	INT stateLast = -1;
	INT timed = 0;

	if (pspSdkInetInit() != 0 || sceNetApctlConnect(index) != 0)
		return FALSE;

	while (TRUE)
	{
		INT state;
		if(sceNetApctlGetState(&state) != 0)
			return FALSE;
		if(state > stateLast)
		{
			if(m_state) m_state->State(state);
			stateLast = state;
		}
		if (state == 4)
			break;
		sceKernelDelayThread(50000);
		timed += 50;
		if(m_state) m_state->Timed(timed);
	}

	return TRUE;
}

VOID Network::Disconnect()
{
	sceNetApctlDisconnect();
	pspSdkInetTerm();
}

VOID Network::SetConnStateClass(NetworkConnState * state)
{
	m_state = state;
}

DWORD Network::ResolveName(CONST CHAR * name)
{
	CHAR buf[1024];
	int rid = -1;
	struct in_addr addr;

	if(sceNetResolverCreate(&rid, buf, sizeof(buf)) < 0)
		return inet_addr(name);

	if(sceNetResolverStartNtoA(rid, name, &addr, 5, 3) < 0)
	{
		sceNetResolverDelete(rid);
		return inet_addr(name);
	}

	if(rid >= 0)
		sceNetResolverDelete(rid);
	return htonl(addr.s_addr);
}

Socket::Socket()
{
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
}

Socket::~Socket()
{
	Close();
}

BOOL Socket::Bind(WORD port)
{
	if(m_sock < 0)
		return FALSE;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(m_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return FALSE;

	return TRUE;
}

BOOL Socket::Listen(DWORD backlog)
{
	if(m_sock < 0 || listen(m_sock, backlog) < 0)
		return FALSE;
	return TRUE;
}

BOOL Socket::Connect(CONST CHAR * name, WORD port)
{
	if(m_sock < 0)
		return FALSE;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = g_Network.ResolveName(name);
	if(connect(m_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return FALSE;

	return TRUE;
}

BOOL Socket::Connect(DWORD ip, WORD port)
{
	if(m_sock < 0)
		return FALSE;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ip);
	if(connect(m_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return FALSE;

	return TRUE;
}

INT Socket::Send(CONST CHAR * buf, INT len)
{
	if(m_sock < 0)
		return -1;

	return send(m_sock, buf, len, 0);
}

INT Socket::Send(DWORD data)
{
	return Send((CONST CHAR *)&data, sizeof(DWORD));
}

INT Socket::Send(WORD data)
{
	return Send((CONST CHAR *)&data, sizeof(WORD));
}

INT Socket::Send(BYTE data)
{
	return Send((CONST CHAR *)&data, sizeof(BYTE));
}

INT Socket::Send(CONST CHAR * str)
{
	return Send(str, strlen(str));
}

INT Socket::Recv(CHAR * buf, INT len)
{
	if(m_sock < 0)
		return -1;

	return recv(m_sock, buf, len, 0);
}

VOID Socket::Close()
{
	close(m_sock);
}

UINT HttpGet::m_WriteData(VOID *ptr, UINT size, UINT nmemb, VOID *stream)
{
	HttpGetParam * param = (HttpGetParam *)stream;
	return param->getclass->WriteData(ptr, size, nmemb, param->stream);
}

VOID HttpGet::SetWriteStream(VOID * stream)
{
	m_param.stream = stream;
}

VOID * HttpGet::GetWriteStream()
{
	return m_param.stream;
}

HttpGet::HttpGet()
{
	m_param.getclass = this;
}

HttpGet::~HttpGet()
{
}

BOOL HttpGet::Get(CONST CHAR * url)
{
	CURL * curl = curl_easy_init();
	if(curl)
	{
		if(OnStart(url))
		{
			curl_easy_setopt(curl, CURLOPT_FILE, &m_param);
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, m_WriteData);
			CURLcode res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			return (OnEnd(url) && (res == CURLE_OK));
		}
		curl_easy_cleanup(curl);
	}
	return FALSE;
}
