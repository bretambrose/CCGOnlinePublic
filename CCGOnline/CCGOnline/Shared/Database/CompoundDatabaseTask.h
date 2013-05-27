/**********************************************************************************************************************

	CompoundDatabaseTask.h
		A component defining 

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

#ifndef COMPOUND_DATABASE_TASK_H
#define COMPOUND_DATABASE_TASK_H

#include "Interfaces/CompoundDatabaseTaskInterface.h"

template< uint32 SIZE >
class TCompoundDatabaseTask : public ICompoundDatabaseTask
{
	public:

		typedef ICompoundDatabaseTask BASECLASS;

		TCompoundDatabaseTask( void ) :
			BASECLASS(),
			ID( DatabaseTaskIDType::INVALID )
		{}

		virtual ~TCompoundDatabaseTask() 
		{ 
			Clear_Child_Tasks(); 
		}

		// IDatabaseTask
		virtual DatabaseTaskIDType::Enum Get_ID( void ) const { return ID; }
		virtual void Set_ID( DatabaseTaskIDType::Enum id ) { ID = id; }

		// ICompoundDatabaseTask
		virtual void Add_Child_Task( IDatabaseTask *task )
		{
			task->Set_ID( ID );

			Loki::TypeInfo type_info( typeid( *task ) );

			auto iter = TasksByType.find( type_info );
			if ( iter == TasksByType.end() )
			{
				TasksByType.insert( TasksByTypeTableType::value_type( type_info, DBTaskListType() ) );
				iter = TasksByType.find( type_info );
			}

			iter->second.push_back( task );
		}

		static const uint32 TaskBatchSize = SIZE;

	protected:

		// IDatabaseTask
		virtual void Set_Parent( ICompoundDatabaseTask * /*parent*/ ) { FATAL_ASSERT( false ); }
		virtual ICompoundDatabaseTask *Get_Parent( void ) const { return nullptr; }

		// ICompoundDatabaseTask
		virtual void Clear_Child_Tasks( void )
		{
			for ( auto table_iter = TasksByType.begin(); table_iter != TasksByType.end(); ++table_iter )
			{
				for ( auto list_iter = table_iter->second.begin(); list_iter != table_iter->second.end(); ++list_iter )
				{
					delete *list_iter;
				}
			}

			TasksByType.clear();
		}

		virtual void On_Child_Task_Type_Success( const Loki::TypeInfo & /*child_type*/ ) {}

		virtual void Get_Child_Tasks_Of_Type( const Loki::TypeInfo &child_type, DBTaskListType &tasks ) const
		{
			auto iter = TasksByType.find( child_type );
			if ( iter == TasksByType.end() )
			{
				return;
			}

			const DBTaskListType &children = iter->second;
			for ( auto iter = children.cbegin(); iter != children.cend(); ++iter )
			{
				tasks.push_back( *iter );
			}
		}

	private:

		DatabaseTaskIDType::Enum ID;

		typedef stdext::hash_map< Loki::TypeInfo, DBTaskListType, STypeInfoContainerHelper > TasksByTypeTableType;

		TasksByTypeTableType TasksByType;
};

#endif // COMPOUND_DATABASE_TASK_H