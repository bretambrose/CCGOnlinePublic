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

#include "IPDatabase/Interfaces/DatabaseCallContextInterface.h"

namespace IP
{
namespace Db
{

template < typename I, uint32_t ISIZE, typename O, uint32_t OSIZE >
class CDatabaseCallContext : public IDatabaseCallContext
{
	public:

		using BASECLASS = IDatabaseCallContext;

		CDatabaseCallContext( void ) :
			BASECLASS(),
			StatementText( L"" )
		{}

		virtual ~CDatabaseCallContext() {}

		// IDatabaseCallContext interface
		// Param accessors
		virtual IDatabaseVariableSet *Get_Param_Rows( void ) { return &Params[0]; }
		virtual IDatabaseVariableSet *Get_Param_Row( uint32_t index ) {
			if ( index < ISIZE )
			{
				return &Params[ index ];
			}

			return nullptr;
		}
		virtual uint32_t Get_Param_Row_Count( void ) const { return ISIZE; }
		virtual uint32_t Get_Sizeof_Param_Type( void ) const { return sizeof(I); }

		// Result accessors
		virtual IDatabaseVariableSet *Get_Result_Rows( void ) { return &Results[0]; }
		virtual IDatabaseVariableSet *Get_Result_Row( uint32_t index ) {
			if ( index < OSIZE )
			{
				return &Results[ index ];
			}

			return nullptr;
		}
		virtual uint32_t Get_Result_Row_Count( void ) const { return OSIZE; }
		virtual uint32_t Get_Sizeof_Result_Type( void ) const { return sizeof(O); }

		virtual const std::wstring & Get_Statement_Text( void ) const { return StatementText; }
		virtual void Set_Statement_Text( const std::wstring &statement_text ) { StatementText = statement_text; }

	private:
		
		I Params[ISIZE];
		O Results[OSIZE];

		std::wstring StatementText;
};

} // namespace Db
} // namespace IP
