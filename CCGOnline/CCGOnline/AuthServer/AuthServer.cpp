/**********************************************************************************************************************

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

#include "IPShared/IPShared.h"
#include "GeneratedCode/RegisterAuthServerEnums.h"

namespace IP
{
namespace Global
{

	void Initialize_AuthServer( void )
	{
		Initialize_IPShared();
		Register_AuthServer_Enums();
	}

	void Shutdown_AuthServer( void )
	{
		Shutdown_IPShared();
	}

} // namespace Global
} // namespace IP

using namespace IP::Global;

int main( int /*argc*/, wchar_t* /*argv*/[] )
{
	Initialize_AuthServer();
	Shutdown_AuthServer();

	return 0;
}

