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

#include <IPShared/Concurrency/ProcessSubject.h>
#include <IPShared/Concurrency/ProcessProperties.h>
#include <IPShared/Concurrency/ProcessID.h>

namespace IP
{
namespace Execution
{

static const SProcessProperties LOGGING_PROCESS_PROPERTIES( EProcessSubject::LOGGING );
static const SProcessProperties MANAGER_PROCESS_PROPERTIES( EProcessSubject::CONCURRENCY_MANAGER );

static const EProcessID MANAGER_PROCESS_ID( EProcessID::CONCURRENCY_MANAGER );
static const EProcessID LOGGING_PROCESS_ID( EProcessID::LOGGING );

} // namespace Execution
} // namespace IP