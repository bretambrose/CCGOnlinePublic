/**********************************************************************************************************************

	TickTime.h
		A component defining a simple wrapper class for a scalar time value.

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

#ifndef TICK_TIME_H
#define TICK_TIME_H

struct STickTime
{
	public:

		STickTime( void ) :
			Ticks( 0 )
		{}

		STickTime( const STickTime &rhs ) :
			Ticks( rhs.Ticks )
		{}

		explicit STickTime( uint64_t ticks ) :
			Ticks( ticks )
		{}

		STickTime & operator =( const STickTime &rhs )
		{
			if ( this != &rhs )
			{
				Ticks = rhs.Ticks;
			}

			return *this;
		}

		STickTime & operator +=( const STickTime &rhs )
		{
			Ticks += rhs.Ticks;
			return *this;
		}

		STickTime & operator -=( const STickTime &rhs )
		{
			Ticks -= rhs.Ticks;
			return *this;
		}

		bool operator ==( const STickTime &rhs ) const
		{
			return Ticks == rhs.Ticks;
		}

		bool operator !=( const STickTime &rhs ) const
		{
			return Ticks != rhs.Ticks;
		}

		uint64_t Get_Ticks( void ) const { return Ticks; }

	private:

		uint64_t Ticks;
};

inline STickTime operator +( const STickTime &lhs, const STickTime &rhs )
{
	return STickTime( lhs.Get_Ticks() + rhs.Get_Ticks() );
}

inline STickTime operator -( const STickTime &lhs, const STickTime &rhs )
{
	return STickTime( lhs.Get_Ticks() - rhs.Get_Ticks() );
}

inline bool operator < ( const STickTime &lhs, const STickTime &rhs )
{
	return lhs.Get_Ticks() < rhs.Get_Ticks();
}

#endif // TICK_TIME_H