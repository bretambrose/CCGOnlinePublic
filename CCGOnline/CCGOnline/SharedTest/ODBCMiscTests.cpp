/**********************************************************************************************************************

	ODBCMiscTests.cpp
		defines some miscellaneous unit tests for ODBC wrapper functionality

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
#include "Database/Interfaces/DatabaseTaskInterface.h"
#include "Database/ODBCImplementation/ODBCParameters.h"
#include "Database/ODBCImplementation/ODBCVariableSet.h"
#include "Database/EmptyVariableSet.h"
#include "Database/DatabaseCalls.h"

class ODBCMiscTests : public testing::Test 
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

TEST_F( ODBCMiscTests, AllocateAndCleanup )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	IDatabaseStatement *statement = connection->Allocate_Statement( L"{call dynamic.get_all_accounts()}" );
	ASSERT_TRUE( statement != nullptr );

	connection->Release_Statement( statement );
	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}

class CGetAccountResultSet : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CGetAccountResultSet( void ) :
			BASECLASS(),
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		virtual ~CGetAccountResultSet() {}

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


TEST_F( ODBCMiscTests, ReadSeededData_GetAllAccounts1_OK )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	IDatabaseStatement *statement = connection->Allocate_Statement( L"{call dynamic.get_all_accounts()}" );
	ASSERT_TRUE( statement != nullptr );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );
	ASSERT_TRUE( statement->Needs_Binding() );	// Not cached, so bind unconditionally

	CEmptyVariableSet *params_array = new CEmptyVariableSet[ 1 ];
	statement->Bind_Input( params_array, sizeof( CEmptyVariableSet ) );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );

	CGetAccountResultSet result_set;
	statement->Bind_Output( &result_set, sizeof( CGetAccountResultSet ), 1 );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );

	ASSERT_TRUE( statement->Is_Ready_For_Use() );

	statement->Execute( 1 );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );

	int64 rows_fetched = 0;
	EFetchResultsStatusType fetch_status = FRST_ONGOING;
	uint64 expected_account_id = 1;
	while ( fetch_status != FRST_ERROR && fetch_status != FRST_FINISHED_ALL )
	{
		fetch_status = statement->Fetch_Results( rows_fetched );
		ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );
		ASSERT_TRUE( rows_fetched == 1 || fetch_status == FRST_FINISHED_ALL );
		if ( fetch_status == FRST_FINISHED_ALL )
		{
			break;
		}

		uint64 current_account_id = result_set.AccountID.Get_Value();
		ASSERT_TRUE( current_account_id == expected_account_id );

		switch ( current_account_id )
		{
			case 1:
				ASSERT_TRUE( _stricmp( result_set.AccountEmail.Get_Buffer(), "bretambrose@gmail.com" ) == 0 );
				ASSERT_TRUE( _stricmp( result_set.Nickname.Get_Buffer(), "Bret" ) == 0 );
				ASSERT_TRUE( result_set.NicknameSequenceID.Get_Value() == 1 );
				break;

			case 2:
				ASSERT_TRUE( _stricmp( result_set.AccountEmail.Get_Buffer(), "petra222@yahoo.com" ) == 0 );
				ASSERT_TRUE( _stricmp( result_set.Nickname.Get_Buffer(), "Peti" ) == 0 );
				ASSERT_TRUE( result_set.NicknameSequenceID.Get_Value() == 1 );
				break;

			case 3:
				ASSERT_TRUE( _stricmp( result_set.AccountEmail.Get_Buffer(), "will@mailinator.com" ) == 0 );
				ASSERT_TRUE( _stricmp( result_set.Nickname.Get_Buffer(), "Will" ) == 0 );
				ASSERT_TRUE( result_set.NicknameSequenceID.Get_Value() == 1 );
				break;

			default:
				ASSERT_TRUE( false );
				break;
		}

		expected_account_id++;
	}

	connection->End_Transaction( true );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );
	ASSERT_TRUE( statement->Is_Ready_For_Use() );

	connection->Release_Statement( statement );
	ASSERT_TRUE( connection->Get_Error_State() == DBEST_SUCCESS );

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;

	delete []params_array;
}

TEST_F( ODBCMiscTests, ReadSeededData_GetAllAccounts2_OK )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	IDatabaseStatement *statement = connection->Allocate_Statement( L"{call dynamic.get_all_accounts()}" );
	ASSERT_TRUE( statement != nullptr );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );
	ASSERT_TRUE( statement->Needs_Binding() );	// Not cached, so bind unconditionally

	CEmptyVariableSet params;
	statement->Bind_Input( &params, sizeof( CEmptyVariableSet ) );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );

	static const uint32 RESULT_SET_SIZE = 2;
	static const uint32 TOTAL_ROWS = 3;

	CGetAccountResultSet result_set[ RESULT_SET_SIZE ];
	statement->Bind_Output( result_set, sizeof( CGetAccountResultSet ), RESULT_SET_SIZE );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );
	ASSERT_TRUE( statement->Is_Ready_For_Use() );

	statement->Execute( 1 );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );

	int64 rows_fetched = 0;
	EFetchResultsStatusType fetch_status = FRST_ONGOING;
	uint64 expected_account_id = 1;
	while ( fetch_status != FRST_ERROR && fetch_status != FRST_FINISHED_ALL )
	{
		fetch_status = statement->Fetch_Results( rows_fetched );
		ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );
		ASSERT_TRUE( rows_fetched <= RESULT_SET_SIZE );
		ASSERT_TRUE( rows_fetched + expected_account_id <= TOTAL_ROWS + 1 );
		if ( fetch_status == FRST_FINISHED_ALL )
		{
			break;
		}

		for ( uint32 i = 0; i < rows_fetched; ++i )
		{
			uint64 current_account_id = result_set[ i ].AccountID.Get_Value();
			ASSERT_TRUE( current_account_id == expected_account_id );

			switch ( current_account_id )
			{
				case 1:
					ASSERT_TRUE( _stricmp( result_set[ i ].AccountEmail.Get_Buffer(), "bretambrose@gmail.com" ) == 0 );
					ASSERT_TRUE( _stricmp( result_set[ i ].Nickname.Get_Buffer(), "Bret" ) == 0 );
					ASSERT_TRUE( result_set[ i ].NicknameSequenceID.Get_Value() == 1 );
					break;

				case 2:
					ASSERT_TRUE( _stricmp( result_set[ i ].AccountEmail.Get_Buffer(), "petra222@yahoo.com" ) == 0 );
					ASSERT_TRUE( _stricmp( result_set[ i ].Nickname.Get_Buffer(), "Peti" ) == 0 );
					ASSERT_TRUE( result_set[ i ].NicknameSequenceID.Get_Value() == 1 );
					break;

				case 3:
					ASSERT_TRUE( _stricmp( result_set[ i ].AccountEmail.Get_Buffer(), "will@mailinator.com" ) == 0 );
					ASSERT_TRUE( _stricmp( result_set[ i ].Nickname.Get_Buffer(), "Will" ) == 0 );
					ASSERT_TRUE( result_set[ i ].NicknameSequenceID.Get_Value() == 1 );
					break;

				default:
					ASSERT_TRUE( false );
					break;
			}

			expected_account_id++;
		}
	}

	ASSERT_TRUE( expected_account_id == TOTAL_ROWS + 1 );

	connection->End_Transaction( true );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );
	ASSERT_TRUE( statement->Is_Ready_For_Use() );

	connection->Release_Statement( statement );
	ASSERT_TRUE( connection->Get_Error_State() == DBEST_SUCCESS );

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}

// valid as function input, invalid all other times
class CTestSignatureValidation1 : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestSignatureValidation1( void ) :
			BASECLASS(),
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		virtual ~CTestSignatureValidation1() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &AccountID );
			variables.push_back( &AccountEmail );
			variables.push_back( &Nickname );
			variables.push_back( &NicknameSequenceID );
		}

		DBUInt64Out AccountID;
		DBString< 255 > AccountEmail;
		DBString< 32 > Nickname;
		DBUInt32In NicknameSequenceID;
};

// valid as procedure input or output, invalid all other times
class CTestSignatureValidation2 : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestSignatureValidation2( void ) :
			BASECLASS(),
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		virtual ~CTestSignatureValidation2() {}

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

// never valid as input or output
class CTestSignatureValidation3 : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestSignatureValidation3( void ) :
			BASECLASS(),
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		virtual ~CTestSignatureValidation3() {}

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
		DBUInt32Out NicknameSequenceID;
};

// valid as procedure input if there is non-empty result set
class CTestSignatureValidation4 : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CTestSignatureValidation4( void ) :
			BASECLASS(),
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		virtual ~CTestSignatureValidation4() {}

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
		DBUInt32InOut NicknameSequenceID;
};

class CDummyTask : public CDatabaseTaskBase
{
	public:

		typedef CDatabaseTaskBase BASECLASS;

		CDummyTask( void ) :
			BASECLASS()
		{}

		virtual ~CDummyTask() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L""; }
		virtual void Build_Column_Name_List( std::vector< const wchar_t * > & /*column_names*/ ) const {}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) {}	
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) {}		
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) {}

		virtual void On_Rollback( void ) {}
		virtual void On_Task_Success( void ) {}				
		virtual void On_Task_Failure( void ) {}			
};

class CSignatureProcedureCallTask : public CDummyTask
{
	public:

		CSignatureProcedureCallTask( void ) {}
		virtual ~CSignatureProcedureCallTask() {}

		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_PROCEDURE_CALL; }
};

class CSignatureFunctionCallTask : public CDummyTask
{
	public:

		CSignatureFunctionCallTask( void ) {}
		virtual ~CSignatureFunctionCallTask() {}

		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_FUNCTION_CALL; }
};

class CSignatureGoodSelectTask : public CDummyTask
{
	public:

		CSignatureGoodSelectTask( void ) {}
		virtual ~CSignatureGoodSelectTask() {}

		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_SELECT; }
		virtual void Build_Column_Name_List( std::vector< const wchar_t * > &column_names ) const 
		{
			column_names.push_back( L"test1" );
			column_names.push_back( L"test2" );
			column_names.push_back( L"test3" );
			column_names.push_back( L"test4" );
		}

};

class CSignatureBadSelectTask : public CDummyTask
{
	public:

		CSignatureBadSelectTask( void ) {}
		virtual ~CSignatureBadSelectTask() {}

		virtual EDatabaseTaskType Get_Task_Type( void ) const { return DTT_SELECT; }
		virtual void Build_Column_Name_List( std::vector< const wchar_t * > &column_names ) const 
		{
			column_names.push_back( L"test1" );
			column_names.push_back( L"test2" );
			column_names.push_back( L"test3" );
		}

};

TEST_F( ODBCMiscTests, TestSignatureValidation )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	CTestSignatureValidation1 params1;
	CTestSignatureValidation2 params2;
	CTestSignatureValidation3 params3;
	CTestSignatureValidation4 params4;
	CEmptyVariableSet empty_params;

	CSignatureProcedureCallTask *procedure_call_task = new CSignatureProcedureCallTask;
	CSignatureFunctionCallTask *function_call_task = new CSignatureFunctionCallTask;
	CSignatureGoodSelectTask *good_select_task = new CSignatureGoodSelectTask;
	CSignatureBadSelectTask *bad_select_task = new CSignatureBadSelectTask;

	ASSERT_TRUE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params2, &params2 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params1, &params2 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params2, &params1 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params2, &params3 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params3, &params2 ) );
	ASSERT_TRUE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params4, &params2 ) );
	ASSERT_TRUE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params4, &empty_params ) );

	ASSERT_TRUE( connection->Validate_Input_Output_Signatures( procedure_call_task, &empty_params, &empty_params ) );
	ASSERT_TRUE( connection->Validate_Input_Output_Signatures( procedure_call_task, &empty_params, &params2 ) );
	ASSERT_TRUE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params2, &empty_params ) );

	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( procedure_call_task, &empty_params, &params1 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params1, &empty_params ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( procedure_call_task, &empty_params, &params3 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( procedure_call_task, &params3, &empty_params ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( procedure_call_task, &empty_params, &params4 ) );

	ASSERT_TRUE( connection->Validate_Input_Output_Signatures( function_call_task, &params1, &empty_params ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &params2, &params1 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &params1, &params2 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &params2, &empty_params ) );

	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &params3, &params2 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &params3, &empty_params ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &params3, &params1 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &params4, &empty_params ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &params4, &params1 ) );

	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &empty_params, &params1 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &empty_params, &params2 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &empty_params, &params3 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &empty_params, &params4 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( function_call_task, &empty_params, &empty_params ) );

	ASSERT_TRUE( connection->Validate_Input_Output_Signatures( good_select_task, &empty_params, &params2 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( good_select_task, &empty_params, &empty_params ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( good_select_task, &params2, &params2 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( bad_select_task, &empty_params, &params2 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( good_select_task, &empty_params, &params3 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( good_select_task, &empty_params, &params1 ) );
	ASSERT_FALSE( connection->Validate_Input_Output_Signatures( good_select_task, &empty_params, &params4 ) );

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
	delete connection;
}
