/*
* foo_alsong_lyric														
* Copyright (C) 2007-2010 Inseok Lee <dlunch@gmail.com>
*
* This library is free software; you can redistribute it and/or modify it 
* under the terms of the GNU Lesser General Public License as published 
* by the Free Software Foundation; version 2.1 of the License.
*
* This library is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
* See the GNU Lesser General Public License for more details.
*
* You can receive a copy of the GNU Lesser General Public License from 
* http://www.gnu.org/
*/

#include "stdafx.h"

#include "Socket.h"

Socket::Socket(const char *Address, int port)
{
	hostent *host = gethostbyname(Address);
	if(!host)
		return;
	in_addr *naddr;
	naddr = (in_addr *)host->h_addr;

	m_Socket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in client_addr;
	ZeroMemory(&client_addr, sizeof(client_addr));
	client_addr.sin_addr = *naddr;
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(port);

	connect(m_Socket, (sockaddr *)(&client_addr), sizeof(sockaddr_in));
}

Socket::~Socket()
{
	if(m_Socket)
		closesocket(m_Socket);
}

int Socket::Send(const char *buf, int len)
{
	return send(m_Socket, buf, len, 0);
}

std::vector<char> Socket::ReceiveUntilEOF()
{
	std::vector<char> data;
	int nRecv;
	char buf[256];

	while(nRecv = recv(m_Socket, buf, 255, 0))
	{
		if(SOCKET_ERROR == nRecv)
		{
			closesocket(m_Socket);
			throw SocketTransferException();
		}
		buf[nRecv] = 0;
		data.insert(data.end(), buf, buf + nRecv);
		if(boost::this_thread::interruption_requested())
			return std::vector<char>();
	}
	data.emplace_back('\0');

	return data;
}
