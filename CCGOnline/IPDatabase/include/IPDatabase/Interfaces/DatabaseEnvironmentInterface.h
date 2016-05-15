/**********************************************************************************************************************

	(c) Copyright 2012, Bret Ambrose (mailto:bretambrose@gmail.com).

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

#pragma once

#include <IPDatabase/IPDatabase.h>

#include <IPCore/Memory/Stl/String.h>

enum DBConnectionIDType;
enum DBErrorStateType;

namespace IP
{
namespace Db
{

class IDatabaseConnection;

IPDATABASE_API class IDatabaseEnvironment
{
	public:

		virtual ~IDatabaseEnvironment() {}

		virtual void Initialize( void ) = 0;
		virtual void Shutdown( void ) = 0;
		virtual void Shutdown_Connection( DBConnectionIDType connection_id ) = 0;
		virtual void Shutdown_Connection( IDatabaseConnection *connection ) = 0;

		virtual IDatabaseConnection *Add_Connection( const IP::String &connection_string, bool cache_statements ) = 0;

		virtual DBErrorStateType Get_Error_State( void ) const = 0;
};

} // namespace Db
} // namespace IP