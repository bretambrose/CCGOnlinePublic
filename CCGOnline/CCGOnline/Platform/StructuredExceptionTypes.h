/**********************************************************************************************************************

	[Placeholder for eventual source license]

	StructuredExceptionTypes.h
		Typedef for the agnostic exception handler.  Used by both Platform and Shared code and thus separated out to
		minimize include dependencies

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#ifndef STRUCTURED_EXCEPTION_TYPES_H
#define STRUCTURED_EXCEPTION_TYPES_H

class CStructuredExceptionInfo;

typedef FastDelegate1< CStructuredExceptionInfo &, void > DExceptionHandler;

#endif // STRUCTURED_EXCEPTION_TYPES_H
