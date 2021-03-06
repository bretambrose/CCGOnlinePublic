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

#include "stdafx.h"

#include "IPShared/Serialization/SerializationHelpers.h"

namespace IP
{
namespace Serialization
{

CTypeSerializationDefinition::CTypeSerializationDefinition( void ) :
	Type(),
	PointerType(),
	BaseType(),
	HasBaseClass( false ),
	DataBindings(),
	VectorPrepDelegate(),
	PointerPrepDelegate(),
	FactoryDelegate()
{
}

CTypeSerializationDefinition::~CTypeSerializationDefinition()
{
	std::for_each( DataBindings.begin(), DataBindings.end(), [ & ]( IDataBinding *binding ){ delete binding; } );
}

} // namespace Serialization
} // namespace IP