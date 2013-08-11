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
#include "Database/ODBCImplementation/ODBCVariableSet.h"
#include "Database/EmptyVariableSet.h"
#include "Database/CompoundDatabaseTaskBatch.h"
#include "Database/DatabaseCalls.h"
#include "Database/DatabaseTaskBatch.h"
#include "ODBCShared.h"
#include "StringUtils.h"

class ODBCSuccessTests : public testing::Test 
{
	public:
	
	static void SetUpTestCase( void ) 
	{
		system( "rebuild_test_db.bat 1> nul" );

		CODBCFactory::Create_Environment();
	}

	static void TearDownTestCase( void ) 
	{
		CODBCFactory::Destroy_Environment();
	}	  

};

class CGetAccountProcedureResultSet : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CGetAccountProcedureResultSet( void ) :
			BASECLASS(),
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		CGetAccountProcedureResultSet( const CGetAccountProcedureResultSet &rhs ) :
			BASECLASS( rhs ),
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
		DBStringIn< 255 > AccountEmail;
		DBStringIn< 32 > Nickname;
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

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.get_all_accounts"; }

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

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
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

		DBTaskBaseListType successful_tasks;
		DBTaskBaseListType failed_tasks;
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
	delete connection;
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

class CGetAllAccountsInOutParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CGetAllAccountsInOutParams( void ) :
			BASECLASS(),
			FilteredNickname(),
			InTest(),
			OutTest(),
			Count()
		{}

		CGetAllAccountsInOutParams( const CGetAllAccountsInOutParams &rhs ) :
			BASECLASS( rhs ),
			FilteredNickname( rhs.FilteredNickname ),
			InTest( rhs.InTest ),
			Count( rhs.Count ),
			OutTest( rhs.OutTest )
		{}

		CGetAllAccountsInOutParams( const std::string &filtered_nickname, uint64 in_test, uint64 out_test ) :
			BASECLASS(),
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

		DBStringIn< 32 > FilteredNickname;
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

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.get_all_accounts_with_in_out"; }

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

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
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

class CGetAccountCountParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CGetAccountCountParams( void ) :
			BASECLASS(),
			AccountCount(),
			FilteredNickname()
		{}

		CGetAccountCountParams( const CGetAccountCountParams &rhs ) :
			BASECLASS( rhs ),
			AccountCount( rhs.AccountCount ),
			FilteredNickname( rhs.FilteredNickname )
		{}

		CGetAccountCountParams( const std::string &filtered_nickname ) :
			BASECLASS(),
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
		DBStringIn< 32 > FilteredNickname;
};

template< uint32 ISIZE >
class CGetAccountCountFunctionCall : public TDatabaseFunctionCall< CGetAccountCountParams, ISIZE >
{
	public:

		typedef TDatabaseFunctionCall< CGetAccountCountParams, ISIZE > BASECLASS;

		CGetAccountCountFunctionCall( const std::string &filter_nickname ) : 
			BASECLASS(),
			FilterNickname( filter_nickname ),
			FinalCount( 0 ),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CGetAccountCountFunctionCall() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.get_account_count"; }

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

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
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

class CGetAccountEmailFunctionParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CGetAccountEmailFunctionParams( void ) :
			BASECLASS(),
			AccountEmail(),
			AccountID()
		{}

		CGetAccountEmailFunctionParams( const CGetAccountEmailFunctionParams &rhs ) :
			BASECLASS( rhs ),
			AccountEmail( rhs.AccountEmail ),
			AccountID( rhs.AccountID )
		{}

		CGetAccountEmailFunctionParams( uint64 account_id ) :
			BASECLASS(),
			AccountEmail(),
			AccountID( account_id )
		{}

		virtual ~CGetAccountEmailFunctionParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountEmail );
			variables.push_back( &AccountID );
		}

		DBStringOut< 255 > AccountEmail;
		DBUInt64In AccountID;
};

template< uint32 ISIZE >
class CGetAccountEmailFunctionCall : public TDatabaseFunctionCall< CGetAccountEmailFunctionParams, ISIZE >
{
	public:

		typedef TDatabaseFunctionCall< CGetAccountEmailFunctionParams, ISIZE > BASECLASS;

		CGetAccountEmailFunctionCall( uint64 account_id ) : 
			BASECLASS(),
			AccountID( account_id ),
			FinalEmail( "" ),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CGetAccountEmailFunctionCall() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.get_account_email_by_id"; }

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

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
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

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.do_nothing"; }

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

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
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


class CTestBooleanDataParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestBooleanDataParams( void ) :
			BASECLASS(),
			TestIn(),
			TestInOut()
		{}

		CTestBooleanDataParams( const CTestBooleanDataParams &rhs ) :
			BASECLASS( rhs ),
			TestIn( rhs.TestIn ),
			TestInOut( rhs.TestInOut )
		{}

		CTestBooleanDataParams( bool in, bool in_out ) :
			BASECLASS(),
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

class CTestBooleanDataResultSet : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestBooleanDataResultSet( void ) :
			BASECLASS(),
			Test()
		{}

		CTestBooleanDataResultSet( const CTestBooleanDataResultSet &rhs ) :
			BASECLASS( rhs ),
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

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.test_boolean_data"; }

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

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
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

class CTestFPDataParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestFPDataParams( void ) :
			BASECLASS(),
			TestFloatIn(),
			TestDoubleIn(),
			TestFloatInOut(),
			TestDoubleInOut()
		{}

		CTestFPDataParams( const CTestFPDataParams &rhs ) :
			BASECLASS( rhs ),
			TestFloatIn( rhs.TestFloatIn ),
			TestDoubleIn( rhs.TestDoubleIn ),
			TestFloatInOut( rhs.TestFloatInOut ),
			TestDoubleInOut( rhs.TestDoubleInOut )
		{}

		CTestFPDataParams( float float_in, double double_in, float float_in_out, double double_in_out ) :
			BASECLASS(),
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

class CTestFPDataResultSet : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestFPDataResultSet( void ) :
			BASECLASS(),
			FloatTest(),
			DoubleTest()
		{}

		CTestFPDataResultSet( const CTestFPDataResultSet &rhs ) :
			BASECLASS( rhs ),
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

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.test_fp_data"; }

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

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}

TEST_F( ODBCSuccessTests, ReadSeedData_TestFPData_OK_Test_1_1_1 )
{
	Run_ReadSeedData_TestFPData_OK_Test< 1, 1 >( 1.0f, 2.0, 3.0f, 4.0, 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TestFPData_OK_Test_3_2_7 )
{
	Run_ReadSeedData_TestFPData_OK_Test< 3, 2 >( 1.0f, 2.0, 3.0f, 4.0, 7 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CNullableFunctionParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CNullableFunctionParams( void ) :
			BASECLASS(),
			AccountID(),
			Nickname()
		{}

		CNullableFunctionParams( const CNullableFunctionParams &rhs ) :
			BASECLASS( rhs ),
			AccountID( rhs.AccountID ),
			Nickname( rhs.Nickname )
		{}

		CNullableFunctionParams( const std::wstring &nickname ) :
			BASECLASS(),
			AccountID(),
			Nickname( nickname )
		{}

		virtual ~CNullableFunctionParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountID );
			variables.push_back( &Nickname );
		}

		DBUInt64Out AccountID;
		DBWStringIn< 64 > Nickname;
};

template< uint32 ISIZE >
class CNullableFunctionCall : public TDatabaseFunctionCall< CNullableFunctionParams, ISIZE >
{
	public:

		typedef TDatabaseFunctionCall< CNullableFunctionParams, ISIZE > BASECLASS;

		CNullableFunctionCall( const std::wstring &nickname ) : 
			BASECLASS(),
			Nickname( nickname ),
			AccountIDIsNull( false ),
			FinalAccountID( -1 ),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CNullableFunctionCall() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.get_account_by_nickname"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );

			if ( AccountIDIsNull )
			{
				ASSERT_TRUE( _wcsicmp( Nickname.c_str(), L"bret" ) != 0 && _wcsicmp( Nickname.c_str(), L"peti" ) != 0 && _wcsicmp( Nickname.c_str(), L"will" ) != 0 );
			}
			else
			{
				switch ( FinalAccountID )
				{
					case 1:
						ASSERT_TRUE( _wcsicmp( Nickname.c_str(), L"bret" ) == 0 );
						break;

					case 2:
						ASSERT_TRUE( _wcsicmp( Nickname.c_str(), L"peti" ) == 0 );
						break;

					case 3:
						ASSERT_TRUE( _wcsicmp( Nickname.c_str(), L"will" ) == 0 );
						break;

					default:
						ASSERT_TRUE( false );
						break;
				}
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			CNullableFunctionParams *params = static_cast< CNullableFunctionParams * >( input_parameters );
			*params = CNullableFunctionParams( Nickname );

			InitializeCalls++; 
		}	
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CNullableFunctionParams *params = static_cast< CNullableFunctionParams * >( input_parameters );

			if ( params->AccountID.Is_Null() )
			{
				AccountIDIsNull = true;
			}
			else
			{
				FinalAccountID = params->AccountID.Get_Value();
			}

			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		std::wstring Nickname;

		int64 FinalAccountID;
		bool AccountIDIsNull;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 ISIZE >
void Run_ReadSeedData_NullableFunction_OK_Test( uint32 task_count, const std::wstring &non_null_nickname, uint32 null_index )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CNullableFunctionCall< ISIZE > > db_task_batch;
	std::vector< CNullableFunctionCall< ISIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CNullableFunctionCall< ISIZE > *db_task = new CNullableFunctionCall< ISIZE >( ( i == null_index ) ? L"dsfhjkfsjk" : non_null_nickname );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableFunctionTest_1_1_1 )
{
	Run_ReadSeedData_NullableFunction_OK_Test< 1 >( 1, L"Bret", 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableFunctionTest_1_1_0 )
{
	Run_ReadSeedData_NullableFunction_OK_Test< 1 >( 1, L"Peti", 0 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableFunctionTest_2_5_3 )
{
	Run_ReadSeedData_NullableFunction_OK_Test< 2 >( 5, L"Will", 3 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableFunctionTest_2_5_4 )
{
	Run_ReadSeedData_NullableFunction_OK_Test< 2 >( 5, L"Will", 4 );
}

////////////////////////////////////////////////////////////////////////////////////////////////

class CNullableProcedureParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CNullableProcedureParams( void ) :
			BASECLASS(),
			NullAccountID(),
			StringInOut(),
			WStringInOut()
		{}

		CNullableProcedureParams( const CNullableProcedureParams &rhs ) :
			BASECLASS( rhs ),
			NullAccountID( rhs.NullAccountID ),
			StringInOut( rhs.StringInOut ),
			WStringInOut( rhs.WStringInOut )
		{}

		CNullableProcedureParams( uint64 account_id, bool string_null, bool wstring_null ) :
			BASECLASS(),
			NullAccountID( account_id ),
			StringInOut(),
			WStringInOut()
		{
			if ( !string_null )
			{
				StringInOut.Set_Value( "TestString" );
			}

			if ( !wstring_null )
			{
				WStringInOut.Set_Value( L"TestWString" );
			}
		}

		virtual ~CNullableProcedureParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &NullAccountID );
			variables.push_back( &StringInOut );
			variables.push_back( &WStringInOut );
		}

		DBUInt64In NullAccountID;
		DBStringInOut< 32 > StringInOut;
		DBWStringInOut< 32 > WStringInOut;
};

class CNullableProcedureResultSet : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CNullableProcedureResultSet( void ) :
			BASECLASS(),
			ID(),
			Email()
		{}

		CNullableProcedureResultSet( const CNullableProcedureResultSet &rhs ) :
			BASECLASS( rhs ),
			ID( rhs.ID ),
			Email( rhs.Email )
		{}

		virtual ~CNullableProcedureResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID );
			variables.push_back( &Email );
		}

		DBUInt64In ID;
		DBStringIn< 255 > Email;
};

template< uint32 ISIZE, uint32 OSIZE >
class CNullableProcedureCall : public TDatabaseProcedureCall< CNullableProcedureParams, ISIZE, CNullableProcedureResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CNullableProcedureParams, ISIZE, CNullableProcedureResultSet, OSIZE > BASECLASS;

		CNullableProcedureCall( uint64 null_account_id, bool string_null, bool wstring_null ) : 
			BASECLASS(),
			NullAccountID( null_account_id ),
			StringNull( string_null ),
			WStringNull( wstring_null ),
			StringOut(),
			WStringOut(),
			Results(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CNullableProcedureCall() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.nullable_procedure"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );
			
			ASSERT_TRUE( WStringOut.Is_Null() == StringNull );
			ASSERT_TRUE( StringOut.Is_Null() == WStringNull );
			
			ASSERT_TRUE( Results.size() == 3 );
			for ( uint32 i = 0; i < Results.size(); ++i )
			{
				if ( i + 1 == NullAccountID )
				{
					ASSERT_TRUE( Results[ i ].ID.Is_Null() );
				}
				else
				{
					ASSERT_FALSE( Results[ i ].ID.Is_Null() );
					ASSERT_TRUE( Results[ i ].ID.Get_Value() == i + 1 );
				}
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			CNullableProcedureParams *params = static_cast< CNullableProcedureParams * >( input_parameters );
			*params = CNullableProcedureParams( NullAccountID, StringNull, WStringNull );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CNullableProcedureResultSet *results = static_cast< CNullableProcedureResultSet * >( result_set );

			for ( uint32 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CNullableProcedureParams *params = static_cast< CNullableProcedureParams * >( input_parameters );

			StringOut = params->StringInOut;
			WStringOut = params->WStringInOut;

			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { Results.clear(); ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint64 NullAccountID;
		bool StringNull;
		bool WStringNull;

		DBStringInOut< 32 > StringOut;
		DBWStringInOut< 32 > WStringOut;

		std::vector< CNullableProcedureResultSet > Results;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_ReadSeedData_NullableProcedure_Test( uint32 task_count, uint64 null_account_id, bool string_null, bool wstring_null )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CNullableProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CNullableProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CNullableProcedureCall< ISIZE, OSIZE > *db_task = new CNullableProcedureCall< ISIZE, OSIZE >( null_account_id, string_null, wstring_null );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableProcedureTest_1_1_1_1_T_F )
{
	Run_ReadSeedData_NullableProcedure_Test< 1, 1 >( 1, 1, true, false );
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableProcedureTest_1_1_1_0_T_F )
{
	Run_ReadSeedData_NullableProcedure_Test< 1, 1 >( 1, 0, true, false );
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableProcedureTest_1_1_1_2_F_T )
{
	Run_ReadSeedData_NullableProcedure_Test< 1, 1 >( 1, 2, false, true );
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableProcedureTest_1_1_1_3_F_T )
{
	Run_ReadSeedData_NullableProcedure_Test< 1, 1 >( 1, 3, false, true );
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableProcedureTest_1_1_1_3_F_F )
{
	Run_ReadSeedData_NullableProcedure_Test< 1, 1 >( 1, 3, false, false );
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableProcedureTest_1_1_1_3_T_T )
{
	Run_ReadSeedData_NullableProcedure_Test< 1, 1 >( 1, 3, true, true );
}

TEST_F( ODBCSuccessTests, ReadSeedData_NullableProcedureTest_3_2_7_2_T_F )
{
	Run_ReadSeedData_NullableProcedure_Test< 3, 2 >( 7, 2, true, false );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSelectAccountDetailsResultSet : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CSelectAccountDetailsResultSet( void ) :
			BASECLASS(),
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		CSelectAccountDetailsResultSet( const CSelectAccountDetailsResultSet &rhs ) :
			BASECLASS( rhs ),
			AccountID( rhs.AccountID ),
			AccountEmail( rhs.AccountEmail ),
			Nickname( rhs.Nickname ),
			NicknameSequenceID( rhs.NicknameSequenceID )
		{}

		virtual ~CSelectAccountDetailsResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountID );
			variables.push_back( &AccountEmail );
			variables.push_back( &Nickname );
			variables.push_back( &NicknameSequenceID );
		}

		DBUInt64In AccountID;
		DBStringIn< 255 > AccountEmail;
		DBStringIn< 255 > Nickname;
		DBInt32In NicknameSequenceID;
};

template< uint32 OSIZE >
class CSelectAccountDetailsTask : public TDatabaseSelect< CSelectAccountDetailsResultSet, OSIZE >
{
	public:

		typedef TDatabaseSelect< CSelectAccountDetailsResultSet, OSIZE > BASECLASS;

		CSelectAccountDetailsTask( void ) : 
			BASECLASS(),
			Results(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CSelectAccountDetailsTask() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.accounts"; }
		virtual void Build_Column_Name_List( std::vector< const wchar_t * > &column_names ) const
		{
			column_names.push_back( L"account_id" );
			column_names.push_back( L"account_email" );
			column_names.push_back( L"nickname" );
			column_names.push_back( L"nickname_sequence_id" );
		}

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );
						
			ASSERT_TRUE( Results.size() == 3 );
			for ( uint32 i = 0; i < Results.size(); ++i )
			{
				ASSERT_TRUE( Results[ i ].AccountID.Get_Value() == i + 1 );
				ASSERT_TRUE( Results[ i ].NicknameSequenceID.Get_Value() == 1 );
				
				std::string email;
				Results[ i ].AccountEmail.Copy_Into( email );

				std::string nickname;
				Results[ i ].Nickname.Copy_Into( nickname );

				switch ( i )
				{
					case 0:
						ASSERT_TRUE( _stricmp( "bretambrose@gmail.com", email.c_str() ) == 0 );
						ASSERT_TRUE( _stricmp( "Bret", nickname.c_str() ) == 0 );
						break;

					case 1:
						ASSERT_TRUE( _stricmp( "petra222@yahoo.com", email.c_str() ) == 0 );
						ASSERT_TRUE( _stricmp( "Peti", nickname.c_str() ) == 0 );
						break;

					case 2:
						ASSERT_TRUE( _stricmp( "will@mailinator.com", email.c_str() ) == 0 );
						ASSERT_TRUE( _stricmp( "Will", nickname.c_str() ) == 0 );
						break;

					default:
						ASSERT_TRUE( false );
						break;
				}
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) 
		{ 
			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CSelectAccountDetailsResultSet *results = static_cast< CSelectAccountDetailsResultSet * >( result_set );

			for ( uint32 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) 
		{ 
			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { Results.clear(); ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		std::vector< CSelectAccountDetailsResultSet > Results;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 OSIZE >
void Run_ReadSeedData_SelectAccountDetails_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CSelectAccountDetailsTask< OSIZE > > db_task_batch;
	std::vector< CSelectAccountDetailsTask< OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CSelectAccountDetailsTask< OSIZE > *db_task = new CSelectAccountDetailsTask< OSIZE >;
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}

TEST_F( ODBCSuccessTests, ReadSeedData_SelectAccountDetails_1_1 )
{
	Run_ReadSeedData_SelectAccountDetails_Test< 1 >( 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_SelectAccountDetails_1_3 )
{
	Run_ReadSeedData_SelectAccountDetails_Test< 1 >( 3 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_SelectAccountDetails_4_3 )
{
	Run_ReadSeedData_SelectAccountDetails_Test< 4 >( 3 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_SelectAccountDetails_4_7 )
{
	Run_ReadSeedData_SelectAccountDetails_Test< 4 >( 7 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_SelectAccountDetails_4_17 )
{
	Run_ReadSeedData_SelectAccountDetails_Test< 4 >( 17 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CTestTVFParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestTVFParams( void ) :
			BASECLASS(),
			AccountID()
		{}

		CTestTVFParams( uint64 account_id ) :
			BASECLASS(),
			AccountID( account_id )
		{}

		virtual ~CTestTVFParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountID );
		}

		DBUInt64In AccountID;
};

class CTestTVFResultSet : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestTVFResultSet( void ) :
			BASECLASS(),
			ProductDesc(),
			ProductKeyDesc()
		{}

		virtual ~CTestTVFResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ProductDesc );
			variables.push_back( &ProductKeyDesc );
		}

		DBStringIn< 255 > ProductDesc;
		DBStringIn< 36 > ProductKeyDesc;
};

template< uint32 ISIZE, uint32 OSIZE >
class CTableValuedFunctionTask : public TDatabaseTableValuedFunctionCall< CTestTVFParams, ISIZE, CTestTVFResultSet, OSIZE >
{
	public:

		typedef TDatabaseTableValuedFunctionCall< CTestTVFParams, ISIZE, CTestTVFResultSet, OSIZE > BASECLASS;

		CTableValuedFunctionTask( uint64 account_id ) : 
			BASECLASS(),
			AccountID( account_id ),
			Results(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 )
		{}

		virtual ~CTableValuedFunctionTask() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.tabled_valued_function"; }
		virtual void Build_Column_Name_List( std::vector< const wchar_t * > &column_names ) const
		{
			column_names.push_back( L"product_desc" );
			column_names.push_back( L"product_key_desc" );
		}

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( FinishedCalls == 1 );
			ASSERT_TRUE( InitializeCalls == 1 );
			
			if ( AccountID == 1 )
			{			
				ASSERT_TRUE( Results.size() == 3 );
				for ( uint32 i = 0; i < Results.size(); ++i )
				{		
					std::string product_desc;
					Results[ i ].ProductDesc.Copy_Into( product_desc );

					ASSERT_TRUE( _stricmp( "JYHAD", product_desc.c_str() ) == 0 ||
									 _stricmp( "LEGENDOFTHEFIVERINGS", product_desc.c_str() ) == 0 ||
									 _stricmp( "NETRUNNER", product_desc.c_str() ) == 0 );
				}
			}
			else
			{
				ASSERT_TRUE( Results.size() == 0 );
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			CTestTVFParams *input_params = static_cast< CTestTVFParams * >( input_parameters );
			*input_params = CTestTVFParams( AccountID );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CTestTVFResultSet *results = static_cast< CTestTVFResultSet * >( result_set );

			for ( uint32 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) 
		{ 
			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { Results.clear(); ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint64 AccountID;

		std::vector< CTestTVFResultSet > Results;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_ReadSeedData_TableValueFunctionTest( uint32 task_count, uint64 account_id )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CTableValuedFunctionTask< ISIZE, OSIZE > > db_task_batch;
	std::vector< CTableValuedFunctionTask< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CTableValuedFunctionTask< ISIZE, OSIZE > *db_task = new CTableValuedFunctionTask< ISIZE, OSIZE >( account_id );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}

TEST_F( ODBCSuccessTests, ReadSeedData_TableValueFunctionTest_1_1_1_1 )
{
	Run_ReadSeedData_TableValueFunctionTest< 1, 1 >( 1, 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TableValueFunctionTest_2_2_3_2 )
{
	Run_ReadSeedData_TableValueFunctionTest< 2, 2 >( 3, 2 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TableValueFunctionTest_2_2_3_1 )
{
	Run_ReadSeedData_TableValueFunctionTest< 2, 2 >( 3, 1 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TableValueFunctionTest_2_2_5_3 )
{
	Run_ReadSeedData_TableValueFunctionTest< 2, 2 >( 5, 3 );
}

TEST_F( ODBCSuccessTests, ReadSeedData_TableValueFunctionTest_2_2_5_1 )
{
	Run_ReadSeedData_TableValueFunctionTest< 2, 2 >( 5, 1 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compound Tests
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ODBCCompoundSuccessTests : public testing::Test 
{
	public:
	
	static void SetUpTestCase( void ) 
	{
		system( "rebuild_test_db.bat 1> nul" );

		CODBCFactory::Create_Environment();
	}

	virtual void SetUp( void )
	{
		system( "clear_test_write_tables.bat 1> nul" );
	}

	static void TearDownTestCase( void ) 
	{
		CODBCFactory::Destroy_Environment();
	}	  

};

template< uint32 BATCH_SIZE, uint32 ISIZE1, uint32 OSIZE1 >
class CTrivialCompoundTask : public TCompoundDatabaseTask< BATCH_SIZE >
{
	public:

		typedef TCompoundDatabaseTask< BATCH_SIZE > BASECLASS;

		CTrivialCompoundTask( void ) :
			BASECLASS()
		{}

		virtual ~CTrivialCompoundTask() {}

		static void Register_Child_Tasks( ICompoundDatabaseTaskBatch *task_batch )
		{
			Register_Database_Child_Task_Type< CGetAllAccountsProcedureCall< ISIZE1, OSIZE1 > >( task_batch );
		}
		
		void Verify_Results( void )
		{
			DBTaskListType child_tasks;
			Get_All_Child_Tasks( child_tasks );

			std::for_each( child_tasks.begin(), child_tasks.end(), []( IDatabaseTask *task ) {
					CGetAllAccountsProcedureCall< ISIZE1, OSIZE1 > *child_task = static_cast< CGetAllAccountsProcedureCall< ISIZE1, OSIZE1 > * >( task );
					child_task->Verify_Results();
				}
			);
		}

		virtual void On_Task_Success( void ) {}				
		virtual void On_Task_Failure( void ) {}	

		virtual void Seed_Child_Tasks( void )
		{
			Add_Child_Task( new CGetAllAccountsProcedureCall< ISIZE1, OSIZE1 > );
		}

	private:
};

template< uint32 BATCH_SIZE, uint32 ISIZE1, uint32 OSIZE1 >
void Run_TrivialCompoundTask_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TCompoundDatabaseTaskBatch< CTrivialCompoundTask< BATCH_SIZE, ISIZE1, OSIZE1 > > db_compound_task_batch;
	std::vector< CTrivialCompoundTask< BATCH_SIZE, ISIZE1, OSIZE1 > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		auto db_task = new CTrivialCompoundTask< BATCH_SIZE, ISIZE1, OSIZE1 >;
		db_task->Set_ID( static_cast< DatabaseTaskIDType::Enum >( i + 1 ) );
		tasks.push_back( db_task );
		db_compound_task_batch.Add_Task( db_task );
	}

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_compound_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}

TEST_F( ODBCCompoundSuccessTests, TrvialCompoundTasks_1_1_1_1_OK )
{
	Run_TrivialCompoundTask_Test< 1, 1, 1 >( 1 );
}

TEST_F( ODBCCompoundSuccessTests, TrvialCompoundTasks_2_1_1_3_OK )
{
	Run_TrivialCompoundTask_Test< 2, 1, 1 >( 3 );
}

TEST_F( ODBCCompoundSuccessTests, TrvialCompoundTasks_3_2_2_7_OK )
{
	Run_TrivialCompoundTask_Test< 3, 2, 2 >( 7 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCompoundInsertParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CCompoundInsertParams( void ) :
			BASECLASS(),
			ID(),
			UserData()
		{}

		CCompoundInsertParams( uint64 id, uint64 user_data ) :
			BASECLASS(),
			ID( id ),
			UserData( user_data )
		{}

		virtual ~CCompoundInsertParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID );
			variables.push_back( &UserData );
		}

		DBUInt64In ID;
		DBUInt64In UserData;
};

template< uint32 ISIZE >
class CCompoundInsertProcedureCall : public TDatabaseProcedureCall< CCompoundInsertParams, ISIZE, CEmptyVariableSet, 1 >
{
	public:

		typedef TDatabaseProcedureCall< CCompoundInsertParams, ISIZE, CEmptyVariableSet, 1 > BASECLASS;

		CCompoundInsertProcedureCall( uint64 id, uint64 index ) : 
			BASECLASS(),
			ID( id ),
			Index( index )
		{}

		virtual ~CCompoundInsertProcedureCall() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.test_compound_insert"; }

		void Verify_Results( void ) {}

	protected:

		// WIP: signatures in flux
		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			 CCompoundInsertParams* params = static_cast< CCompoundInsertParams * >( input_parameters );
			 *params = CCompoundInsertParams( ID, Index );
	   }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 rows_fetched ) 
		{
			ASSERT_TRUE( rows_fetched == 0 );
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) {}	

		virtual void On_Rollback( void ) { ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint64 ID;
		uint64 Index;

};


template< uint32 BATCH_SIZE, uint32 ISIZE1, uint32 ISIZE2, uint32 OSIZE2 >
class CCompoundInsertTask : public TCompoundDatabaseTask< BATCH_SIZE >
{
	public:

		typedef TCompoundDatabaseTask< BATCH_SIZE > BASECLASS;

		typedef CCompoundInsertProcedureCall< ISIZE1 > Child1Type;
		typedef CCompoundInsertCountProcedureCall< ISIZE2, OSIZE2 > Child2Type;

		CCompoundInsertTask( uint64 id, uint32 insert_count ) :
			BASECLASS(),
			ID( id ),
			InsertCount( insert_count )
		{
			Register_Child_Type_Success_Callback( Loki::TypeInfo( typeid( Child1Type ) ), FastDelegate0<>(this, &CCompoundInsertTask::On_Insert_Success) );
		}

		virtual ~CCompoundInsertTask() {}

		static void Register_Child_Tasks( ICompoundDatabaseTaskBatch *task_batch )
		{
			Register_Database_Child_Task_Type< Child1Type >( task_batch );
			Register_Database_Child_Task_Type< Child2Type >( task_batch );
		}
		
		void Verify_Results( void )
		{
			DBTaskListType child_tasks;
			Get_Child_Tasks_Of_Type( Loki::TypeInfo( typeid( Child1Type ) ), child_tasks );

			std::for_each( child_tasks.begin(), child_tasks.end(), []( IDatabaseTask *task ) {
					Child1Type *child_task = static_cast< Child1Type * >( task );
					child_task->Verify_Results();
				}
			);

			child_tasks.clear();
			Get_Child_Tasks_Of_Type( Loki::TypeInfo( typeid( Child2Type ) ), child_tasks );

			std::for_each( child_tasks.begin(), child_tasks.end(), []( IDatabaseTask *task ) {
					Child2Type *child_task = static_cast< Child2Type * >( task );
					child_task->Verify_Results();
				}
			);
		}

		virtual void On_Task_Success( void ) {}				
		virtual void On_Task_Failure( void ) {}	

		virtual void Seed_Child_Tasks( void )
		{
			for( uint32 i = 0; i < InsertCount; ++i )
			{
				Add_Child_Task( new Child1Type( ID, i + 1 ) );
			}
		}

	private:

		void On_Insert_Success()
		{
			Add_Child_Task( new Child2Type( ID, InsertCount ) );
		}

		uint64 ID;
		uint32 InsertCount;
};


template< uint32 BATCH_SIZE, uint32 ISIZE1, uint32 ISIZE2, uint32 INSERT_COUNT >
void Run_CompoundInsertTask_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	typedef CCompoundInsertTask< BATCH_SIZE, ISIZE1, ISIZE2, 1 > CompoundTaskType;
	TCompoundDatabaseTaskBatch< CompoundTaskType > db_compound_task_batch;
	std::vector< CompoundTaskType * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		auto db_task = new CompoundTaskType( i + 1, INSERT_COUNT );
		db_task->Set_ID( static_cast< DatabaseTaskIDType::Enum >( i + 1 ) );
		tasks.push_back( db_task );
		db_compound_task_batch.Add_Task( db_task );
	}

	DBTaskBaseListType successful_tasks;
	DBTaskBaseListType failed_tasks;
	db_compound_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == 0 );
	ASSERT_TRUE( successful_tasks.size() == task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}

TEST_F( ODBCCompoundSuccessTests, CompoundInsertTasks_1_1_1_1_1_OK )
{
	Run_CompoundInsertTask_Test< 1, 1, 1, 1 >( 1 );
}

TEST_F( ODBCCompoundSuccessTests, CompoundInsertTasks_2_1_1_3_1_OK )
{
	Run_CompoundInsertTask_Test< 2, 1, 1, 3 >( 1 );
}

TEST_F( ODBCCompoundSuccessTests, CompoundInsertTasks_2_1_1_3_5_OK )
{
	Run_CompoundInsertTask_Test< 2, 1, 1, 3 >( 5 );
}

TEST_F( ODBCCompoundSuccessTests, CompoundInsertTasks_2_2_2_3_5_OK )
{
	Run_CompoundInsertTask_Test< 2, 2, 2, 3 >( 5 );
}

TEST_F( ODBCCompoundSuccessTests, CompoundInsertTasks_2_2_2_5_3_OK )
{
	Run_CompoundInsertTask_Test< 2, 2, 2, 5 >( 3 );
}