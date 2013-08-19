/**********************************************************************************************************************

	SharedTest.cpp
		the entry point for the console application that runs all the shared library tests

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

#include "Shared.h"
#include "GeneratedCode/RegisterSharedTestEnums.h"

class Movable
{
	public:
		Movable( uint32 data ) :
			Data( data )
		{}

		Movable( void ) :
			Data( 0 )
		{}

		Movable( const Movable &rhs ) :
			Data( rhs.Data )
		{}

		~Movable() {}

		uint32 Data;
};

class Holder
{
	public:
		
		Holder( void ) :
			Movables()
		{}

		Holder( Holder &&rhs ) :
			Movables( std::move( rhs.Movables ) )
		{}

		void Add_Via_Move1( unique_ptr< Movable > &movable )
		{
			Movables.emplace_back( std::move( movable ) );
		}

/*		void Add_Via_Move2( unique_ptr< Movable > &&movable )
		{
			Movables.emplace_back( movable );
		}
*/
		void Fetch( std::vector< unique_ptr< Movable > > &movables )
		{
			movables.reserve( movables.size() + Movables.size() );
			
			std::move( Movables.begin(), Movables.end(), std::back_inserter( movables ) );

			Movables.clear();
		}

	private:

		std::vector< unique_ptr< Movable > > Movables;
};


namespace NSharedTest
{
	void Initialize( void )
	{
		NShared::Initialize();
		Register_SharedTest_Enums();
	}

	void Shutdown( void )
	{
		NShared::Shutdown();
	}
}

int main(int argc, wchar_t* argv[])
{
	NSharedTest::Initialize();

	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	NSharedTest::Shutdown();

	return 0;
}

