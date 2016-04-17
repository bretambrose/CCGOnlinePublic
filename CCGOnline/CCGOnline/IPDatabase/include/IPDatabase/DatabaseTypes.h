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

#include <IPCore/Memory/Stl/List.h>
#include <IPCore/Memory/Stl/UnorderedMap.h>
#include <IPShared/TypeInfoUtils.h>

//:EnumBegin()
enum DBErrorStateType
{
	DBEST_SUCCESS,					//:EnumEntry( "Success" )

	DBEST_FATAL_ERROR,			//:EnumEntry( "Fatal Error" )
	DBEST_RECOVERABLE_ERROR,	//:EnumEntry( "Recoverable Error" )
	DBEST_WARNING					//:EnumEntry( "Warning" )
};
//:EnumEnd

namespace IP
{
namespace Db
{

inline bool Is_Fatal_DB_Error( DBErrorStateType error ) { return error == DBEST_FATAL_ERROR; }
inline bool Was_Database_Operation_Successful( DBErrorStateType error ) { return error == DBEST_SUCCESS || error == DBEST_WARNING; }

} // namespace Db
} // namespace IP

enum DBConnectionIDType
{
	DBCIDT_INVALID
};

enum DBStatementIDType
{
	DBSIDT_INVALID
};

//:EnumBegin()
enum EDatabaseVariableType
{
	DVT_INPUT,					//:EnumEntry( "In" )
	DVT_INPUT_OUTPUT,			//:EnumEntry( "InOut" )
	DVT_OUTPUT					//:EnumEntry( "Out" )
};
//:EnumEnd

//:EnumBegin()
enum EDatabaseVariableValueType
{
	DVVT_INVALID,

	DVVT_INT32,					//:EnumEntry( "Int32" )
	DVVT_UINT32,				//:EnumEntry( "UInt32" )
	DVVT_INT64,					//:EnumEntry( "Int64" )
	DVVT_UINT64,				//:EnumEntry( "UInt64" )
	DVVT_STRING,				//:EnumEntry( "String" )
	DVVT_WSTRING,				//:EnumEntry( "WString" )
	DVVT_FLOAT,					//:EnumEntry( "Float" )
	DVVT_DOUBLE,				//:EnumEntry( "Double" )
	DVVT_BOOLEAN				//:EnumEntry( "Boolean" )
};
//:EnumEnd

enum EFetchResultsStatusType
{
	FRST_ONGOING,
	FRST_FINISHED_SET,
	FRST_FINISHED_ALL,

	FRST_ERROR
};

enum EDatabaseTaskType
{
	DTT_PROCEDURE_CALL,
	DTT_FUNCTION_CALL,
	DTT_SELECT,
	DTT_TABLE_VALUED_FUNCTION_CALL
};

namespace IP
{
namespace Db
{

class IDatabaseTaskBase;
class IDatabaseTask;
class ICompoundDatabaseTask;

} // namespace Db
} // namespace IP

using DBTaskBaseListType = IP::List< IP::Db::IDatabaseTaskBase * >;
using DBTaskListType = IP::List< IP::Db::IDatabaseTask * >;
using DBCompoundTaskListType = IP::List< IP::Db::ICompoundDatabaseTask * >;

using DBTaskListTableType = IP::UnorderedMap< Loki::TypeInfo, DBTaskListType *, IP::STypeInfoContainerHelper >;
using DBTaskListTablePairType = std::pair< Loki::TypeInfo, DBTaskListType * >;

