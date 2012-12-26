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
#include "Database/ODBCImplementation/ODBCParameters.h"
#include "Database/DatabaseVariableSet.h"

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

}

class CGetAccountResultSet : public IDatabaseVariableSet
{
	public:

		CGetAccountResultSet( void ) :
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

	statement->End_Transaction( true );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );
	ASSERT_TRUE( statement->Is_Ready_For_Use() );

	connection->Release_Statement( statement );
	ASSERT_TRUE( connection->Get_Error_State() == DBEST_SUCCESS );

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );

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

	statement->End_Transaction( true );
	ASSERT_TRUE( statement->Get_Error_State() == DBEST_SUCCESS );
	ASSERT_TRUE( statement->Is_Ready_For_Use() );

	connection->Release_Statement( statement );
	ASSERT_TRUE( connection->Get_Error_State() == DBEST_SUCCESS );

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

class CTestInputValidation1 : public IDatabaseVariableSet
{
	public:

		CTestInputValidation1( void ) :
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		CTestInputValidation1( const CTestInputValidation1 &rhs ) :
			AccountID( rhs.AccountID ),
			AccountEmail( rhs.AccountEmail ),
			Nickname( rhs.Nickname ),
			NicknameSequenceID( rhs.NicknameSequenceID )
		{}

		virtual ~CTestInputValidation1() {}

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

TEST_F( ODBCMiscTests, TestInputValidation1 )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	CTestInputValidation1 params;
	ASSERT_FALSE( connection->Validate_Input_Signature( DTT_PROCEDURE_CALL, &params ) );
	ASSERT_FALSE( connection->Validate_Output_Signature( DTT_PROCEDURE_CALL, &params ) );
	ASSERT_TRUE( connection->Validate_Input_Signature( DTT_FUNCTION_CALL, &params ) );
	ASSERT_FALSE( connection->Validate_Output_Signature( DTT_FUNCTION_CALL, &params ) );

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

class CTestInputValidation2 : public IDatabaseVariableSet
{
	public:

		CTestInputValidation2( void ) :
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		CTestInputValidation2( const CTestInputValidation2 &rhs ) :
			AccountID( rhs.AccountID ),
			AccountEmail( rhs.AccountEmail ),
			Nickname( rhs.Nickname ),
			NicknameSequenceID( rhs.NicknameSequenceID )
		{}

		virtual ~CTestInputValidation2() {}

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

TEST_F( ODBCMiscTests, TestInputValidation2 )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	CTestInputValidation2 params;
	ASSERT_TRUE( connection->Validate_Input_Signature( DTT_PROCEDURE_CALL, &params ) );
	ASSERT_TRUE( connection->Validate_Output_Signature( DTT_PROCEDURE_CALL, &params ) );
	ASSERT_FALSE( connection->Validate_Input_Signature( DTT_FUNCTION_CALL, &params ) );
	ASSERT_FALSE( connection->Validate_Output_Signature( DTT_FUNCTION_CALL, &params ) );

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

class CTestInputValidation3 : public IDatabaseVariableSet
{
	public:

		CTestInputValidation3( void ) :
			AccountID(),
			AccountEmail(),
			Nickname(),
			NicknameSequenceID()
		{}

		CTestInputValidation3( const CTestInputValidation3 &rhs ) :
			AccountID( rhs.AccountID ),
			AccountEmail( rhs.AccountEmail ),
			Nickname( rhs.Nickname ),
			NicknameSequenceID( rhs.NicknameSequenceID )
		{}

		virtual ~CTestInputValidation3() {}

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

TEST_F( ODBCMiscTests, TestInputValidation3 )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	CTestInputValidation3 params;
	ASSERT_FALSE( connection->Validate_Input_Signature( DTT_PROCEDURE_CALL, &params ) );
	ASSERT_FALSE( connection->Validate_Output_Signature( DTT_PROCEDURE_CALL, &params ) );
	ASSERT_FALSE( connection->Validate_Input_Signature( DTT_FUNCTION_CALL, &params ) );
	ASSERT_FALSE( connection->Validate_Output_Signature( DTT_FUNCTION_CALL, &params ) );

	CEmptyVariableSet empty_params;
	ASSERT_TRUE( connection->Validate_Input_Signature( DTT_PROCEDURE_CALL, &empty_params ) );
	ASSERT_FALSE( connection->Validate_Input_Signature( DTT_FUNCTION_CALL, &empty_params ) );
	ASSERT_TRUE( connection->Validate_Output_Signature( DTT_PROCEDURE_CALL, &empty_params ) );
	ASSERT_TRUE( connection->Validate_Output_Signature( DTT_FUNCTION_CALL, &empty_params ) );

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}