/**********************************************************************************************************************

	ODBCShared.h
		

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

#ifndef ODBC_SHARED_H
#define ODBC_SHARED_H

#include "Database/ODBCImplementation/ODBCVariableSet.h"
#include "Database/DatabaseCalls.h"

class CCompoundInsertCountParams : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CCompoundInsertCountParams( void ) :
			BASECLASS(),
			ID()
		{}

		CCompoundInsertCountParams( uint64 id ) :
			BASECLASS(),
			ID( id )
		{}

		virtual ~CCompoundInsertCountParams() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &ID );
		}

		DBUInt64In ID;
};

class CCompoundInsertCountResultSet : public CODBCVariableSet
{
	public:

		typedef CODBCVariableSet BASECLASS;

		CCompoundInsertCountResultSet( void ) :
			BASECLASS(),
			InsertCount()
		{}

		virtual ~CCompoundInsertCountResultSet() {}

		virtual void Get_Variables( std::vector< IDatabaseVariable * > &variables )
		{
			variables.push_back( &InsertCount );
		}

		DBUInt32In InsertCount;
};

template< uint32 ISIZE, uint32 OSIZE >
class CCompoundInsertCountProcedureCall : public TDatabaseProcedureCall< CCompoundInsertCountParams, ISIZE, CCompoundInsertCountResultSet, OSIZE >
{
	public:

		typedef TDatabaseProcedureCall< CCompoundInsertCountParams, ISIZE, CCompoundInsertCountResultSet, OSIZE > BASECLASS;

		CCompoundInsertCountProcedureCall( uint64 id, uint32 expected_count ) : 
			BASECLASS(),
			ID( id ),
			Count( 0 ),
			ExpectedCount( expected_count )
		{}

		virtual ~CCompoundInsertCountProcedureCall() {}

		virtual const wchar_t *Get_Database_Object_Name( void ) const { return L"dynamic.test_compound_insert_count"; }

		void Verify_Results( void ) 
		{
			ASSERT_TRUE( Count == ExpectedCount );
		}

		uint32 Get_Insert_Count( void ) const { return Count; }

	protected:

		// WIP: signatures in flux
		virtual void Initialize_Parameters( IDatabaseVariableSet *input_parameters ) 
		{ 
			 CCompoundInsertCountParams* params = static_cast< CCompoundInsertCountParams * >( input_parameters );
			 *params = CCompoundInsertCountParams( ID );
	   }	
			
		virtual void On_Fetch_Results( IDatabaseVariableSet *result_set, int64 rows_fetched ) 
		{
			ASSERT_TRUE( rows_fetched == 1 );

			CCompoundInsertCountResultSet* results = static_cast< CCompoundInsertCountResultSet * >( result_set );
			Count = results[ 0 ].InsertCount.Get_Value();
		}
					
		virtual void On_Fetch_Results_Finished( IDatabaseVariableSet * /*input_parameters*/ ) {}	

		virtual void On_Rollback( void ) { ASSERT_TRUE( false ); }
		virtual void On_Task_Success( void ) { ASSERT_TRUE( false ); }				
		virtual void On_Task_Failure( void ) { ASSERT_TRUE( false ); }

	private:

		uint64 ID;
		uint32 Count;
		uint32 ExpectedCount;

};


#endif // ODBC_SHARED_H