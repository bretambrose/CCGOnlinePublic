/**********************************************************************************************************************

	ODBCSuccessTests.cpp
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
#include "StringUtils.h"

class ODBCSuccessTests : public testing::Test 
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

template< uint32 ISIZE, uint32 OSIZE >
class CGetAllAccountsProcedureCall : public TDatabaseProcedureCall< CEmptyVariableSet, ISIZE, CGetAccountProcedureResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CEmptyVariableSet, ISIZE, CGetAccountProcedureResultSet, OSIZE > BASECLASS;

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

template< uint32 ISIZE, uint32 OSIZE >
void Run_ReadSeedData_GetAllAccounts_OK_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CGetAllAccountsProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CGetAllAccountsProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CGetAllAccountsProcedureCall< ISIZE, OSIZE > *db_task = new CGetAllAccountsProcedureCall< ISIZE, OSIZE >;
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_1_1_1_OK )
{
	Run_ReadSeedData_GetAllAccounts_OK_Test< 1, 1 >( 1 );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_1_2_1_OK )
{
	Run_ReadSeedData_GetAllAccounts_OK_Test< 1, 2 >( 1 );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_1_3_1_OK )
{
	Run_ReadSeedData_GetAllAccounts_OK_Test< 1, 3 >( 1 );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_1_4_1_OK )
{
	Run_ReadSeedData_GetAllAccounts_OK_Test< 1, 4 >( 1 );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_1_1_2_OK )
{
	Run_ReadSeedData_GetAllAccounts_OK_Test< 1, 1 >( 2 );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_1_2_2_OK )
{
	Run_ReadSeedData_GetAllAccounts_OK_Test< 1, 2 >( 2 );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_2_2_5_OK )
{
	Run_ReadSeedData_GetAllAccounts_OK_Test< 2, 2 >( 5 );
}

template< uint32 ISIZE, uint32 OSIZE >
void Run_ReadSeedData_GetAllAccounts_OK_ReuseStatement_Test( uint32 task_count, uint32 reuse_loops )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", true );
	ASSERT_TRUE( connection != nullptr );

	for ( uint32 j = 0; j < reuse_loops; ++j )
	{
		TDatabaseTaskBatch< CGetAllAccountsProcedureCall< ISIZE, OSIZE > > db_task_batch;
		std::vector< CGetAllAccountsProcedureCall< ISIZE, OSIZE > * > tasks;
		for ( uint32 i = 0; i < task_count; ++i )
		{
			CGetAllAccountsProcedureCall< ISIZE, OSIZE > *db_task = new CGetAllAccountsProcedureCall< ISIZE, OSIZE >;
			tasks.push_back( db_task );
			db_task_batch.Add_Task( db_task );
		}

		DBTaskListType successful_tasks;
		DBTaskListType failed_tasks;
		db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

		ASSERT_TRUE( failed_tasks.size() == 0 );
		ASSERT_TRUE( successful_tasks.size() == task_count );
	
		for ( uint32 i = 0; i < tasks.size(); ++i )
		{
			tasks[ i ]->Verify_Results();
			delete tasks[ i ];
		}
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_1_1_1_OK_UseCache )
{
	Run_ReadSeedData_GetAllAccounts_OK_ReuseStatement_Test< 1, 1 >( 1, 2 );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_2_2_5_OK_UseCache )
{
	Run_ReadSeedData_GetAllAccounts_OK_ReuseStatement_Test< 2, 2 >( 5, 2 );
}

TEST_F( ODBCSuccessTests, ReadSeededData_GetAllAccounts_2_2_7_OK_UseCache )
{
	Run_ReadSeedData_GetAllAccounts_OK_ReuseStatement_Test< 2, 2 >( 7, 4 );
}

/////////////////////////////

class CGetAllAccountsInOutParams : public IDatabaseVariableSet
{
	public:

		CGetAllAccountsInOutParams( void ) :
			FilteredNickname(),
			InTest(),
			OutTest(),
			Count()
		{}

		CGetAllAccountsInOutParams( const CGetAllAccountsInOutParams &rhs ) :
			FilteredNickname( rhs.FilteredNickname ),
			InTest( rhs.InTest ),
			Count( rhs.Count ),
			OutTest( rhs.OutTest )
		{}

		CGetAllAccountsInOutParams( const std::string &filtered_nickname, uint64 in_test, uint64 out_test ) :
			FilteredNickname( filtered_nickname ),
			InTest( in_test ),
			Count( 0 ),
			OutTest( out_test )
		{}

		virtual ~CGetAllAccountsInOutParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &FilteredNickname );
			variables.push_back( &InTest );
			variables.push_back( &Count );
			variables.push_back( &OutTest );
		}

		DBString< 32 > FilteredNickname;
		DBUInt64InOut InTest;
		DBUInt64InOut Count;
		DBUInt64InOut OutTest;
};

template< uint32 ISIZE, uint32 OSIZE >
class CGetAllAccountsInOutProcedureCall : public TDatabaseProcedureCall< CGetAllAccountsInOutParams, ISIZE, CGetAccountProcedureResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CGetAllAccountsInOutParams, ISIZE, CGetAccountProcedureResultSet, OSIZE > BASECLASS;

		CGetAllAccountsInOutProcedureCall( const std::string &filter_nickname, uint64 in_test, uint64 out_test ) : 
			BASECLASS(),
			FilterNickname( filter_nickname ),
			InTest( in_test ),
			OutTest( out_test ),
			FinalInTest( 0 ),
			FinalOutTest( 0 ),
			FinalCount( 0 ),
			Accounts(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CGetAllAccountsInOutProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.get_all_accounts_with_in_out"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );
			ASSERT_TRUE( FinalCount == 3 );

			for ( uint32 i = 0; i < Accounts.size(); ++i )
			{
				uint64 current_account_id = Accounts[ i ].AccountID.Get_Value();

				ASSERT_TRUE( _stricmp( Accounts[ i ].Nickname.Get_Buffer(), FilterNickname.c_str() ) != 0 );

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

			ASSERT_TRUE( FinalInTest == OutTest );
			ASSERT_TRUE( FinalOutTest == InTest );
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			CGetAllAccountsInOutParams *params = static_cast< CGetAllAccountsInOutParams * >( input_parameters );
			*params = CGetAllAccountsInOutParams( FilterNickname, InTest, OutTest );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) {
			CGetAccountProcedureResultSet *results = static_cast< CGetAccountProcedureResultSet * >( result_set );
			for ( uint32 i = 0; i < rows_fetched; ++i )
			{
				Accounts.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CGetAllAccountsInOutParams *params = static_cast< CGetAllAccountsInOutParams * >( input_parameters );

			FinalInTest = params->InTest.Get_Value();
			FinalOutTest = params->OutTest.Get_Value();
			FinalCount = params->Count.Get_Value();

			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		std::vector< CGetAccountProcedureResultSet > Accounts;

		std::string FilterNickname;

		uint64 InTest;
		uint64 OutTest;

		uint64 FinalInTest;
		uint64 FinalOutTest;
		uint64 FinalCount;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_ReadSeedData_GetAllAccountsWithInOut_OK_Test( const std::string &filtered_name, uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CGetAllAccountsInOutProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CGetAllAccountsInOutProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CGetAllAccountsInOutProcedureCall< ISIZE, OSIZE > *db_task = new CGetAllAccountsInOutProcedureCall< ISIZE, OSIZE >( filtered_name, 666, 5 );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCSuccessTests, ReadSeedData_GetAllAccountsWithInOut_OK_Test_NoFilter_2_2 )
{
	Run_ReadSeedData_GetAllAccountsWithInOut_OK_Test< 2, 2 >( std::string( "Blah" ), 2 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_GetAllAccountsWithInOut_OK_Test_BretFilter_2_2 )
{
	Run_ReadSeedData_GetAllAccountsWithInOut_OK_Test< 2, 2 >( std::string( "Bret" ), 4 );
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function tests

class CGetAccountCountParams : public IDatabaseVariableSet
{
	public:

		CGetAccountCountParams( void ) :
			AccountCount(),
			FilteredNickname()
		{}

		CGetAccountCountParams( const CGetAccountCountParams &rhs ) :
			AccountCount( rhs.AccountCount ),
			FilteredNickname( rhs.FilteredNickname )
		{}

		CGetAccountCountParams( const std::string &filtered_nickname ) :
			AccountCount(),
			FilteredNickname( filtered_nickname )
		{}

		virtual ~CGetAccountCountParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountCount );
			variables.push_back( &FilteredNickname );
		}

		DBUInt64Out AccountCount;
		DBString< 32 > FilteredNickname;
};

template< uint32 ISIZE >
class CGetAccountCountFunctionCall : public TDatabaseFunctionCall< CGetAccountCountParams, ISIZE, CEmptyVariableSet, 1 >
{
	public:

		typedef TDatabaseFunctionCall< CGetAccountCountParams, ISIZE, CEmptyVariableSet, 1 > BASECLASS;

		CGetAccountCountFunctionCall( const std::string &filter_nickname ) : 
			BASECLASS(),
			FilterNickname( filter_nickname ),
			FinalCount( 0 ),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CGetAccountCountFunctionCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.get_account_count"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );

			if ( _stricmp( FilterNickname.c_str(), "Bret" ) != 0 && _stricmp( FilterNickname.c_str(), "Peti" ) != 0 && _stricmp( FilterNickname.c_str(), "Will" ) != 0 )
			{
				ASSERT_TRUE( FinalCount == 3 );
			}
			else
			{
				ASSERT_TRUE( FinalCount == 2 );
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			CGetAccountCountParams *params = static_cast< CGetAccountCountParams * >( input_parameters );
			*params = CGetAccountCountParams( FilterNickname );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 rows_fetched ) {
			ASSERT_TRUE( rows_fetched == 0 );
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CGetAccountCountParams *params = static_cast< CGetAccountCountParams * >( input_parameters );

			FinalCount = params->AccountCount.Get_Value();

			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		std::string FilterNickname;

		uint64 FinalCount;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 ISIZE >
void Run_ReadSeedData_GetAccountCount_OK_Test( const std::string &filtered_name, uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CGetAccountCountFunctionCall< ISIZE > > db_task_batch;
	std::vector< CGetAccountCountFunctionCall< ISIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CGetAccountCountFunctionCall< ISIZE > *db_task = new CGetAccountCountFunctionCall< ISIZE >( filtered_name );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCSuccessTests, ReadSeedData_GetAccountCount_OK_Test_NoFilter_1_1 )
{
	Run_ReadSeedData_GetAccountCount_OK_Test< 1 >( std::string( "Blah" ), 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_GetAccountCount_OK_Test_NoFilter_2_3 )
{
	Run_ReadSeedData_GetAccountCount_OK_Test< 2 >( std::string( "Blah" ), 3 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_GetAccountCount_OK_Test_BretFilter_3_7 )
{
	Run_ReadSeedData_GetAccountCount_OK_Test< 3 >( std::string( "Bret" ), 7 );
}

////

class CGetAccountEmailFunctionParams : public IDatabaseVariableSet
{
	public:

		CGetAccountEmailFunctionParams( void ) :
			AccountEmail(),
			AccountID()
		{}

		CGetAccountEmailFunctionParams( const CGetAccountEmailFunctionParams &rhs ) :
			AccountEmail( rhs.AccountEmail ),
			AccountID( rhs.AccountID )
		{}

		CGetAccountEmailFunctionParams( uint64 account_id ) :
			AccountEmail(),
			AccountID( account_id )
		{}

		virtual ~CGetAccountEmailFunctionParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountEmail );
			variables.push_back( &AccountID );
		}

		DBString< 255, DVT_OUTPUT > AccountEmail;
		DBUInt64In AccountID;
};

template< uint32 ISIZE >
class CGetAccountEmailFunctionCall : public TDatabaseFunctionCall< CGetAccountEmailFunctionParams, ISIZE, CEmptyVariableSet, 1 >
{
	public:

		typedef TDatabaseFunctionCall< CGetAccountEmailFunctionParams, ISIZE, CEmptyVariableSet, 1 > BASECLASS;

		CGetAccountEmailFunctionCall( uint64 account_id ) : 
			BASECLASS(),
			AccountID( account_id ),
			FinalEmail( "" ),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CGetAccountEmailFunctionCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.get_account_email_by_id"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );

			switch ( AccountID )
			{
				case 1:
					ASSERT_TRUE( _stricmp( FinalEmail.c_str(), "bretambrose@gmail.com" ) == 0 );
					break;

				case 2:
					ASSERT_TRUE( _stricmp( FinalEmail.c_str(), "petra222@yahoo.com" ) == 0 );
					break;

				case 3:
					ASSERT_TRUE( _stricmp( FinalEmail.c_str(), "will@mailinator.com" ) == 0 );
					break;

				default:
					ASSERT_TRUE( FinalEmail.size() == 0 );
					break;
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			CGetAccountEmailFunctionParams *params = static_cast< CGetAccountEmailFunctionParams * >( input_parameters );
			*params = CGetAccountEmailFunctionParams( AccountID );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 rows_fetched ) {
			ASSERT_TRUE( rows_fetched == 0 );
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CGetAccountEmailFunctionParams *params = static_cast< CGetAccountEmailFunctionParams * >( input_parameters );

			params->AccountEmail.Copy_Into( FinalEmail );

			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint64 AccountID;

		std::string FinalEmail;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 ISIZE >
void Run_ReadSeedData_GetAccountEmail_OK_Test( uint64 account_id, uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CGetAccountEmailFunctionCall< ISIZE > > db_task_batch;
	std::vector< CGetAccountEmailFunctionCall< ISIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CGetAccountEmailFunctionCall< ISIZE > *db_task = new CGetAccountEmailFunctionCall< ISIZE >( account_id );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCSuccessTests, ReadSeedData_GetAccountEmail_OK_Test_GoodID_1_1 )
{
	Run_ReadSeedData_GetAccountEmail_OK_Test< 1 >( 1, 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_GetAccountEmail_OK_Test_GoodID_2_3 )
{
	Run_ReadSeedData_GetAccountEmail_OK_Test< 2 >( 3, 3 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_GetAccountEmail_OK_Test_BadID_3_7 )
{
	Run_ReadSeedData_GetAccountEmail_OK_Test< 3 >( 0, 7 );
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Procedure with no input, no results

template< uint32 ISIZE >
class CDoNothingProcedureCall : public TDatabaseProcedureCall< CEmptyVariableSet, ISIZE, CEmptyVariableSet, 1 >
{
	public:

		typedef TDatabaseProcedureCall< CEmptyVariableSet, ISIZE, CEmptyVariableSet, 1 > BASECLASS;

		CDoNothingProcedureCall( void ) : 
			BASECLASS(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CDoNothingProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.do_nothing"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) 
		{ 
			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 rows_fetched ) {
			ASSERT_TRUE( rows_fetched == 0 );
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) 
		{ 
			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 ISIZE >
void Run_DoNothing_OK_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CDoNothingProcedureCall< ISIZE > > db_task_batch;
	std::vector< CDoNothingProcedureCall< ISIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CDoNothingProcedureCall< ISIZE > *db_task = new CDoNothingProcedureCall< ISIZE >();
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCSuccessTests, DoNothing_OK_Test_1_1 )
{
	Run_DoNothing_OK_Test< 1 >( 1 );
}

TEST_F( ODBCSuccessTests, DoNothing_OK_Test_2_3 )
{
	Run_DoNothing_OK_Test< 2 >( 3 );
}

TEST_F( ODBCSuccessTests, DoNothing_OK_Test_3_7 )
{
	Run_DoNothing_OK_Test< 3 >( 7 );
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Boolean input/output test


class CTestBooleanDataParams : public IDatabaseVariableSet
{
	public:

		CTestBooleanDataParams( void ) :
			TestIn(),
			TestInOut()
		{}

		CTestBooleanDataParams( const CTestBooleanDataParams &rhs ) :
			TestIn( rhs.TestIn ),
			TestInOut( rhs.TestInOut )
		{}

		CTestBooleanDataParams( bool in, bool in_out ) :
			TestIn( in ),
			TestInOut( in_out )
		{}

		virtual ~CTestBooleanDataParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &TestIn );
			variables.push_back( &TestInOut );
		}

		DBBoolIn TestIn;
		DBBoolInOut TestInOut;
};

class CTestBooleanDataResultSet : public IDatabaseVariableSet
{
	public:

		CTestBooleanDataResultSet( void ) :
			Test()
		{}

		CTestBooleanDataResultSet( const CTestBooleanDataResultSet &rhs ) :
			Test( rhs.Test )
		{}

		virtual ~CTestBooleanDataResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &Test );
		}

		DBBoolIn Test;
};

template< uint32 ISIZE, uint32 OSIZE >
class CTestBooleanDataProcedureCall : public TDatabaseProcedureCall< CTestBooleanDataParams, ISIZE, CTestBooleanDataResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CTestBooleanDataParams, ISIZE, CTestBooleanDataResultSet, OSIZE > BASECLASS;

		CTestBooleanDataProcedureCall( bool in, bool in_out ) : 
			BASECLASS(),
			In( in ),
			InOut( in_out ),
			FinalInOut( false ),
			Results(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CTestBooleanDataProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.test_boolean_data"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );
			ASSERT_TRUE( FinalInOut == ( InOut ^ In ) );
			
			ASSERT_TRUE( Results.size() == 3 );
			for ( uint32 i = 0; i < 3; ++i )
			{
				ASSERT_TRUE( Results[ i ].Test.Get_Value() == ( i == 0 ) );
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			CTestBooleanDataParams *params = static_cast< CTestBooleanDataParams * >( input_parameters );
			*params = CTestBooleanDataParams( In, InOut );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CTestBooleanDataResultSet *results = static_cast< CTestBooleanDataResultSet * >( result_set );

			for ( uint32 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CTestBooleanDataParams *params = static_cast< CTestBooleanDataParams * >( input_parameters );

			FinalInOut = params->TestInOut.Get_Value();

			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { Results.clear(); ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		bool In;
		bool InOut;

		bool FinalInOut;

		std::vector< CTestBooleanDataResultSet > Results;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_ReadSeedData_TestBooleanData_OK_Test( bool in, bool in_out, uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CTestBooleanDataProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CTestBooleanDataProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CTestBooleanDataProcedureCall< ISIZE, OSIZE > *db_task = new CTestBooleanDataProcedureCall< ISIZE, OSIZE >( in, in_out );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TestBooleanData_OK_Test_1_1_1_true_true )
{
	Run_ReadSeedData_TestBooleanData_OK_Test< 1, 1 >( true, true, 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TestBooleanData_OK_Test_1_1_1_true_false )
{
	Run_ReadSeedData_TestBooleanData_OK_Test< 1, 1 >( true, false, 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TestBooleanData_OK_Test_1_1_1_false_true )
{
	Run_ReadSeedData_TestBooleanData_OK_Test< 1, 1 >( false, true, 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TestBooleanData_OK_Test_1_1_1_false_false )
{
	Run_ReadSeedData_TestBooleanData_OK_Test< 1, 1 >( false, false, 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TestBooleanData_OK_Test_2_2_5_false_true )
{
	Run_ReadSeedData_TestBooleanData_OK_Test< 2, 2 >( false, true, 5 );
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Float and double input/output test

class CTestFPDataParams : public IDatabaseVariableSet
{
	public:

		CTestFPDataParams( void ) :
			TestFloatIn(),
			TestDoubleIn(),
			TestFloatInOut(),
			TestDoubleInOut()
		{}

		CTestFPDataParams( const CTestFPDataParams &rhs ) :
			TestFloatIn( rhs.TestFloatIn ),
			TestDoubleIn( rhs.TestDoubleIn ),
			TestFloatInOut( rhs.TestFloatInOut ),
			TestDoubleInOut( rhs.TestDoubleInOut )
		{}

		CTestFPDataParams( float float_in, double double_in, float float_in_out, double double_in_out ) :
			TestFloatIn( float_in ),
			TestDoubleIn( double_in ),
			TestFloatInOut( float_in_out ),
			TestDoubleInOut( double_in_out )
		{}

		virtual ~CTestFPDataParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &TestFloatIn );
			variables.push_back( &TestDoubleIn );
			variables.push_back( &TestFloatInOut );
			variables.push_back( &TestDoubleInOut );
		}

		DBFloatIn TestFloatIn;
		DBDoubleIn TestDoubleIn;
		DBFloatInOut TestFloatInOut;
		DBDoubleInOut TestDoubleInOut;
};

class CTestFPDataResultSet : public IDatabaseVariableSet
{
	public:

		CTestFPDataResultSet( void ) :
			FloatTest(),
			DoubleTest()
		{}

		CTestFPDataResultSet( const CTestFPDataResultSet &rhs ) :
			FloatTest( rhs.FloatTest ),
			DoubleTest( rhs.DoubleTest )
		{}

		virtual ~CTestFPDataResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &FloatTest );
			variables.push_back( &DoubleTest );
		}

		DBFloatIn FloatTest;
		DBDoubleIn DoubleTest;
};

template< uint32 ISIZE, uint32 OSIZE >
class CTestFPDataProcedureCall : public TDatabaseProcedureCall< CTestFPDataParams, ISIZE, CTestFPDataResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CTestFPDataParams, ISIZE, CTestFPDataResultSet, OSIZE > BASECLASS;

		CTestFPDataProcedureCall( float float_in, double double_in, float float_in_out, double double_in_out ) : 
			BASECLASS(),
			FloatIn( float_in ),
			DoubleIn( double_in ),
			FloatInOut( float_in_out ),
			DoubleInOut( double_in_out ),
			FinalFloatInOut( 0.0f ),
			FinalDoubleInOut( 0.0 ),
			Results(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CTestFPDataProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.test_fp_data"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );
			ASSERT_FLOAT_EQ( FinalFloatInOut, FloatIn + FloatInOut );
			ASSERT_DOUBLE_EQ( FinalDoubleInOut, DoubleIn + DoubleInOut );
			
			ASSERT_TRUE( Results.size() == 3 );
			for ( uint32 i = 0; i < 3; ++i )
			{
				ASSERT_FLOAT_EQ( Results[ i ].FloatTest.Get_Value(), static_cast< float >( sqrt( static_cast< float >( i + 1 ) ) ) );
				ASSERT_DOUBLE_EQ( Results[ i ].DoubleTest.Get_Value(), sqrt( static_cast< double >( i + 2 ) ) );
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			CTestFPDataParams *params = static_cast< CTestFPDataParams * >( input_parameters );
			*params = CTestFPDataParams( FloatIn, DoubleIn, FloatInOut, DoubleInOut );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CTestFPDataResultSet *results = static_cast< CTestFPDataResultSet * >( result_set );

			for ( uint32 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CTestFPDataParams *params = static_cast< CTestFPDataParams * >( input_parameters );

			FinalFloatInOut = params->TestFloatInOut.Get_Value();
			FinalDoubleInOut = params->TestDoubleInOut.Get_Value();

			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { Results.clear(); ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		float FloatIn;
		double DoubleIn;
		float FloatInOut;
		double DoubleInOut;

		float FinalFloatInOut;
		double FinalDoubleInOut;

		std::vector< CTestFPDataResultSet > Results;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_ReadSeedData_TestFPData_OK_Test( float float_in, double double_in, float float_in_out, double double_in_out, uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CTestFPDataProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CTestFPDataProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CTestFPDataProcedureCall< ISIZE, OSIZE > *db_task = new CTestFPDataProcedureCall< ISIZE, OSIZE >( float_in, double_in, float_in_out, double_in_out );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TestFPData_OK_Test_1_1_1 )
{
	Run_ReadSeedData_TestFPData_OK_Test< 1, 1 >( 1.0f, 2.0, 3.0f, 4.0, 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TestFPData_OK_Test_3_2_7 )
{
	Run_ReadSeedData_TestFPData_OK_Test< 3, 2 >( 1.0f, 2.0, 3.0f, 4.0, 7 );
}