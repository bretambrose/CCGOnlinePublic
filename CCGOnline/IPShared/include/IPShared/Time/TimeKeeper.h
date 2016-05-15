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

#pragma once

#include <IPShared/IPShared.h>

#include <IPCore/System/Time.h>

namespace IP
{
namespace Time
{

// Class that tracks the current tick times for one or more time types
IPSHARED_API class CTimeKeeper
{
	public:

		CTimeKeeper( void );
		virtual ~CTimeKeeper() {}

		virtual SystemTimePoint Get_Current_Time( void ) const;
		SystemTimePoint Get_Base_Time( void ) const { return BaseTime; }
		virtual void Set_Base_Time( SystemTimePoint base_time ) { BaseTime = base_time; }

		SystemDuration Get_Elapsed_Time( void ) const;
		double Get_Elapsed_Seconds( void ) const;
		
	private:

		SystemTimePoint BaseTime;
};

} // namespace Time
} // namespace IP