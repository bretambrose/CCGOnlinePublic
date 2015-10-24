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

enum EDatabaseVariableType;
enum EDatabaseVariableValueType;

namespace IP
{
namespace Db
{

class IDatabaseVariable
{
	public:

		virtual ~IDatabaseVariable() {}

		virtual EDatabaseVariableType Get_Parameter_Type( void ) const = 0;			// Input only
		virtual EDatabaseVariableValueType Get_Value_Type( void ) const = 0;
		virtual uint32_t Get_Value_Size( void ) const = 0;									// Input only
		virtual uint32_t Get_Decimals( void ) const = 0;										// Input only
		virtual void *Get_Value_Address( void ) = 0;
		virtual uint32_t Get_Value_Buffer_Size( void ) const = 0;
		virtual void *Get_Auxiliary_Address( void ) = 0;
		virtual bool Is_Null( void ) const = 0;

};

} // namespace Db
} // namespace IP