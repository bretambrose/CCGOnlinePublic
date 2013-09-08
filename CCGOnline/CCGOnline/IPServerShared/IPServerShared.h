/**********************************************************************************************************************

	IP_ServerShared.h
		Component containing initialize/shutdown for the IP_ServerShared library; calls Server library initalize/shutdown

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

#ifndef IP_SERVER_SHARED_H
#define IP_SERVER_SHARED_H

namespace NIPServerShared
{
	void Initialize( void );
	void Shutdown( void );
} 

#endif // SERVER_SHARED_H