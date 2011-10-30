/**********************************************************************************************************************

	ServerShared.cpp
		Component containing  initialize/shutdown for ServerShared library

	(c) Copyright 2011, Bret Ambrose (mailto:bretambrose@gmail.com).

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************************************************************/

#include "stdafx.h"

#include "ServerShared.h"

#include "Shared.h"

/**********************************************************************************************************************
	NServerShared::Initialize_Server_Shared -- Initializes the ServerShared library and all its ancestors

**********************************************************************************************************************/
void NServerShared::Initialize( void )
{
	NShared::Initialize();
}

/**********************************************************************************************************************
	NServerShared::Shutdown_Server_Shared -- Shuts down the ServerShared library and all its ancestors

**********************************************************************************************************************/
void  NServerShared::Shutdown( void )
{
	NShared::Shutdown();
}