#include "stdafx.h"

#include "IPShared/Serialization/SerializationRegistrar.h"

#include <stack>

CSerializationRegistrar::TypeSerializationDefinitionTableType CSerializationRegistrar::TypeSerializationDefinitions;
CSerializationRegistrar::FactorySetTableType CSerializationRegistrar::FactorySets;
CSerializationRegistrar::XMLSerializerTableType CSerializationRegistrar::XMLSerializers;
CSerializationRegistrar::DerivedClassTableType CSerializationRegistrar::DerivedClasses;
CSerializationRegistrar::PolymorphicTypesTableType CSerializationRegistrar::PolymorphicTypes;
CSerializationRegistrar::PolymorphicEnumTablesTableType CSerializationRegistrar::PolymorphicEnumTables;
CSerializationRegistrar::XMLSerializerTableType CSerializationRegistrar::PolymorphicSerializers;

void CSerializationRegistrar::Register_Primitive_XML_Serializer( const Loki::TypeInfo &type_info, IXMLSerializer *serializer )
{
	FATAL_ASSERT( serializer != nullptr );

	XMLSerializers[ type_info ] = serializer;
}

void CSerializationRegistrar::Add_Inheritance_Relationship( const Loki::TypeInfo &derived_type_info, const Loki::TypeInfo &base_type_info )
{
	FATAL_ASSERT( derived_type_info != base_type_info );

	auto iter = DerivedClasses.find( base_type_info );
	if ( iter == DerivedClasses.end() )
	{
		DerivedClassTableType::value_type entry( base_type_info, std::vector< Loki::TypeInfo >() );
		iter = DerivedClasses.insert( entry ).first;
	}

	iter->second.push_back( derived_type_info );
}

CTypeSerializationDefinition *CSerializationRegistrar::Get_Type_Serialization_Definition( const Loki::TypeInfo& type_info )
{
	auto iter = TypeSerializationDefinitions.find( type_info );
	if ( iter != TypeSerializationDefinitions.end() )
	{
		return iter->second;
	}

	return nullptr;
}

CSerializerFactorySet *CSerializationRegistrar::Get_Factory_Set( const Loki::TypeInfo& type_info )
{
	auto iter = FactorySets.find( type_info );
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

bool CSerializationRegistrar::Inherits_From( const Loki::TypeInfo &base_type_info, const Loki::TypeInfo &derived_type_info )
{
	if ( base_type_info == derived_type_info )
	{
		return true;
	}

	auto type_info = derived_type_info;
	auto iter = TypeSerializationDefinitions.find( type_info );
	while ( iter != TypeSerializationDefinitions.end() )
	{
		CTypeSerializationDefinition *type_definition = iter->second;
		if ( !type_definition->Has_Base_Class() )
		{
			return false;
		}

		const Loki::TypeInfo &base_info = type_definition->Get_Base_Type_Info();
		if ( base_info == base_type_info )
		{
			return true;
		}

		iter = TypeSerializationDefinitions.find( base_info );
	}

	return false;
}

bool CSerializationRegistrar::Has_Polymorphic_Enum_Mapping( const Loki::TypeInfo &type_info )
{
	return PolymorphicTypes.find( type_info ) != PolymorphicTypes.end();
}

IXMLSerializer *CSerializationRegistrar::Get_Or_Build_Polymorphic_Enum_Serializer( const Loki::TypeInfo &type_info )
{
	auto iter = PolymorphicTypes.find( type_info );
	FATAL_ASSERT( iter != PolymorphicTypes.end() );

	const auto &enum_type_info = iter->second;
	auto ser_iter = PolymorphicSerializers.find( enum_type_info );
	if ( ser_iter != PolymorphicSerializers.end() )
	{
		return ser_iter->second;
	}

	auto table_iter = PolymorphicEnumTables.find( enum_type_info );
	FATAL_ASSERT( table_iter != PolymorphicEnumTables.end() );

	auto table = table_iter->second;
	FATAL_ASSERT( table != nullptr );
	FATAL_ASSERT( table->size() > 0 );

	auto poly_serializer = new CEnumPolymorphicXMLSerializer( enum_type_info );

	for ( const auto &table_iter : *table )
	{
		const Loki::TypeInfo &entry_type_info = table_iter.second;
		CTypeSerializationDefinition *type_definition = Get_Type_Serialization_Definition( entry_type_info );
		FATAL_ASSERT( type_definition != nullptr );

		const Loki::TypeInfo &pointer_type_info = type_definition->Get_Pointer_Type_Info();
		auto pointer_iter = XMLSerializers.find( pointer_type_info );
		if ( pointer_iter == XMLSerializers.end() )
		{
			auto entry_iter = XMLSerializers.find( entry_type_info );
			if ( entry_iter == XMLSerializers.end() )
			{
				auto entry_serializer = Build_Composite_Serializer( entry_type_info );
				FATAL_ASSERT( entry_serializer != nullptr );
				XMLSerializers[ entry_type_info ] = entry_serializer;

				entry_iter = XMLSerializers.find( entry_type_info );
			}

			auto pointer_serializer = new CPointerXMLSerializer( entry_iter->second, type_definition->Get_Pointer_Prep_Delegate() );
			FATAL_ASSERT( pointer_serializer != nullptr );

			XMLSerializers[ pointer_type_info ] = pointer_serializer;
			
			pointer_iter = XMLSerializers.find( pointer_type_info );
		}

		FATAL_ASSERT( pointer_iter != XMLSerializers.end() );
		poly_serializer->Add( table_iter.first, pointer_iter->second );
	}

	PolymorphicSerializers[ enum_type_info ] = poly_serializer;

	return poly_serializer;
}

CTypeSerializationDefinition *CSerializationRegistrar::Get_Most_Derived_Type_Singleton( const Loki::TypeInfo &type_info )
{
	CTypeSerializationDefinition *most_derived = nullptr;

	auto iter = DerivedClasses.find( type_info );
	while ( iter != DerivedClasses.end() )
	{
		size_t derived_class_count = iter->second.size();
		if ( derived_class_count > 1 )
		{
			return nullptr;
		}

		FATAL_ASSERT( derived_class_count == 1 );

		const Loki::TypeInfo &derived_type_info = iter->second[ 0 ];
		most_derived = TypeSerializationDefinitions[ derived_type_info ];

		iter = DerivedClasses.find( derived_type_info );
	}

	return most_derived;
}

void CSerializationRegistrar::Build_Derived_Type_List( const Loki::TypeInfo &type_info, std::vector< Loki::TypeInfo > &derived_types )
{
	std::stack< Loki::TypeInfo > type_stack;
	type_stack.push( type_info );

	while( !type_stack.empty() )
	{
		Loki::TypeInfo top_info = type_stack.top();
		type_stack.pop();

		derived_types.push_back( top_info );

		auto derived_iter = DerivedClasses.find( top_info );
		const auto &derived_types = derived_iter->second;
		for ( const auto &iter : derived_types )
		{
			type_stack.push( iter );
		}
	}
}

void CSerializationRegistrar::Finalize( void )
{
	for ( const auto &poly_entry : PolymorphicTypes )
	{
		const Loki::TypeInfo &enum_type = poly_entry.second;
		Loki::TypeInfo type_chain = poly_entry.first;
		CTypeSerializationDefinition *type_chain_definition = TypeSerializationDefinitions[ type_chain ];
		while ( type_chain_definition->Has_Base_Class() )
		{
			type_chain = type_chain_definition->Get_Base_Type_Info();
			auto iter = PolymorphicTypes.find( type_chain );
			if ( iter == PolymorphicTypes.end() )
			{
				PolymorphicTypes[ type_chain ] = enum_type;
			}
			else
			{
				FATAL_ASSERT( iter->second == enum_type );
			} 

			type_chain_definition = TypeSerializationDefinitions[ type_chain ];
		}
	}

	for ( const auto &type_entry : TypeSerializationDefinitions )
	{
		CTypeSerializationDefinition *type_definition = type_entry.second;

		const Loki::TypeInfo &type_info = type_definition->Get_Type_Info();
		if( XMLSerializers.find( type_info ) != XMLSerializers.end() )
		{
			continue;
		}

		auto serializer = Build_Composite_Serializer( type_info );
		FATAL_ASSERT( serializer != nullptr );

		XMLSerializers[ type_info ] = serializer;

		const Loki::TypeInfo &pointer_type_info = type_definition->Get_Pointer_Type_Info();
		FATAL_ASSERT( XMLSerializers.find( pointer_type_info ) == XMLSerializers.end() );

		auto pointer_serializer = new CPointerXMLSerializer( serializer, type_definition->Get_Pointer_Prep_Delegate() );
		
		XMLSerializers[ pointer_type_info ] = pointer_serializer;
	}
}
