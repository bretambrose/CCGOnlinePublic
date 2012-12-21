/**********************************************************************************************************************

	ODBCDatabaseTaskTests.cpp
		defines unit tests for ODBC database task functionality

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

#include "Database/ODBCImplementation/ODBCFactory.h"
#include "Database/Interfaces/DatabaseConnectionInterface.h"
#include "Database/Interfaces/DatabaseEnvironmentInterface.h"
#include "Database/Interfaces/DatabaseStatementInterface.h"
#include "Database/Interfaces/DatabaseVariableSetInterface.h"
#include "Database/ODBCImplementation/ODBCParameters.h"
#include "Database/DatabaseVariableSet.h"
#include "Database/DatabaseCalls.h"
#include "Database/DatabaseTaskBatch.h"

class ODBCDatabaseTaskTests : public testing::Test 
{
	public:
	
	static void SetUpTestCase() 
	{
		system( "rebuild_test_db.bat 1> nul" );

		CODBCFactory::Create_Environment();
	}

	static void TearDownTestCase() 
	{
		CODBCFactory::Destroy_Environment();
	}	  

};

class CGetAccountProcedureResultSet : public IDatabaseVariableSet
{
	public:

		CGetAccountProcedureResultSet( void ) :
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		CGetAccountProcedureResultSet( const CGetAccountProcedureResultSet &rhs ) :
			AccountID( rhs.AccountID ),
			AccountEmail( rhs.AccountEmail ),
			Nickname( rhs.Nickname ),
			NicknameSequenceID( rhs.NicknameSequenceID )
		{}

		virtual ~CGetAccountProcedureResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountID );
			variables.push_back( &AccountEmail );
			variables.push_back( &Nickname );
			variables.push_back( &NicknameSequenceID );
		}

		DBUInt64In AccountID;
		DBString< 255 > AccountEmail;
		DBString< 32 > Nickname;
		DBUInt32In NicknameSequenceID;
};

template< uint32 OSIZE >
class CGetAllAccountsProcedureCall : public TDatabaseProcedureCall< CEmptyVariableSet, 1, CGetAccountProcedureResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CEmptyVariableSet, 1, CGetAccountProcedureResultSet, OSIZE > BASECLASS;

		CGetAllAccountsProcedureCall( void ) : 
			BASECLASS(),
			Accounts(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CGetAllAccountsProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.get_all_accounts"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );
			ASSERT_TRUE( Accounts.size() == 3 );

			for ( uint32 i = 0; i < Accounts.size(); ++i )
			{
				uint64 current_account_id = Accounts[ i ].AccountID.Get_Value();
				ASSERT_TRUE( current_account_id == i + 1 );

				switch ( current_account_id )
				{
					case 1:
						ASSERT_TRUE( _stricmp( Accounts[ i ].AccountEmail.Get_Buffer(), "bretambrose@gmail.com" ) == 0 );
						ASSERT_TRUE( _stricmp( Accounts[ i ].Nickname.Get_Buffer(), "Bret" ) == 0 );
						ASSERT_TRUE( Accounts[ i ].NicknameSequenceID.Get_Value() == 1 );
						break;

					case 2:
						ASSERT_TRUE( _stricmp( Accounts[ i ].AccountEmail.Get_Buffer(), "petra222@yahoo.com" ) == 0 );
						ASSERT_TRUE( _stricmp( Accounts[ i ].Nickname.Get_Buffer(), "Peti" ) == 0 );
						ASSERT_TRUE( Accounts[ i ].NicknameSequenceID.Get_Value() == 1 );
						break;

					case 3:
						ASSERT_TRUE( _stricmp( Accounts[ i ].AccountEmail.Get_Buffer(), "will@mailinator.com" ) == 0 );
						ASSERT_TRUE( _stricmp( Accounts[ i ].Nickname.Get_Buffer(), "Will" ) == 0 );
						ASSERT_TRUE( Accounts[ i ].NicknameSequenceID.Get_Value() == 1 );
						break;

					default:
						ASSERT_TRUE( false );
						break;
				}
			}
		}

	protected:

		// WIP: signatures in flux
		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) { InitializeCalls++; }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) {
			CGetAccountProcedureResultSet *results = static_cast< CGetAccountProcedureResultSet * >( result_set );
			for ( uint32 i = 0; i < rows_fetched; ++i )
			{
				Accounts.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) { FinishedCalls++;}	

		virtual void On_Rollback( void ) { ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		std::vector< CGetAccountProcedureResultSet > Accounts;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};


TEST_F( ODBCDatabaseTaskTests, ReadSeededData_GetAllAccounts1_OK )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	CGetAllAccountsProcedureCall< 1 > *db_task = new CGetAllAccountsProcedureCall< 1 >;

	TDatabaseTaskBatch< CGetAllAccountsProcedureCall< 1 > > db_task_batch;
	db_task_batch.Add_Task( db_task );

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == 1 );
	
	db_task->Verify_Results();

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCDatabaseTaskTests, ReadSeededData_GetAllAccounts2_OK )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	CGetAllAccountsProcedureCall< 2 > *db_task = new CGetAllAccountsProcedureCall< 2 >;

	TDatabaseTaskBatch< CGetAllAccountsProcedureCall< 2 > > db_task_batch;
	db_task_batch.Add_Task( db_task );

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == 1 );
	
	db_task->Verify_Results();

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCDatabaseTaskTests, ReadSeededData_GetAllAccounts3_OK )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	CGetAllAccountsProcedureCall< 3 > *db_task = new CGetAllAccountsProcedureCall< 3 >;

	TDatabaseTaskBatch< CGetAllAccountsProcedureCall< 3 > > db_task_batch;
	db_task_batch.Add_Task( db_task );

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == 1 );
	
	db_task->Verify_Results();

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCDatabaseTaskTests, ReadSeededData_GetAllAccounts4_OK )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	CGetAllAccountsProcedureCall< 4 > *db_task = new CGetAllAccountsProcedureCall< 4 >;

	TDatabaseTaskBatch< CGetAllAccountsProcedureCall< 4 > > db_task_batch;
	db_task_batch.Add_Task( db_task );

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == 1 );
	
	db_task->Verify_Results();

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}