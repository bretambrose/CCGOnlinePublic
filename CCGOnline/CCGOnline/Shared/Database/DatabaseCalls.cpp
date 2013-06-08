/**********************************************************************************************************************

	DatabaseCalls.cpp
		A component defining ??

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

#include "stdafx.h"

#include "DatabaseCalls.h"

#include "Interfaces/CompoundDatabaseTaskInterface.h"


void CDatabaseTaskBase::Set_Parent( ICompoundDatabaseTask *parent )
{
	FATAL_ASSERT( parent != nullptr );

	Parent = parent;
	ID = parent->Get_ID();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCompoundDatabaseTaskBase::~CCompoundDatabaseTaskBase()
{
	Clear_Child_Tasks();

	std::for_each( Children.begin(), Children.end(), []( const DBTaskListTablePairType& pair ){ delete pair.second; } );
	Children.clear();
}

void CCompoundDatabaseTaskBase::Add_Child_Task( IDatabaseTask *task )
{
	FATAL_ASSERT( ID != DatabaseTaskIDType::INVALID );

	task->Set_ID( ID );

	Loki::TypeInfo task_type( typeid( *task ) );
	DBTaskListType *task_list = nullptr;
	auto iter = Children.find( task_type );
	if ( iter == Children.end() )
	{
		task_list = new DBTaskListType;
		Children[ task_type ] = task_list;
	}
	else
	{
		task_list = iter->second;
	}

	FATAL_ASSERT( task_list != nullptr );

	task_list->push_back( task );
}

void CCompoundDatabaseTaskBase::Clear_Child_Tasks( void )
{
	for( auto iter = Children.begin(); iter != Children.end(); ++iter)
	{
		DBTaskListType *task_list = iter->second;
		if ( task_list == nullptr )
		{
			continue;
		}

		std::for_each( task_list->begin(), task_list->end(), []( IDatabaseTask *task ){ delete task; } );
		task_list->clear();
	}
}

void CCompoundDatabaseTaskBase::Get_Child_Tasks_Of_Type( const Loki::TypeInfo &child_type, DBTaskListType &tasks ) const
{
	auto iter = Children.find( child_type );
	if ( iter != Children.end() )
	{
		DBTaskListType *task_list = iter->second;
		if ( task_list != nullptr )
		{
			std::copy( task_list->begin(), task_list->end(), std::back_inserter( tasks ) );
		}
	}
}

void CCompoundDatabaseTaskBase::On_Child_Task_Success( const Loki::TypeInfo &child_type )
{
	auto iter = SuccessCallbacks.find( child_type );
	if ( iter == SuccessCallbacks.end() )
	{
		return;
	}

	iter->second( child_type );
}

void CCompoundDatabaseTaskBase::Register_Child_Type_Success_Callback( const Loki::TypeInfo &child_type, const ChildSuccessHandlerFunctorType &success_callback )
{
	FATAL_ASSERT( SuccessCallbacks.find( child_type ) == SuccessCallbacks.end() );

	SuccessCallbacks[ child_type ] = success_callback;
}

void CCompoundDatabaseTaskBase::Get_All_Child_Tasks( DBTaskListType &tasks ) const
{
	for( auto iter = Children.begin(); iter != Children.end(); ++iter)
	{
		DBTaskListType *task_list = iter->second;
		if ( task_list == nullptr )
		{
			continue;
		}

		std::copy( task_list->begin(), task_list->end(), back_inserter( tasks ) );
	}
}