#include "stdafx.h"

#include "IPShared/Serialization/SerializationRegistrar.h"

CTypeSerializationDefinition::CTypeSerializationDefinition() :
	DataBindings(),
	BaseClassType(),
	HasBaseClass( false )
{
}

CTypeSerializationDefinition::CTypeSerializationDefinition(const Loki::TypeInfo &base_class_type) :
	DataBindings(),
	BaseClassType( base_class_type ),
	HasBaseClass( true )
{
}

CTypeSerializationDefinition::~CTypeSerializationDefinition()
{
	std::for_each( DataBindings.begin(), DataBindings.end(), [ & ]( IDataBinding *binding ){ delete binding; } );
}


CSerializationRegistrar::TypeSerializationDefinitionTableType CSerializationRegistrar::TypeSerializationDefinitions;
CSerializationRegistrar::FactorySetTableType CSerializationRegistrar::FactorySets;
CSerializationRegistrar::XMLSerializerTableType CSerializationRegistrar::XMLSerializers;
CSerializationRegistrar::InheritanceTableType CSerializationRegistrar::InheritanceRelationships;

CTypeSerializationDefinition *CSerializationRegistrar::Get_Type_Serialization_Definition( const Loki::TypeInfo& type_info )
{
	auto iter = TypeSerializationDefinitions.find( type_info );
	if ( iter != TypeSerializationDefinitions.end() )
	{
		return iter->second;
	}

	return nullptr;
}

CSerializerFactorySet *CSerializationRegistrar::Get_Factory_Set( const Loki::TypeInfo& type_info_base, const Loki::TypeInfo& type_info_derived )
{
	auto iter = FactorySets.find( TypeInfoPair( type_info_base, type_info_derived ) );
	if ( iter != FactorySets.end() )
	{
		return iter->second;
	}

	return nullptr;
}

void CSerializationRegistrar::Cleanup()
{
	for ( auto iter : TypeSerializationDefinitions )
	{
		delete iter.second;
	}

	for ( auto iter : FactorySets )
	{
		delete iter.second;
	}

	for ( auto iter : XMLSerializers )
	{
		delete iter.second;
	}
}

void CSerializationRegistrar::Register_XML_Serializer( const Loki::TypeInfo& type_info, IXMLSerializer *serializer )
{
	XMLSerializers[ type_info ] = serializer;
}

bool CSerializationRegistrar::Inherits_From( const Loki::TypeInfo &base_type_info, const Loki::TypeInfo &derived_type_info )
{
	if ( base_type_info == derived_type_info )
	{
		return true;
	}

	auto type_info = derived_type_info;
	auto iter = InheritanceRelationships.find( type_info );
	while ( iter != InheritanceRelationships.end() )
	{
		if ( iter->second == base_type_info )
		{
			return true;
		}

		iter = InheritanceRelationships.find( iter->second );
	}

	return false;
}


