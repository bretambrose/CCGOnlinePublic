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

#include "targetver.h"

#include "IPPlatform/Universal.h"

// std includes
#include <list>
#include <vector>
#include <set>
#include <unordered_map>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <functional>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <type_traits>
#include <assert.h>

// Loki includes
#include "loki/include/loki/LokiTypeInfo.h"

// Misc includes
#pragma warning( push )
#pragma warning( disable : 4100 )
#include "IPPlatform/FastDelegate.h"
#pragma warning( pop ) 

// Global using directives; be careful with these
using namespace fastdelegate;

// self includes
#include "IPPlatform/DebugAssert.h"
#include "IPShared/TypeInfoUtils.h"
#include "gtest/gtest.h"

