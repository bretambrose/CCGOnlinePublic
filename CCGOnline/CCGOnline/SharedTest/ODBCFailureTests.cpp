/**********************************************************************************************************************

	ODBCFailureTests.cpp
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

class ODBCFailureTests : public testing::Test 
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

template< uint32 ISIZE, uint32 OSIZE >
class CMissingProcedureCall : public TDatabaseProcedureCall< CEmptyVariableSet, ISIZE, CEmptyVariableSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CEmptyVariableSet, ISIZE, CEmptyVariableSet, OSIZE > BASECLASS;

		CMissingProcedureCall( void ) : 
			BASECLASS(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CMissingProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.missing_procedure"; }

		void Verify_Results( uint32 expected_rollback_count ) 
		{
			ASSERT_TRUE( InitializeCalls == Rollbacks + 1 );
			ASSERT_TRUE( FinishedCalls == 0 );
			ASSERT_TRUE( Rollbacks == expected_rollback_count );
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) { InitializeCalls++; }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) {}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) { FinishedCalls++;}	

		virtual void On_Rollback( void ) { Rollbacks++; }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_MissingProcedureCall_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CMissingProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CMissingProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CMissingProcedureCall< ISIZE, OSIZE > *db_task = new CMissingProcedureCall< ISIZE, OSIZE >;
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == task_count );
	ASSERT_TRUE( successful_tasks.size() == 0 );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		uint32 expected_rollback_count = i % ISIZE;	// this is probably unreliable across ODBC impl; it assumes the task list gets repeatedly rolled back in sequential order
		tasks[ i ]->Verify_Results( expected_rollback_count );
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}


TEST_F( ODBCFailureTests, MissingProcedureCall_1_1_1 )
{
	Run_MissingProcedureCall_Test< 1, 1 >( 1 );
}

TEST_F( ODBCFailureTests, MissingProcedureCall_1_1_3 )
{
	Run_MissingProcedureCall_Test< 1, 1 >( 3 );
}

TEST_F( ODBCFailureTests, MissingProcedureCall_2_1_2 )
{
	Run_MissingProcedureCall_Test< 2, 1 >( 2 );
}

TEST_F( ODBCFailureTests, MissingProcedureCall_2_1_3 )
{
	Run_MissingProcedureCall_Test< 2, 1 >( 3 );
}

TEST_F( ODBCFailureTests, MissingProcedureCall_2_1_5 )
{
	Run_MissingProcedureCall_Test< 2, 1 >( 5 );
}

TEST_F( ODBCFailureTests, MissingProcedureCall_3_1_3 )
{
	Run_MissingProcedureCall_Test< 3, 1 >( 3 );
}

class CTooFewInputParams : public IDatabaseVariableSet
{
	public:

		CTooFewInputParams( void ) :
			ID( 0 ),
			Nickname( "Bret" )
		{}

		CTooFewInputParams( const CTooFewInputParams &rhs ) :
			ID( rhs.ID ),
			Nickname( rhs.Nickname )
		{}

		virtual ~CTooFewInputParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID );
			variables.push_back( &Nickname );
		}

		DBUInt64In ID;
		DBString< 32 > Nickname;
};

class CTooManyInputParams : public IDatabaseVariableSet
{
	public:

		CTooManyInputParams( void ) :
			ID( 0 ),
			Nickname( "Bret" ),
			NicknameSequenceID( 1 ),
			ExtraParam( 2.0f )
		{}

		CTooManyInputParams( const CTooManyInputParams &rhs ) :
			ID( rhs.ID ),
			Nickname( rhs.Nickname ),
			NicknameSequenceID( rhs.NicknameSequenceID ),
			ExtraParam( rhs.ExtraParam )
		{}

		virtual ~CTooManyInputParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID );
			variables.push_back( &Nickname );
			variables.push_back( &NicknameSequenceID );
			variables.push_back( &ExtraParam );
		}

		DBUInt64In ID;
		DBStringIn< 32 > Nickname;
		DBUInt32In NicknameSequenceID;
		DBFloatIn ExtraParam;
};

template< typename IPARAMS, uint32 ISIZE, uint32 OSIZE >
class CWrongArityProcedureCall : public TDatabaseProcedureCall< IPARAMS, ISIZE, CEmptyVariableSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< IPARAMS, ISIZE, CEmptyVariableSet, OSIZE > BASECLASS;

		CWrongArityProcedureCall( void ) : 
			BASECLASS(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CWrongArityProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.arity_failure"; }

		void Verify_Results( uint32 expected_rollback_count ) 
		{
			ASSERT_TRUE( InitializeCalls == Rollbacks + 1 );
			ASSERT_TRUE( FinishedCalls == 0 );
			ASSERT_TRUE( Rollbacks == expected_rollback_count );
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) { InitializeCalls++; }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) {}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) { FinishedCalls++;}	

		virtual void On_Rollback( void ) { Rollbacks++; }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
};

template< typename IPARAMS, uint32 ISIZE, uint32 OSIZE >
void Run_WrongArityProcedureCall_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CWrongArityProcedureCall< IPARAMS, ISIZE, OSIZE > > db_task_batch;
	std::vector< CWrongArityProcedureCall< IPARAMS, ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CWrongArityProcedureCall< IPARAMS, ISIZE, OSIZE > *db_task = new CWrongArityProcedureCall< IPARAMS, ISIZE, OSIZE >;
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == task_count );
	ASSERT_TRUE( successful_tasks.size() == 0 );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		uint32 expected_rollback_count = i % ISIZE;	// this is probably unreliable across ODBC impl; it assumes the task list gets repeatedly rolled back in sequential order
		tasks[ i ]->Verify_Results( expected_rollback_count );
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, TooFewParamsProcedureCall_1_1_1 )
{
	Run_WrongArityProcedureCall_Test< CTooFewInputParams, 1, 1 >( 1 );
}

TEST_F( ODBCFailureTests, TooFewParamsProcedureCall_1_1_3 )
{
	Run_WrongArityProcedureCall_Test< CTooFewInputParams, 1, 1 >( 3 );
}

TEST_F( ODBCFailureTests, TooFewParamsProcedureCall_2_1_2 )
{
	Run_WrongArityProcedureCall_Test< CTooFewInputParams, 2, 1 >( 2 );
}

TEST_F( ODBCFailureTests, TooFewParamsProcedureCall_2_1_3 )
{
	Run_WrongArityProcedureCall_Test< CTooFewInputParams, 2, 1 >( 3 );
}

TEST_F( ODBCFailureTests, TooFewParamsProcedureCall_2_1_5 )
{
	Run_WrongArityProcedureCall_Test< CTooFewInputParams, 2, 1 >( 5 );
}

TEST_F( ODBCFailureTests, TooFewParamsProcedureCall_3_1_3 )
{
	Run_WrongArityProcedureCall_Test< CTooFewInputParams, 3, 1 >( 3 );
}

/////////////////////////////////////////////////////////////////////

TEST_F( ODBCFailureTests, TooManyParamsProcedureCall_1_1_1 )
{
	Run_WrongArityProcedureCall_Test< CTooManyInputParams, 1, 1 >( 1 );
}

TEST_F( ODBCFailureTests, TooManyParamsProcedureCall_1_1_3 )
{
	Run_WrongArityProcedureCall_Test< CTooManyInputParams, 1, 1 >( 3 );
}

TEST_F( ODBCFailureTests, TooManyParamsProcedureCall_2_1_2 )
{
	Run_WrongArityProcedureCall_Test< CTooManyInputParams, 2, 1 >( 2 );
}

TEST_F( ODBCFailureTests, TooManyParamsProcedureCall_2_1_3 )
{
	Run_WrongArityProcedureCall_Test< CTooManyInputParams, 2, 1 >( 3 );
}

TEST_F( ODBCFailureTests, TooManyParamsProcedureCall_2_1_5 )
{
	Run_WrongArityProcedureCall_Test< CTooManyInputParams, 2, 1 >( 5 );
}

TEST_F( ODBCFailureTests, TooManyParamsProcedureCall_3_1_3 )
{
	Run_WrongArityProcedureCall_Test< CTooManyInputParams, 3, 1 >( 3 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CInconvertibleProcedureParams : public IDatabaseVariableSet
{
	public:

		CInconvertibleProcedureParams( void ) :
			String( "Bret" )
		{}

		CInconvertibleProcedureParams( const CInconvertibleProcedureParams &rhs ) :
			String( rhs.String )
		{}

		virtual ~CInconvertibleProcedureParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &String );
		}

		DBStringIn< 32 > String;
};

template< uint32 ISIZE, uint32 OSIZE >
class CBadParamConversionProcedureCall : public TDatabaseProcedureCall< CInconvertibleProcedureParams, ISIZE, CEmptyVariableSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CInconvertibleProcedureParams, ISIZE, CEmptyVariableSet, OSIZE > BASECLASS;

		CBadParamConversionProcedureCall( void ) : 
			BASECLASS(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CBadParamConversionProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.bad_input_params"; }

		void Verify_Results( uint32 expected_rollback_count ) 
		{
			ASSERT_TRUE( InitializeCalls == Rollbacks + 1 );
			ASSERT_TRUE( FinishedCalls == 0 );
			ASSERT_TRUE( Rollbacks == expected_rollback_count );
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) { InitializeCalls++; }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) {}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) { FinishedCalls++;}	

		virtual void On_Rollback( void ) { Rollbacks++; }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_BadParamConversionProcedureCall_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CBadParamConversionProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CBadParamConversionProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CBadParamConversionProcedureCall< ISIZE, OSIZE > *db_task = new CBadParamConversionProcedureCall< ISIZE, OSIZE >;
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == task_count );
	ASSERT_TRUE( successful_tasks.size() == 0 );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		uint32 expected_rollback_count = i % ISIZE;	// this is probably unreliable across ODBC impl; it assumes the task list gets repeatedly rolled back in sequential order
		tasks[ i ]->Verify_Results( expected_rollback_count );
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, BadParamConversionProcedureCall_1_1_1 )
{
	Run_BadParamConversionProcedureCall_Test< 1, 1 >( 1 );
}

TEST_F( ODBCFailureTests, BadParamConversionProcedureCall_1_1_2 )
{
	Run_BadParamConversionProcedureCall_Test< 1, 1 >( 2 );
}

TEST_F( ODBCFailureTests, BadParamConversionProcedureCall_2_1_2 )
{
	Run_BadParamConversionProcedureCall_Test< 2, 1 >( 2 );
}

TEST_F( ODBCFailureTests, BadParamConversionProcedureCall_2_1_3 )
{
	Run_BadParamConversionProcedureCall_Test< 2, 1 >( 3 );
}

TEST_F( ODBCFailureTests, BadParamConversionProcedureCall_2_1_5 )
{
	Run_BadParamConversionProcedureCall_Test< 2, 1 >( 5 );
}

TEST_F( ODBCFailureTests, BadParamConversionProcedureCall_3_1_3 )
{
	Run_BadParamConversionProcedureCall_Test< 3, 1 >( 3 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CInconvertibleFunctionParams : public IDatabaseVariableSet
{
	public:

		CInconvertibleFunctionParams( void ) :
			Float(),
			ID()
		{}

		CInconvertibleFunctionParams( const CInconvertibleFunctionParams &rhs ) :
			Float( rhs.Float ),
			ID( rhs.ID )
		{}

		virtual ~CInconvertibleFunctionParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &Float );
			variables.push_back( &ID );
		}

		DBFloatOut Float;
		DBUInt64In ID;
};

template< uint32 ISIZE, uint32 OSIZE >
class CBadParamConversionFunctionCall : public TDatabaseFunctionCall< CInconvertibleFunctionParams, ISIZE, CEmptyVariableSet, OSIZE >
{
	public:

		typedef TDatabaseFunctionCall< CInconvertibleFunctionParams, ISIZE, CEmptyVariableSet, OSIZE > BASECLASS;

		CBadParamConversionFunctionCall( void ) : 
			BASECLASS(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CBadParamConversionFunctionCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.bad_function_return"; }

		void Verify_Results( uint32 expected_rollback_count ) 
		{
			ASSERT_TRUE( InitializeCalls == Rollbacks + 1 );
			ASSERT_TRUE( FinishedCalls == 0 );
			ASSERT_TRUE( Rollbacks == expected_rollback_count );
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) { InitializeCalls++; }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) {}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) { FinishedCalls++;}	

		virtual void On_Rollback( void ) { Rollbacks++; }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_BadParamConversionFunctionCall_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CBadParamConversionFunctionCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CBadParamConversionFunctionCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CBadParamConversionFunctionCall< ISIZE, OSIZE > *db_task = new CBadParamConversionFunctionCall< ISIZE, OSIZE >;
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == task_count );
	ASSERT_TRUE( successful_tasks.size() == 0 );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		uint32 expected_rollback_count = i % ISIZE;	// this is probably unreliable across ODBC impl; it assumes the task list gets repeatedly rolled back in sequential order
		tasks[ i ]->Verify_Results( expected_rollback_count );
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, BadParamConversionFunctionCall_1_1_1 )
{
	Run_BadParamConversionFunctionCall_Test< 1, 1 >( 1 );
}

TEST_F( ODBCFailureTests, BadParamConversionFunctionCall_1_1_2 )
{
	Run_BadParamConversionFunctionCall_Test< 1, 1 >( 1 );
}

TEST_F( ODBCFailureTests, BadParamConversionFunctionCall_2_1_2)
{
	Run_BadParamConversionFunctionCall_Test< 2, 1 >( 2 );
}

TEST_F( ODBCFailureTests, BadParamConversionFunctionCall_2_1_5)
{
	Run_BadParamConversionFunctionCall_Test< 2, 1 >( 5 );
}

TEST_F( ODBCFailureTests, BadParamConversionFunctionCall_3_1_3)
{
	Run_BadParamConversionFunctionCall_Test< 3, 1 >( 3 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CInconvertibleResultSet : public IDatabaseVariableSet
{
	public:

		CInconvertibleResultSet( void ) :
			Float()
		{}

		CInconvertibleResultSet( const CInconvertibleResultSet &rhs ) :
			Float( rhs.Float )
		{}

		virtual ~CInconvertibleResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &Float );
		}

		DBFloatIn Float;
};

template< uint32 ISIZE, uint32 OSIZE >
class CBadResultSetConversionProcedureCall : public TDatabaseProcedureCall< CEmptyVariableSet, ISIZE, CInconvertibleResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CEmptyVariableSet, ISIZE, CInconvertibleResultSet, OSIZE > BASECLASS;

		CBadResultSetConversionProcedureCall( void ) : 
			BASECLASS(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CBadResultSetConversionProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.bad_result_set_conversion"; }

		void Verify_Results( uint32 expected_rollback_count ) 
		{
			ASSERT_TRUE( InitializeCalls == Rollbacks + 1 );
			ASSERT_TRUE( FinishedCalls == 0 );
			ASSERT_TRUE( Rollbacks == expected_rollback_count );
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) { InitializeCalls++; }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) {}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) { FinishedCalls++;}	

		virtual void On_Rollback( void ) { Rollbacks++; }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_BadResultSetConversionProcedureCall_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CBadResultSetConversionProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CBadResultSetConversionProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CBadResultSetConversionProcedureCall< ISIZE, OSIZE > *db_task = new CBadResultSetConversionProcedureCall< ISIZE, OSIZE >;
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == task_count );
	ASSERT_TRUE( successful_tasks.size() == 0 );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		uint32 expected_rollback_count = i % ISIZE;	// this is probably unreliable across ODBC impl; it assumes the task list gets repeatedly rolled back in sequential order
		tasks[ i ]->Verify_Results( expected_rollback_count );
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, BadResultSetConversionProcedureCall_1_1_1 )
{
	Run_BadResultSetConversionProcedureCall_Test< 1, 1 >( 1 );
}

TEST_F( ODBCFailureTests, BadResultSetConversionProcedureCall_1_1_2 )
{
	Run_BadResultSetConversionProcedureCall_Test< 1, 1 >( 2 );
}

TEST_F( ODBCFailureTests, BadResultSetConversionProcedureCall_2_1_2 )
{
	Run_BadResultSetConversionProcedureCall_Test< 2, 1 >( 2 );
}

TEST_F( ODBCFailureTests, BadResultSetConversionProcedureCall_2_1_5 )
{
	Run_BadResultSetConversionProcedureCall_Test< 2, 1 >( 5 );
}

TEST_F( ODBCFailureTests, BadResultSetConversionProcedureCall_3_1_3 )
{
	Run_BadResultSetConversionProcedureCall_Test< 3, 1 >( 3 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CFunctionInputProcedureSet : public IDatabaseVariableSet
{
	public:

		CFunctionInputProcedureSet( void ) :
			Result(),
			Input()
		{}

		CFunctionInputProcedureSet( const CFunctionInputProcedureSet &rhs ) :
			Result( rhs.Result ),
			Input( rhs.Input )
		{}

		virtual ~CFunctionInputProcedureSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &Result );
			variables.push_back( &Input );
		}

		DBUInt64Out Result;
		DBUInt64In Input;
};

template< uint32 ISIZE, uint32 OSIZE >
class CFunctionInputProcedureCall : public TDatabaseFunctionCall< CFunctionInputProcedureSet, ISIZE, CEmptyVariableSet, OSIZE >
{
	public:

		typedef TDatabaseFunctionCall< CFunctionInputProcedureSet, ISIZE, CEmptyVariableSet, OSIZE > BASECLASS;

		CFunctionInputProcedureCall( void ) : 
			BASECLASS(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CFunctionInputProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.function_input_procedure"; }

		void Verify_Results( uint32 expected_rollback_count ) 
		{
			ASSERT_TRUE( InitializeCalls == Rollbacks + 1 );
			ASSERT_TRUE( FinishedCalls == 0 );
			ASSERT_TRUE( Rollbacks == expected_rollback_count );
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) { InitializeCalls++; }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) {}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) { FinishedCalls++;}	

		virtual void On_Rollback( void ) { Rollbacks++; }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_FunctionInputProcedureCall_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CFunctionInputProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CFunctionInputProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CFunctionInputProcedureCall< ISIZE, OSIZE > *db_task = new CFunctionInputProcedureCall< ISIZE, OSIZE >;
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == task_count );
	ASSERT_TRUE( successful_tasks.size() == 0 );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		uint32 expected_rollback_count = i % ISIZE;	// this is probably unreliable across ODBC impl; it assumes the task list gets repeatedly rolled back in sequential order
		tasks[ i ]->Verify_Results( expected_rollback_count );
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, FunctionInputProcedureCall_1_1_1 )
{
	Run_FunctionInputProcedureCall_Test< 1, 1 >( 1 );
}

TEST_F( ODBCFailureTests, FunctionInputProcedureCall_1_1_2 )
{
	Run_FunctionInputProcedureCall_Test< 1, 1 >( 2 );
}

TEST_F( ODBCFailureTests, FunctionInputProcedureCall_2_1_2 )
{
	Run_FunctionInputProcedureCall_Test< 2, 1 >( 2 );
}

TEST_F( ODBCFailureTests, FunctionInputProcedureCall_2_1_5 )
{
	Run_FunctionInputProcedureCall_Test< 2, 1 >( 5 );
}

TEST_F( ODBCFailureTests, FunctionInputProcedureCall_3_1_3 )
{
	Run_FunctionInputProcedureCall_Test< 3, 1 >( 3 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CProcedureInputFunctionSet : public IDatabaseVariableSet
{
	public:

		CProcedureInputFunctionSet( void ) :
			Input1(),
			Input2()
		{}

		CProcedureInputFunctionSet( const CProcedureInputFunctionSet &rhs ) :
			Input1( rhs.Input1 ),
			Input2( rhs.Input2 )
		{}

		virtual ~CProcedureInputFunctionSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &Input1 );
			variables.push_back( &Input2 );
		}

		DBUInt64In Input1;
		DBUInt64In Input2;
};

template< uint32 ISIZE, uint32 OSIZE >
class CProcedureInputFunctionCall : public TDatabaseProcedureCall< CProcedureInputFunctionSet, ISIZE, CEmptyVariableSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CProcedureInputFunctionSet, ISIZE, CEmptyVariableSet, OSIZE > BASECLASS;

		CProcedureInputFunctionCall( void ) : 
			BASECLASS(),
			FinishedCalls( 0 ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CProcedureInputFunctionCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.procedure_input_function"; }

		void Verify_Results( uint32 expected_rollback_count ) 
		{
			ASSERT_TRUE( InitializeCalls == Rollbacks + 1 );
			ASSERT_TRUE( FinishedCalls == 0 );
			ASSERT_TRUE( Rollbacks == expected_rollback_count );
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet * /*input_parameters*/ ) { InitializeCalls++; }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) {}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) { FinishedCalls++;}	

		virtual void On_Rollback( void ) { Rollbacks++; }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_ProcedureInputFunctionCall_Test( uint32 task_count )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CProcedureInputFunctionCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CProcedureInputFunctionCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CProcedureInputFunctionCall< ISIZE, OSIZE > *db_task = new CProcedureInputFunctionCall< ISIZE, OSIZE >;
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	ASSERT_TRUE( failed_tasks.size() == task_count );
	ASSERT_TRUE( successful_tasks.size() == 0 );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		uint32 expected_rollback_count = i % ISIZE;	// this is probably unreliable across ODBC impl; it assumes the task list gets repeatedly rolled back in sequential order
		tasks[ i ]->Verify_Results( expected_rollback_count );
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, ProcedureInputFunctionCall_1_1_1 )
{
	Run_ProcedureInputFunctionCall_Test< 1, 1 >( 1 );
}

TEST_F( ODBCFailureTests, ProcedureInputFunctionCall_1_1_2 )
{
	Run_ProcedureInputFunctionCall_Test< 1, 1 >( 2 );
}

TEST_F( ODBCFailureTests, ProcedureInputFunctionCall_2_1_2 )
{
	Run_ProcedureInputFunctionCall_Test< 2, 1 >( 2 );
}

TEST_F( ODBCFailureTests, ProcedureInputFunctionCall_2_1_5 )
{
	Run_ProcedureInputFunctionCall_Test< 2, 1 >( 5 );
}

TEST_F( ODBCFailureTests, ProcedureInputFunctionCall_3_1_3 )
{
	Run_ProcedureInputFunctionCall_Test< 3, 1 >( 3 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CExceptionInputParams : public IDatabaseVariableSet
{
	public:

		CExceptionInputParams( void ) :
			Throw(),
			AccountCount()
		{}

		CExceptionInputParams( bool throw_exception ) :
			Throw( throw_exception ),
			AccountCount()
		{}

		CExceptionInputParams( const CExceptionInputParams &rhs ) :
			Throw( rhs.Throw ),
			AccountCount( rhs.AccountCount )
		{}

		virtual ~CExceptionInputParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &Throw );
			variables.push_back( &AccountCount );
		}

		DBBoolIn Throw;
		DBUInt64InOut AccountCount;
};

class CExceptionOutputParams : public IDatabaseVariableSet
{
	public:

		CExceptionOutputParams( void ) :
			ID()
		{}

		CExceptionOutputParams( const CExceptionOutputParams &rhs ) :
			ID( rhs.ID )
		{}

		virtual ~CExceptionOutputParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID );
		}

		DBUInt64In ID;
};

template< uint32 ISIZE, uint32 OSIZE >
class CThrowExceptionProcedureCall : public TDatabaseProcedureCall< CExceptionInputParams, ISIZE, CExceptionOutputParams, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CExceptionInputParams, ISIZE, CExceptionOutputParams, OSIZE > BASECLASS;

		CThrowExceptionProcedureCall( const wchar_t *proc_name, bool throw_exception ) : 
			BASECLASS(),
			ProcName( proc_name ),
			ThrowException( throw_exception ),
			Results(),
			AccountCount( 0 ),
			FinishedCalls( 0 ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CThrowExceptionProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return ProcName; }

		void Verify_Results( uint32 self_index, uint32 exception_index1, uint32 exception_index2 ) 
		{
			FATAL_ASSERT( exception_index1 < exception_index2 );

			uint32 expected_rollbacks = 0;
			uint32 expected_inits = 1;
			uint32 my_batch = self_index / ISIZE;
			uint32 my_batch_index = self_index % ISIZE;

			if ( my_batch == exception_index1 / ISIZE )
			{
				if ( self_index != exception_index1 )
				{
					expected_rollbacks++;
				}

				if ( my_batch_index > exception_index1 % ISIZE )
				{
					expected_inits++;
				}
			}

			if ( my_batch == exception_index2 / ISIZE )
			{
				if ( self_index != exception_index2 && self_index != exception_index1 )
				{
					expected_rollbacks++;
				}

				if ( my_batch_index > exception_index2 % ISIZE )
				{
					expected_inits++;
				}
			}

			ASSERT_TRUE( InitializeCalls == expected_inits );
			ASSERT_TRUE( Rollbacks == expected_rollbacks );

			if ( self_index == exception_index1 || self_index == exception_index2 )
			{
				ASSERT_TRUE( FinishedCalls == 0 );
			}
			else
			{
				ASSERT_TRUE( FinishedCalls > 0 );	// unable to constrain this any further
				ASSERT_TRUE( Results.size() == 3 );
				ASSERT_TRUE( AccountCount == 3 );
				for ( uint32 i = 0; i < Results.size(); ++i )
				{
					ASSERT_TRUE( Results[ i ].ID.Get_Value() == i + 1 );
				}
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) {
			CExceptionInputParams *input_params = static_cast< CExceptionInputParams * >( input_parameters );
			*input_params = CExceptionInputParams( ThrowException );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CExceptionOutputParams *result_rows = static_cast< CExceptionOutputParams * >( result_set );
			for ( int64 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( result_rows[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CExceptionInputParams *input_params = static_cast< CExceptionInputParams * >( input_parameters );
			AccountCount = input_params->AccountCount.Get_Value();

			FinishedCalls++;
		}	

		virtual void On_Rollback( void ) { 
			Results.clear();
			AccountCount = 0;

			Rollbacks++; 
		}

		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		const wchar_t *ProcName;
		bool ThrowException;

		std::vector< CExceptionOutputParams > Results;
		uint64 AccountCount;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_ThrowExceptionProcedureCall_Test( const wchar_t *proc_name, uint32 task_count, uint32 exception_index1, uint32 exception_index2 )
{
	FATAL_ASSERT( exception_index1 < task_count && exception_index1 < exception_index2 );

	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CThrowExceptionProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CThrowExceptionProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CThrowExceptionProcedureCall< ISIZE, OSIZE > *db_task = new CThrowExceptionProcedureCall< ISIZE, OSIZE >( proc_name, i == exception_index1 || i == exception_index2 );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	uint32 exception_count = ( exception_index2 < task_count ) ? 2 : 1;

	ASSERT_TRUE( failed_tasks.size() == exception_count );
	ASSERT_TRUE( successful_tasks.size() == task_count - exception_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results( i, exception_index1, exception_index2 );
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_1_1_1_0_NULL )
{
	Run_ThrowExceptionProcedureCall_Test< 1, 1 >( L"dynamic.exception_thrower", 1, 0, 666 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_2_2_2_0_NULL )
{
	Run_ThrowExceptionProcedureCall_Test< 2, 2 >( L"dynamic.exception_thrower", 2, 0, 666 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_2_2_2_1_NULL )
{
	Run_ThrowExceptionProcedureCall_Test< 2, 2 >( L"dynamic.exception_thrower", 2, 1, 666 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_2_2_5_1_NULL )
{
	Run_ThrowExceptionProcedureCall_Test< 2, 2 >( L"dynamic.exception_thrower", 5, 1, 666 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_3_0_NULL )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 3, 0, 666 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_3_1_NULL )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 3, 1, 666 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_3_2_NULL )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 3, 2, 666 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_7_2_NULL )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 7, 2, 666 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_2_2_2_0_1 )
{
	Run_ThrowExceptionProcedureCall_Test< 2, 2 >( L"dynamic.exception_thrower", 2, 0, 1 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_3_0_1 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 3, 0, 1 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_3_0_2 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 3, 0, 2 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_3_1_2 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 3, 0, 2 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_6_0_1 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 6, 0, 1 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_6_1_2 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 6, 1, 2 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_6_0_2 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 6, 0, 2 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_6_0_3 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 6, 0, 3 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_6_1_4 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 6, 1, 4 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_6_2_5 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 6, 2, 5 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_6_3_4 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 6, 3, 4 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_6_3_5 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 6, 3, 5 );
}

TEST_F( ODBCFailureTests, UserExceptionProcedureCall_3_2_6_4_5 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.exception_thrower", 6, 4, 5 );
}

//////

TEST_F( ODBCFailureTests, UniquenessViolationProcedureCall_3_2_6_0_1 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.unique_constraint_violator", 6, 0, 1 );
}

TEST_F( ODBCFailureTests, UniquenessViolationProcedureCall_3_2_6_1_2 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.unique_constraint_violator", 6, 1, 2 );
}

TEST_F( ODBCFailureTests, UniquenessViolationProcedureCall_3_2_6_0_2 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.unique_constraint_violator", 6, 0, 2 );
}

TEST_F( ODBCFailureTests, UniquenessViolationProcedureCall_3_2_6_0_3 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.unique_constraint_violator", 6, 0, 3 );
}

TEST_F( ODBCFailureTests, UniquenessViolationProcedureCall_3_2_6_1_4 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.unique_constraint_violator", 6, 1, 4 );
}

TEST_F( ODBCFailureTests, UniquenessViolationProcedureCall_3_2_6_2_5 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.unique_constraint_violator", 6, 2, 5 );
}

TEST_F( ODBCFailureTests, UniquenessViolationProcedureCall_3_2_6_3_4 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.unique_constraint_violator", 6, 3, 4 );
}

TEST_F( ODBCFailureTests, UniquenessViolationProcedureCall_3_2_6_3_5 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.unique_constraint_violator", 6, 3, 5 );
}

TEST_F( ODBCFailureTests, UniquenessViolationProcedureCall_3_2_6_4_5 )
{
	Run_ThrowExceptionProcedureCall_Test< 3, 2 >( L"dynamic.unique_constraint_violator", 6, 4, 5 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMissingSelectParams : public IDatabaseVariableSet
{
	public:

		CMissingSelectParams( void ) :
			SkipSelect(),
			AccountCount()
		{}

		CMissingSelectParams( bool skip_select ) :
			SkipSelect( skip_select ),
			AccountCount()
		{}

		CMissingSelectParams( const CMissingSelectParams &rhs ) :
			SkipSelect( rhs.SkipSelect ),
			AccountCount( rhs.AccountCount )
		{}

		virtual ~CMissingSelectParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &SkipSelect );
			variables.push_back( &AccountCount );
		}

		DBBoolIn SkipSelect;
		DBUInt64InOut AccountCount;
};

class CMissingSelectResultSet : public IDatabaseVariableSet
{
	public:

		CMissingSelectResultSet( void ) :
			ID()
		{}

		CMissingSelectResultSet( const CMissingSelectResultSet &rhs ) :
			ID( rhs.ID )
		{}

		virtual ~CMissingSelectResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID );
		}

		DBUInt64In ID;
};

template< uint32 ISIZE, uint32 OSIZE >
class CMissingSelectProcedureCall : public TDatabaseProcedureCall< CMissingSelectParams, ISIZE, CMissingSelectResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CMissingSelectParams, ISIZE, CMissingSelectResultSet, OSIZE > BASECLASS;

		CMissingSelectProcedureCall( bool skip_select ) : 
			BASECLASS(),
			SkipSelect( skip_select ),
			Results(),
			AccountCount( 0 ),
			FinishedCalls( 0 ),
			Success( false ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CMissingSelectProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.skip_select_procedure"; }

		void Verify_Results( uint32 self_index, uint32 exception_index1, uint32 exception_index2 ) 
		{
			if ( self_index == exception_index1 || self_index == exception_index2 )
			{
				ASSERT_TRUE( !Success );
			}
			else
			{
				ASSERT_TRUE( Success );
				ASSERT_TRUE( FinishedCalls > 0 );	// unable to constrain this any further
				ASSERT_TRUE( Results.size() == 3 );
				ASSERT_TRUE( AccountCount == 3 );
				for ( uint32 i = 0; i < Results.size(); ++i )
				{
					ASSERT_TRUE( Results[ i ].ID.Get_Value() == i + 1 );
				}
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) {
			CMissingSelectParams *input_params = static_cast< CMissingSelectParams * >( input_parameters );
			*input_params = CMissingSelectParams( SkipSelect );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CMissingSelectResultSet *result_rows = static_cast< CMissingSelectResultSet * >( result_set );
			for ( int64 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( result_rows[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CMissingSelectParams *input_params = static_cast< CMissingSelectParams * >( input_parameters );
			AccountCount = input_params->AccountCount.Get_Value();

			FinishedCalls++;
			Success = true;
		}	

		virtual void On_Rollback( void ) { 
			Results.clear();
			AccountCount = 0;

			Rollbacks++; 
			Success = false;
		}

		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		bool SkipSelect;

		std::vector< CMissingSelectResultSet > Results;
		uint64 AccountCount;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
		bool Success;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_MissingSelectProcedureCall_Test( uint32 task_count, uint32 skip_index1, uint32 skip_index2 )
{
	FATAL_ASSERT( skip_index1 < task_count && skip_index1 < skip_index2 );

	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CMissingSelectProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CMissingSelectProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CMissingSelectProcedureCall< ISIZE, OSIZE > *db_task = new CMissingSelectProcedureCall< ISIZE, OSIZE >( i == skip_index1 || i == skip_index2 );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	uint32 exception_count = ( skip_index2 < task_count ) ? 2 : 1;

	ASSERT_TRUE( failed_tasks.size() == exception_count );
	ASSERT_TRUE( successful_tasks.size() == task_count - exception_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results( i, skip_index1, skip_index2 );
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_1_1_1_0_NULL )
{
	Run_MissingSelectProcedureCall_Test< 1, 1 >( 1, 0, 666 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_2_1_2_0_NULL )
{
	Run_MissingSelectProcedureCall_Test< 2, 1 >( 2, 0, 666 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_2_1_2_1_NULL )
{
	Run_MissingSelectProcedureCall_Test< 2, 1 >( 2, 1, 666 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_3_0_NULL )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 3, 0, 666 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_3_1_NULL )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 3, 1, 666 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_3_2_NULL )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 3, 2, 666 );
}

///

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_6_0_1 )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 6, 0, 1 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_6_1_2 )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 6, 1, 2 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_6_0_2 )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 6, 0, 2 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_6_0_3 )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 6, 0, 3 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_6_1_3 )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 6, 1, 3 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_6_2_4 )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 6, 2, 4 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_6_3_4 )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 6, 3, 4 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_6_3_5 )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 6, 3, 5 );
}

TEST_F( ODBCFailureTests, MissingSelectProcedureCall_3_1_6_4_5 )
{
	Run_MissingSelectProcedureCall_Test< 3, 1 >( 6, 4, 5 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CExtraSelectParams : public IDatabaseVariableSet
{
	public:

		CExtraSelectParams( void ) :
			ExtraSelect(),
			AccountCount()
		{}

		CExtraSelectParams( bool extra_select ) :
			ExtraSelect( extra_select ),
			AccountCount()
		{}

		CExtraSelectParams( const CExtraSelectParams &rhs ) :
			ExtraSelect( rhs.ExtraSelect ),
			AccountCount( rhs.AccountCount )
		{}

		virtual ~CExtraSelectParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ExtraSelect );
			variables.push_back( &AccountCount );
		}

		DBBoolIn ExtraSelect;
		DBUInt64InOut AccountCount;
};

template< uint32 ISIZE, uint32 OSIZE >
class CExtraSelectProcedureCall : public TDatabaseProcedureCall< CExtraSelectParams, ISIZE, CEmptyVariableSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CExtraSelectParams, ISIZE, CEmptyVariableSet, OSIZE > BASECLASS;

		CExtraSelectProcedureCall( bool extra_select ) : 
			BASECLASS(),
			ExtraSelect( extra_select ),
			AccountCount( 0 ),
			FinishedCalls( 0 ),
			Success( false ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CExtraSelectProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.extra_select_procedure"; }

		void Verify_Results( void ) 
		{
			if ( ExtraSelect )
			{
				ASSERT_FALSE( Success );
			}
			else
			{
				ASSERT_TRUE( Success );
				ASSERT_TRUE( FinishedCalls > 0 );	// unable to constrain this any further
				ASSERT_TRUE( AccountCount == 3 );
			}
		}

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) {
			CExtraSelectParams *input_params = static_cast< CExtraSelectParams * >( input_parameters );
			*input_params = CExtraSelectParams( ExtraSelect );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet * /*result_set*/, int64 /*rows_fetched*/ ) 
		{
			ASSERT_TRUE( false );
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet *input_parameters ) 
		{ 
			CExtraSelectParams *input_params = static_cast< CExtraSelectParams * >( input_parameters );
			AccountCount = input_params->AccountCount.Get_Value();

			FinishedCalls++;
			Success = true;
		}	

		virtual void On_Rollback( void ) { 
			AccountCount = 0;

			Rollbacks++; 
			Success = false;
		}

		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		bool ExtraSelect;

		uint64 AccountCount;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
		bool Success;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_ExtraSelectProcedureCall_Test( uint32 task_count, uint32 non_extra_index )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CExtraSelectProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CExtraSelectProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CExtraSelectProcedureCall< ISIZE, OSIZE > *db_task = new CExtraSelectProcedureCall< ISIZE, OSIZE >( i != non_extra_index );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	uint32 bad_task_count = ( non_extra_index < task_count ) ? task_count - 1 : task_count;

	ASSERT_TRUE( failed_tasks.size() == bad_task_count );
	ASSERT_TRUE( successful_tasks.size() == task_count - bad_task_count );
	
	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}


TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_1_1_1_1 )
{
	Run_ExtraSelectProcedureCall_Test< 1, 1 >( 1, 1 );
}

TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_1_1_1_0 )
{
	Run_ExtraSelectProcedureCall_Test< 1, 1 >( 1, 0 );
}

TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_2_1_2_0 )
{
	Run_ExtraSelectProcedureCall_Test< 2, 1 >( 2, 0 );
}

TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_2_1_2_1 )
{
	Run_ExtraSelectProcedureCall_Test< 2, 1 >( 2, 1 );
}

TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_2_1_2_2 )
{
	Run_ExtraSelectProcedureCall_Test< 2, 1 >( 2, 2 );
}

TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_3_1_3_0 )
{
	Run_ExtraSelectProcedureCall_Test< 3, 1 >( 3, 0 );
}

TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_3_1_3_1 )
{
	Run_ExtraSelectProcedureCall_Test< 3, 1 >( 3, 1 );
}

TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_3_1_3_2 )
{
	Run_ExtraSelectProcedureCall_Test< 3, 1 >( 3, 2 );
}

TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_3_1_6_1 )
{
	Run_ExtraSelectProcedureCall_Test< 3, 1 >( 6, 1 );
}

TEST_F( ODBCFailureTests, ExtraSelectProcedureCall_3_1_6_4 )
{
	Run_ExtraSelectProcedureCall_Test< 3, 1 >( 6, 4 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CTooManyResultsParams : public IDatabaseVariableSet
{
	public:

		CTooManyResultsParams( void ) :
			ExtraSelect()
		{}

		CTooManyResultsParams( bool extra_select ) :
			ExtraSelect( extra_select )
		{}

		CTooManyResultsParams( const CTooManyResultsParams &rhs ) :
			ExtraSelect( rhs.ExtraSelect )
		{}

		virtual ~CTooManyResultsParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ExtraSelect );
		}

		DBBoolIn ExtraSelect;
};

class CTooManyResultsResultSet : public IDatabaseVariableSet
{
	public:

		CTooManyResultsResultSet( void ) :
			ID()
		{}

		CTooManyResultsResultSet( const CTooManyResultsResultSet &rhs ) :
			ID( rhs.ID )
		{}

		virtual ~CTooManyResultsResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID );
		}

		DBUInt64In ID;
};

template< uint32 ISIZE, uint32 OSIZE >
class CTooManyResultsProcedureCall : public TDatabaseProcedureCall< CTooManyResultsParams, ISIZE, CTooManyResultsResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CTooManyResultsParams, ISIZE, CTooManyResultsResultSet, OSIZE > BASECLASS;

		CTooManyResultsProcedureCall( bool extra_select ) : 
			BASECLASS(),
			ExtraSelect( extra_select ),
			Results(),
			FinishedCalls( 0 ),
			Success( false ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CTooManyResultsProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.too_many_results_procedure"; }

		void Verify_Results( void ) 
		{
			if ( !ExtraSelect )
			{
				ASSERT_TRUE( Success );
				ASSERT_TRUE( FinishedCalls > 0 );	// unable to constrain this any further
				ASSERT_TRUE( Results.size() == 3 );
				for ( uint32 i = 0; i < Results.size(); ++i )
				{
					ASSERT_TRUE( Results[ i ].ID.Get_Value() == i + 1 );
				}
			}
		}

		bool Has_Extra_Select( void ) const { return ExtraSelect; }

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) {
			CTooManyResultsParams *input_params = static_cast< CTooManyResultsParams * >( input_parameters );
			*input_params = CTooManyResultsParams( ExtraSelect );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CTooManyResultsResultSet *results = static_cast< CTooManyResultsResultSet * >( result_set );

			for ( int64 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) 
		{ 
			FinishedCalls++;
			Success = true;
		}	

		virtual void On_Rollback( void ) { 
			Results.clear();
			Rollbacks++; 
			Success = false;
		}

		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		bool ExtraSelect;

		std::vector< CTooManyResultsResultSet > Results;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
		bool Success;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_TooManyResultsProcedureCall_Test( uint32 task_count, uint32 extra_index )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CTooManyResultsProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CTooManyResultsProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CTooManyResultsProcedureCall< ISIZE, OSIZE > *db_task = new CTooManyResultsProcedureCall< ISIZE, OSIZE >( i == extra_index );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	uint32 good_task_count = ( extra_index < task_count ) ? task_count - 1 : task_count;

	ASSERT_TRUE( failed_tasks.size() == task_count - good_task_count );
	ASSERT_TRUE( successful_tasks.size() == good_task_count );
	
	for ( auto iter = failed_tasks.cbegin(); iter != failed_tasks.cend(); ++iter )
	{
		CTooManyResultsProcedureCall< ISIZE, OSIZE > *task = static_cast< CTooManyResultsProcedureCall< ISIZE, OSIZE > * >( *iter );
		ASSERT_TRUE( task->Has_Extra_Select() );
	}

	for ( auto iter = successful_tasks.cbegin(); iter != successful_tasks.cend(); ++iter )
	{
		CTooManyResultsProcedureCall< ISIZE, OSIZE > *task = static_cast< CTooManyResultsProcedureCall< ISIZE, OSIZE > * >( *iter );
		ASSERT_TRUE( !task->Has_Extra_Select() );
	}

	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_1_1_1_1 )
{
	Run_TooManyResultsProcedureCall_Test< 1, 1 >( 1, 1 );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_1_1_1_0 )
{
	Run_TooManyResultsProcedureCall_Test< 1, 1 >( 1, 0 );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_2_1_2_0 )
{
	Run_TooManyResultsProcedureCall_Test< 2, 1 >( 2, 0 );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_2_1_2_1 )
{
	Run_TooManyResultsProcedureCall_Test< 2, 1 >( 2, 1 );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_2_1_2_2 )
{
	Run_TooManyResultsProcedureCall_Test< 2, 1 >( 2, 2 );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_3_1_3_0 )
{
	Run_TooManyResultsProcedureCall_Test< 3, 1 >( 3, 0 );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_3_1_3_1 )
{
	Run_TooManyResultsProcedureCall_Test< 3, 1 >( 3, 1 );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_3_1_3_2 )
{
	Run_TooManyResultsProcedureCall_Test< 3, 1 >( 3, 2 );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_3_1_6_1 )
{
	Run_TooManyResultsProcedureCall_Test< 3, 1 >( 6, 1 );
}

TEST_F( ODBCFailureTests, TooManyResultsProcedureCall_3_1_6_4 )
{
	Run_TooManyResultsProcedureCall_Test< 3, 1 >( 6, 4 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CExtraColumnParams : public IDatabaseVariableSet
{
	public:

		CExtraColumnParams( void ) :
			ExtraColumn()
		{}

		CExtraColumnParams( bool extra_column ) :
			ExtraColumn( extra_column )
		{}

		CExtraColumnParams( const CExtraColumnParams &rhs ) :
			ExtraColumn( rhs.ExtraColumn )
		{}

		virtual ~CExtraColumnParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ExtraColumn );
		}

		DBBoolIn ExtraColumn;
};

class CExtraColumnResultSet : public IDatabaseVariableSet
{
	public:

		CExtraColumnResultSet( void ) :
			ID()
		{}

		CExtraColumnResultSet( const CExtraColumnResultSet &rhs ) :
			ID( rhs.ID )
		{}

		virtual ~CExtraColumnResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID );
		}

		DBUInt64In ID;
};

template< uint32 ISIZE, uint32 OSIZE >
class CExtraColumnProcedureCall : public TDatabaseProcedureCall< CExtraColumnParams, ISIZE, CExtraColumnResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CExtraColumnParams, ISIZE, CExtraColumnResultSet, OSIZE > BASECLASS;

		CExtraColumnProcedureCall( bool extra_column ) : 
			BASECLASS(),
			ExtraColumn( extra_column ),
			Results(),
			FinishedCalls( 0 ),
			Success( false ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CExtraColumnProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.extra_column_procedure"; }

		void Verify_Results( void ) 
		{
			if ( !ExtraColumn )
			{
				ASSERT_TRUE( Success );
				ASSERT_TRUE( FinishedCalls > 0 );	// unable to constrain this any further
				ASSERT_TRUE( Results.size() == 3 );
				for ( uint32 i = 0; i < Results.size(); ++i )
				{
					ASSERT_TRUE( Results[ i ].ID.Get_Value() == i + 1 );
				}
			}
		}

		bool Has_Extra_Column( void ) const { return ExtraColumn; }

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) {
			CExtraColumnParams *input_params = static_cast< CExtraColumnParams * >( input_parameters );
			*input_params = CExtraColumnParams( ExtraColumn );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CExtraColumnResultSet *results = static_cast< CExtraColumnResultSet * >( result_set );

			for ( int64 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) 
		{ 
			FinishedCalls++;
			Success = true;
		}	

		virtual void On_Rollback( void ) { 
			Results.clear();
			Rollbacks++; 
			Success = false;
		}

		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		bool ExtraColumn;

		std::vector< CExtraColumnResultSet > Results;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
		bool Success;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_ExtraColumnProcedureCall_Test( uint32 task_count, uint32 extra_index )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CExtraColumnProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CExtraColumnProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CExtraColumnProcedureCall< ISIZE, OSIZE > *db_task = new CExtraColumnProcedureCall< ISIZE, OSIZE >( i == extra_index );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	uint32 good_task_count = ( extra_index < task_count ) ? task_count - 1 : task_count;

	ASSERT_TRUE( failed_tasks.size() == task_count - good_task_count );
	ASSERT_TRUE( successful_tasks.size() == good_task_count );
	
	for ( auto iter = failed_tasks.cbegin(); iter != failed_tasks.cend(); ++iter )
	{
		CExtraColumnProcedureCall< ISIZE, OSIZE > *task = static_cast< CExtraColumnProcedureCall< ISIZE, OSIZE > * >( *iter );
		ASSERT_TRUE( task->Has_Extra_Column() );
	}

	for ( auto iter = successful_tasks.cbegin(); iter != successful_tasks.cend(); ++iter )
	{
		CExtraColumnProcedureCall< ISIZE, OSIZE > *task = static_cast< CExtraColumnProcedureCall< ISIZE, OSIZE > * >( *iter );
		ASSERT_TRUE( !task->Has_Extra_Column() );
	}

	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, ExtraColumnProcedureCall_1_1_1_1 )
{
	Run_ExtraColumnProcedureCall_Test< 1, 1 >( 1, 1 );
}

TEST_F( ODBCFailureTests, ExtraColumnProcedureCall_1_1_1_0 )
{
	Run_ExtraColumnProcedureCall_Test< 1, 1 >( 1, 0 );
}

TEST_F( ODBCFailureTests, ExtraColumnProcedureCall_2_1_2_0 )
{
	Run_ExtraColumnProcedureCall_Test< 2, 1 >( 2, 0 );
}

TEST_F( ODBCFailureTests, ExtraColumnProcedureCall_2_1_2_1 )
{
	Run_ExtraColumnProcedureCall_Test< 2, 1 >( 2, 1 );
}

TEST_F( ODBCFailureTests, ExtraColumnProcedureCall_3_2_7_0 )
{
	Run_ExtraColumnProcedureCall_Test< 3, 2 >( 7, 0 );
}

TEST_F( ODBCFailureTests, ExtraColumnProcedureCall_3_2_7_2 )
{
	Run_ExtraColumnProcedureCall_Test< 3, 2 >( 7, 2 );
}

TEST_F( ODBCFailureTests, ExtraColumnProcedureCall_3_2_7_4 )
{
	Run_ExtraColumnProcedureCall_Test< 3, 2 >( 7, 4 );
}

TEST_F( ODBCFailureTests, ExtraColumnProcedureCall_3_2_7_6 )
{
	Run_ExtraColumnProcedureCall_Test< 3, 2 >( 7, 6 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CMissingColumnParams : public IDatabaseVariableSet
{
	public:

		CMissingColumnParams( void ) :
			MissingColumn()
		{}

		CMissingColumnParams( bool missing_column ) :
			MissingColumn( missing_column )
		{}

		CMissingColumnParams( const CMissingColumnParams &rhs ) :
			MissingColumn( rhs.MissingColumn )
		{}

		virtual ~CMissingColumnParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &MissingColumn );
		}

		DBBoolIn MissingColumn;
};

class CMissingColumnResultSet : public IDatabaseVariableSet
{
	public:

		CMissingColumnResultSet( void ) :
			ID1(),
			ID2()
		{}

		CMissingColumnResultSet( const CMissingColumnResultSet &rhs ) :
			ID1( rhs.ID1 ),
			ID2( rhs.ID2 )
		{}

		virtual ~CMissingColumnResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID1 );
			variables.push_back( &ID2 );
		}

		DBUInt64In ID1;
		DBUInt64In ID2;
};

template< uint32 ISIZE, uint32 OSIZE >
class CMissingColumnProcedureCall : public TDatabaseProcedureCall< CMissingColumnParams, ISIZE, CMissingColumnResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CMissingColumnParams, ISIZE, CMissingColumnResultSet, OSIZE > BASECLASS;

		CMissingColumnProcedureCall( bool missing_column ) : 
			BASECLASS(),
			MissingColumn( missing_column ),
			Results(),
			FinishedCalls( 0 ),
			Success( false ),
			InitializeCalls( 0 ),
			Rollbacks( 0 )
		{}

		virtual ~CMissingColumnProcedureCall() {}

		virtual const wchar_t *Get_Procedure_Name( void ) const { return L"dynamic.missing_column_procedure"; }

		void Verify_Results( void ) 
		{
			if ( !MissingColumn )
			{
				ASSERT_TRUE( Success );
				ASSERT_TRUE( FinishedCalls > 0 );	// unable to constrain this any further
				ASSERT_TRUE( Results.size() == 3 );
				for ( uint32 i = 0; i < Results.size(); ++i )
				{
					ASSERT_TRUE( Results[ i ].ID1.Get_Value() == i + 1 );
					ASSERT_TRUE( Results[ i ].ID2.Get_Value() == 1 );
				}
			}
		}

		bool Has_Missing_Column( void ) const { return MissingColumn; }

	protected:

		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) {
			CMissingColumnParams *input_params = static_cast< CMissingColumnParams * >( input_parameters );
			*input_params = CMissingColumnParams( MissingColumn );

			InitializeCalls++; 
		}	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			CMissingColumnResultSet *results = static_cast< CMissingColumnResultSet * >( result_set );

			for ( int64 i = 0; i < rows_fetched; ++i )
			{
				Results.push_back( results[ i ] );
			}
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) 
		{ 
			FinishedCalls++;
			Success = true;
		}	

		virtual void On_Rollback( void ) { 
			Results.clear();
			Rollbacks++; 
			Success = false;
		}

		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		bool MissingColumn;

		std::vector< CMissingColumnResultSet > Results;

		uint32 FinishedCalls;
		uint32 InitializeCalls;
		uint32 Rollbacks;
		bool Success;
};

template< uint32 ISIZE, uint32 OSIZE >
void Run_MissingColumnProcedureCall_Test( uint32 task_count, uint32 missing_index )
{
	IDatabaseConnection *connection = CODBCFactory::Get_Environment()->Add_Connection( L"Driver={SQL Server Native Client 11.0};Server=AZAZELPC\\CCGONLINE;Database=testdb;UID=testserver;PWD=TEST5erver#;", false );
	ASSERT_TRUE( connection != nullptr );

	TDatabaseTaskBatch< CMissingColumnProcedureCall< ISIZE, OSIZE > > db_task_batch;
	std::vector< CMissingColumnProcedureCall< ISIZE, OSIZE > * > tasks;
	for ( uint32 i = 0; i < task_count; ++i )
	{
		CMissingColumnProcedureCall< ISIZE, OSIZE > *db_task = new CMissingColumnProcedureCall< ISIZE, OSIZE >( i == missing_index );
		tasks.push_back( db_task );
		db_task_batch.Add_Task( db_task );
	}

	DBTaskListType successful_tasks;
	DBTaskListType failed_tasks;
	db_task_batch.Execute_Tasks( connection, successful_tasks, failed_tasks );

	uint32 good_task_count = ( missing_index < task_count ) ? task_count - 1 : task_count;

	ASSERT_TRUE( failed_tasks.size() == task_count - good_task_count );
	ASSERT_TRUE( successful_tasks.size() == good_task_count );
	
	for ( auto iter = failed_tasks.cbegin(); iter != failed_tasks.cend(); ++iter )
	{
		CMissingColumnProcedureCall< ISIZE, OSIZE > *task = static_cast< CMissingColumnProcedureCall< ISIZE, OSIZE > * >( *iter );
		ASSERT_TRUE( task->Has_Missing_Column() );
	}

	for ( auto iter = successful_tasks.cbegin(); iter != successful_tasks.cend(); ++iter )
	{
		CMissingColumnProcedureCall< ISIZE, OSIZE > *task = static_cast< CMissingColumnProcedureCall< ISIZE, OSIZE > * >( *iter );
		ASSERT_TRUE( !task->Has_Missing_Column() );
	}

	for ( uint32 i = 0; i < tasks.size(); ++i )
	{
		tasks[ i ]->Verify_Results();
		delete tasks[ i ];
	}

	CODBCFactory::Get_Environment()->Shutdown_Connection( connection->Get_ID() );
}

TEST_F( ODBCFailureTests, MissingColumnProcedureCall_1_1_1_1 )
{
	Run_MissingColumnProcedureCall_Test< 1, 1 >( 1, 1 );
}

TEST_F( ODBCFailureTests, MissingColumnProcedureCall_1_1_1_0 )
{
	Run_MissingColumnProcedureCall_Test< 1, 1 >( 1, 0 );
}

TEST_F( ODBCFailureTests, MissingColumnProcedureCall_2_1_2_0 )
{
	Run_MissingColumnProcedureCall_Test< 2, 1 >( 2, 0 );
}

TEST_F( ODBCFailureTests, MissingColumnProcedureCall_2_1_2_1 )
{
	Run_MissingColumnProcedureCall_Test< 2, 1 >( 2, 1 );
}

TEST_F( ODBCFailureTests, MissingColumnProcedureCall_3_2_7_0 )
{
	Run_MissingColumnProcedureCall_Test< 3, 2 >( 7, 0 );
}

TEST_F( ODBCFailureTests, MissingColumnProcedureCall_3_2_7_2 )
{
	Run_MissingColumnProcedureCall_Test< 3, 2 >( 7, 2 );
}

TEST_F( ODBCFailureTests, MissingColumnProcedureCall_3_2_7_4 )
{
	Run_MissingColumnProcedureCall_Test< 3, 2 >( 7, 4 );
}

TEST_F( ODBCFailureTests, MissingColumnProcedureCall_3_2_7_6 )
{
	Run_MissingColumnProcedureCall_Test< 3, 2 >( 7, 6 );
}