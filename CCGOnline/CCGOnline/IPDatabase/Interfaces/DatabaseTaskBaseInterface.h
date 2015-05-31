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

#ifndef DATABASE_TASK_BASE_INTERFACE_H
#define DATABASE_TASK_BASE_INTERFACE_H

class ICompoundDatabaseTask;

namespace DatabaseTaskIDType
{
	enum Enum
	{
		INVALID
	};
}

class IDatabaseTaskBase
{
	public:
		
		IDatabaseTaskBase( void ) {}
		virtual ~IDatabaseTaskBase() {}

		virtual DatabaseTaskIDType::Enum Get_ID( void ) const = 0;
		virtual void Set_ID( DatabaseTaskIDType::Enum id ) = 0;	

		virtual void On_Task_Success( void ) = 0;					
		virtual void On_Task_Failure( void ) = 0;	

	protected:	

		virtual void Set_Parent( ICompoundDatabaseTask *parent ) = 0;
		virtual ICompoundDatabaseTask *Get_Parent( void ) const = 0;

};

#endif // DATABASE_TASK_BASE_INTERFACE_H