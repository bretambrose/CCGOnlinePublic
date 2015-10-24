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

#include "DatabaseTaskBaseInterface.h"

#include "IPDatabase/DatabaseTypes.h"

namespace IP
{
namespace Db
{

template< typename T > class TCompoundDatabaseTaskBatch;

class ICompoundDatabaseTask : public IDatabaseTaskBase
{
	public:
		
		using BASECLASS = IDatabaseTaskBase;

		ICompoundDatabaseTask( void ) {}
		virtual ~ICompoundDatabaseTask() {}

		virtual void Add_Child_Task( IDatabaseTask *task ) = 0;
		virtual void On_Child_Task_Success( const Loki::TypeInfo &child_type ) = 0;

		virtual void Seed_Child_Tasks( void ) = 0;
		virtual void Clear_Child_Tasks( void ) = 0;
		virtual void Get_Child_Tasks_Of_Type( const Loki::TypeInfo &child_type, DBTaskListType &tasks ) const = 0;
		virtual void Get_All_Child_Tasks( DBTaskListType &tasks ) const = 0;

};

} // namespace Db
} // namespace IP