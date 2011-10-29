/**********************************************************************************************************************

	[Placeholder for eventual source license]

	TimeType.h
		Definition for a type that enumerate the different kinds of tick-based time scales used in the library

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef TIME_TYPE_H
#define TIME_TYPE_H

enum ETimeType
{
	TT_FIRST = 0,

	TT_REAL_TIME = TT_FIRST,
	TT_GAME_TIME,

	TT_COUNT
};

#endif // TIME_TYPE_H
