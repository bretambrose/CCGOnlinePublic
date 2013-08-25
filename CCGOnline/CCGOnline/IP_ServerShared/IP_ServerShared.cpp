/**********************************************************************************************************************

	IP_ServerShared.cpp
		Component containing  initialize/shutdown for the IP_ServerShared library

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

#include "IP_ServerShared.h"

#include "IP_Shared.h"
#include "GeneratedCode/RegisterIP_ServerSharedEnums.h"

/**********************************************************************************************************************
	NIPServerShared::Initialize -- Initializes the IP_ServerShared library and all its ancestors

**********************************************************************************************************************/
void NIPServerShared::Initialize( void )
{
	NIPShared::Initialize();
	Register_IP_ServerShared_Enums();
}

/**********************************************************************************************************************
	NIPServerShared::Shutdown -- Shuts down the IP_ServerShared library and all its ancestors

**********************************************************************************************************************/
void  NIPServerShared::Shutdown( void )
{
	NIPShared::Shutdown();
}