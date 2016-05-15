/**********************************************************************************************************************

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

#include <IPShared/Time/TimeKeeper.h>


namespace IP
{
namespace Time
{

CTimeKeeper::CTimeKeeper( void ) :
	BaseTime()
{
}

SystemTimePoint CTimeKeeper::Get_Current_Time( void ) const 
{ 
	return Get_Current_System_Time(); 
}

SystemDuration CTimeKeeper::Get_Elapsed_Time( void ) const
{
	return Get_Current_Time() - Get_Base_Time();
}


double CTimeKeeper::Get_Elapsed_Seconds( void ) const
{
	return Convert_Duration_To_Seconds( Get_Elapsed_Time() );
}

} // namespace Time
} // namespace IP