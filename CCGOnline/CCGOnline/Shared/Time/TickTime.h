/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TickTime.h
		A component defining a simple wrapper class for a scalar time value.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

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

		explicit STickTime( uint64 ticks ) :
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

		uint64 Get_Ticks( void ) const { return Ticks; }

	private:

		uint64 Ticks;
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