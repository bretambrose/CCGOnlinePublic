/**********************************************************************************************************************

	DatabaseCalls.h
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

#ifndef DATABASE_CALLS_H
#define DATABASE_CALLS_H

#include "Interfaces/CompoundDatabaseTaskInterface.h"
#include "Interfaces/DatabaseTaskInterface.h"
#include "DatabaseTypes.h"

class CEmptyVariableSet;

class CDatabaseTaskBase : public IDatabaseTask
{
	public:

		typedef IDatabaseTask BASECLASS;

		CDatabaseTaskBase( void ) :
			BASECLASS(),
			Parent( nullptr ),
			ID( DatabaseTaskIDType::INVALID )
		{}

		virtual ~CDatabaseTaskBase() {}

		virtual DatabaseTaskIDType::Enum Get_ID( void ) const { return ID; }
		virtual void Set_ID( DatabaseTaskIDType::Enum id ) { ID = id; }

	protected:

		virtual void Set_Parent( ICompoundDatabaseTask *parent );
		virtual ICompoundDatabaseTask *Get_Parent( void ) const { return Parent; }

	private:

		ICompoundDatabaseTask *Parent;

		DatabaseTaskIDType::Enum ID;
};

template < typename I, uint32_t ISIZE >
class TDatabaseFunctionCall : public CDatabaseTaskBase
{
	public:

		typedef CDatabaseTaskBase BASECLASS;

		TDatabaseFunctionCall( void ) :
			BASECLASS()
		{}

		virtual ~TDatabaseFunctionCall() {}

		virtual void Build_Column_Name_List( std::vector< const wchar_t * > & /*column_names*/ ) const {}
		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_FUNCTION_CALL; }

		typedef I InputParametersType;
		typedef CEmptyVariableSet ResultSetType;

		static const uint32_t InputParameterBatchSize = ISIZE;
		static const uint32_t ResultSetBatchSize = 1;

	protected:

		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64_t rows_fetched ) {
			ASSERT_TRUE( rows_fetched == 0 );
		}
};

template < typename I, uint32_t ISIZE, typename O, uint32_t OSIZE >
class TDatabaseProcedureCall : public CDatabaseTaskBase
{
	public:

		typedef CDatabaseTaskBase BASECLASS;

		TDatabaseProcedureCall( void ) :
			BASECLASS()
		{}

		virtual ~TDatabaseProcedureCall() {}

		virtual void Build_Column_Name_List( std::vector< const wchar_t * > & /*column_names*/ ) const {}
		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_PROCEDURE_CALL; }

		typedef I InputParametersType;
		typedef O ResultSetType;

		static const uint32_t InputParameterBatchSize = ISIZE;
		static const uint32_t ResultSetBatchSize = OSIZE;
};

template < typename O, uint32_t OSIZE >
class TDatabaseSelect : public CDatabaseTaskBase
{
	public:

		typedef CDatabaseTaskBase BASECLASS;

		TDatabaseSelect( void ) :
			BASECLASS()
		{}

		virtual ~TDatabaseSelect() {}

		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_SELECT; }

		typedef CEmptyVariableSet InputParametersType;
		typedef O ResultSetType;

		static const uint32_t InputParameterBatchSize = 1;
		static const uint32_t ResultSetBatchSize = OSIZE;
};

template < typename I, uint32_t ISIZE, typename O, uint32_t OSIZE >
class TDatabaseTableValuedFunctionCall : public CDatabaseTaskBase
{
	public:

		typedef CDatabaseTaskBase BASECLASS;

		TDatabaseTableValuedFunctionCall( void ) :
			BASECLASS()
		{}

		virtual ~TDatabaseTableValuedFunctionCall() {}

		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_TABLE_VALUED_FUNCTION_CALL; }

		typedef I InputParametersType;
		typedef O ResultSetType;

		static const uint32_t InputParameterBatchSize = ISIZE;
		static const uint32_t ResultSetBatchSize = OSIZE;
};

// Compound

class CCompoundDatabaseTaskBase : public ICompoundDatabaseTask
{
	public:

		typedef ICompoundDatabaseTask BASECLASS;

		CCompoundDatabaseTaskBase( void ) :
			BASECLASS(),
			ID( DatabaseTaskIDType::INVALID ),
			Children(),
			SuccessCallbacks()
		{}

		virtual ~CCompoundDatabaseTaskBase();

		// IDatabaseTask interface
		virtual DatabaseTaskIDType::Enum Get_ID( void ) const { return ID; }
		virtual void Set_ID( DatabaseTaskIDType::Enum id ) { ID = id; }

		// ICompoundDatabaseTask interface
		virtual void Add_Child_Task( IDatabaseTask *task );
		virtual void On_Child_Task_Success( const Loki::TypeInfo &child_type );

		virtual void Clear_Child_Tasks( void );
		virtual void Get_Child_Tasks_Of_Type( const Loki::TypeInfo &child_type, DBTaskListType &tasks ) const;
		virtual void Get_All_Child_Tasks( DBTaskListType &tasks ) const;

	protected:

		typedef FastDelegate0<> ChildSuccessHandlerFunctorType;

		// IDatabaseTask interface
		virtual void Set_Parent( ICompoundDatabaseTask * /*parent*/ ) { FATAL_ASSERT( false ); }
		virtual ICompoundDatabaseTask *Get_Parent( void ) const { return nullptr; }

		void Register_Child_Type_Success_Callback( const Loki::TypeInfo &child_type, const ChildSuccessHandlerFunctorType &success_callback );

	private:

		typedef std::unordered_map< Loki::TypeInfo, ChildSuccessHandlerFunctorType, STypeInfoContainerHelper > ChildTypeCallbackTableType;

		DatabaseTaskIDType::Enum ID;

		DBTaskListTableType Children;

		ChildTypeCallbackTableType SuccessCallbacks;
};

template < uint32_t BATCH_SIZE >
class TCompoundDatabaseTask : public CCompoundDatabaseTaskBase {
	public:

		typedef CCompoundDatabaseTaskBase BASECLASS;

		TCompoundDatabaseTask( void ) :
			BASECLASS()
		{}

		virtual ~TCompoundDatabaseTask() {}

		static const uint32_t CompoundTaskBatchSize = BATCH_SIZE;

	private:
};

#endif // DATABASE_CALLS_H