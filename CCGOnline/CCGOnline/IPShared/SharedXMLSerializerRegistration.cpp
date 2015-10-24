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

#include "IPShared/SharedXMLSerializerRegistration.h"

#include "IPShared/Serialization/SerializationRegistrar.h"
#include "IPShared/Serialization/XML/PrimitiveXMLSerializers.h"
#include "IPShared/SlashCommands/SlashCommandDataDefinition.h"

using namespace IP::Command;
using namespace IP::Serialization;
using namespace IP::Serialization::XML;

namespace IP
{
namespace Global
{

void Register_IPShared_XML_Serializers( void )
{
	Register_Primitive_Serializers();

	REGISTER_PRIMITIVE_XML_SERIALIZER( ESlashCommandParamType, new CEnumXMLSerializer< ESlashCommandParamType > );

	CSlashCommandParam::Register_Type_Definition();
	CSlashCommandDataDefinition::Register_Type_Definition();
}

} // namespace Global
} // namespace IP