/**********************************************************************************************************************

	[Placeholder for eventual source license]

	stdafx.h
		Pre-compiled header includes for the Shared library

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

#pragma once

#include "targetver.h"

// std includes
#include <vector>
#include <set>
#include <hash_map>
#include <map>
#include <memory>
#include <string>
#include <algorithm>
#include <assert.h>

// Loki includes
#include "loki/LokiTypeInfo.h"

// Boost includes
#include <boost/scoped_ptr.hpp>

// Misc includes
#pragma warning( push )
#pragma warning( disable : 4100 )
#include "FastDelegate.h"
#pragma warning( pop ) 

// Global using directives; be careful with these
using namespace fastdelegate;

using std::tr1::shared_ptr;
using std::tr1::static_pointer_cast;

using boost::scoped_ptr;

// self includes
#include "PlatformTypes.h"
#include "DebugAssert.h"

