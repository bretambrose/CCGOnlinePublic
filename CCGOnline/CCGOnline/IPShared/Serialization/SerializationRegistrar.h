#ifndef SERIALIZATION_REGISTRAR_H
#define SERIALIZATION_REGISTRAR_H

#include <functional>
#include "IPShared/Serialization/SerializationBinding.h"
#include "IPShared/Serialization/XML/XMLSerializerInterface.h"
#include "IPShared/Serialization/XML/PrimitiveXMLSerializers.h"


class CSerializationRegistrar
{
	public:
		
		// registration
		static void Register_Primitive_XML_Serializer( const Loki::TypeInfo &type_info, IXMLSerializer *serializer );

		template< typename T >
		static void Register_Type_Serialization_Definition( CTypeSerializationDefinition *definition );

		template< typename T, typename U >
		static void Build_Binding_Set( CTypeSerializationDefinition *definition, const std::wstring &name, U T::* member_pointer, bool allow_polymorphism );

		template < typename T >
		static void Register_Polymorphic_Enum_Entry( T enum_entry, const Loki::TypeInfo &type_info );

		// Administration
		static void Finalize( void );
		static void Cleanup( void );

		// Accessors
		static CTypeSerializationDefinition *Get_Type_Serialization_Definition( const Loki::TypeInfo& type_info );

		template< typename T >
		static IXMLSerializer *Get_XML_Serializer( bool allow_polymorphism = true );

	private:

		static void Add_Inheritance_Relationship( const Loki::TypeInfo &derived_type_info, const Loki::TypeInfo &base_type_info );

		template< typename T >
		static void Register_Factory_Set_For_Type()
		{
			Loki::TypeInfo type_info( typeid( T ) );
			if ( FactorySets.find( type_info ) != FactorySets.end() )
			{
				return;
			}

			CSerializerFactorySet *factory_set = new CSerializerFactorySet( CGetOrBuildXMLSerializer< T >() );
			FactorySets[ type_info ] = factory_set;
		}

		static CSerializerFactorySet *Get_Factory_Set( const Loki::TypeInfo& type_info );

		static void Register_XML_Serializer( const Loki::TypeInfo& type_info, IXMLSerializer *serializer );

		static void Add_Binding_Set_To_Serializer( CCompositeXMLSerializer *serializer, CTypeSerializationDefinition *type_definition )
		{
			auto binding_set = type_definition->Get_Data_Bindings();

			for ( uint32_t i = 0; i < binding_set.size(); ++i )
			{
				IDataBinding *binding = binding_set[ i ];
				auto factory_set = Get_Factory_Set( binding->Get_Member_Type() );
				FATAL_ASSERT( factory_set != nullptr );

				serializer->Add( binding->Get_Name(), binding->Get_Member_Offset(), factory_set->Get_XML_Factory()( binding->Allow_Polymorphism() ) );
			}
		}

		static IXMLSerializer *Build_Composite_Serializer( const Loki::TypeInfo &type_info )
		{
			auto type_definition_iter = TypeSerializationDefinitions.find( type_info );
			FATAL_ASSERT( type_definition_iter != TypeSerializationDefinitions.end() );

			CTypeSerializationDefinition *type_definition = type_definition_iter->second;
			FATAL_ASSERT( type_definition != nullptr );

			auto serializer = new CCompositeXMLSerializer;
			Add_Binding_Set_To_Serializer( serializer, type_definition );

			while( type_definition->Has_Base_Class() )
			{
				auto base_definition_iter = TypeSerializationDefinitions.find( type_definition->Get_Base_Type_Info() );
				FATAL_ASSERT( base_definition_iter != TypeSerializationDefinitions.end() );

				Add_Binding_Set_To_Serializer( serializer, base_definition_iter->second );

				type_definition = base_definition_iter->second;
			}

			return serializer;
		}

		template< typename T >
		class CGetOrBuildXMLSerializer
		{
			public:

				IXMLSerializer* operator()( bool allow_polymorphism ) const {
					IP_UNREFERENCED_PARAM( allow_polymorphism );

					Loki::TypeInfo type_info( typeid( T ) );
					auto iter = XMLSerializers.find( type_info );
					if( iter != XMLSerializers.end() )
					{
						return iter->second;
					}

					auto serializer = Build_Composite_Serializer( type_info );
					FATAL_ASSERT( serializer != nullptr );

					XMLSerializers[ type_info ] = serializer;

					return serializer;
				}
		};

		template< typename T >
		class CGetOrBuildXMLSerializer< std::vector< T > >
		{
			public:

				IXMLSerializer* operator()( bool allow_polymorphism ) const {
					Loki::TypeInfo type_info( typeid( std::vector< T > ) );
					auto iter = XMLSerializers.find( type_info );
					if( iter != XMLSerializers.end() )
					{
						return iter->second;
					}

					IXMLSerializer *serializer = CGetOrBuildXMLSerializer< T >()( allow_polymorphism );
					IXMLSerializer *vector_serializer = new CVectorXMLSerializer( serializer, Prep_Vector_For_Read< T > );
					XMLSerializers[ type_info ] = vector_serializer;

					return vector_serializer;
				}
		};

		template< typename T >
		class CGetOrBuildXMLSerializer< T * >
		{
			public:

				IXMLSerializer* operator()( bool allow_polymorphism ) const {
					Loki::TypeInfo type_info( typeid( T ) );
					auto type_iter = TypeSerializationDefinitions.find( type_info );

					if ( allow_polymorphism && type_iter != TypeSerializationDefinitions.end() )
					{
						if ( Has_Polymorphic_Enum_Mapping( type_info ) )
						{
							return Get_Or_Build_Polymorphic_Enum_Serializer( type_info );
						}

						CTypeSerializationDefinition *most_derived_type_definition = Get_Most_Derived_Type_Singleton( type_info );
						if ( most_derived_type_definition != nullptr )
						{
							const Loki::TypeInfo &most_derived_pointer_type_info = most_derived_type_definition->Get_Pointer_Type_Info();
							auto pointer_derived_iter = XMLSerializers.find( most_derived_pointer_type_info );
							if ( pointer_derived_iter != XMLSerializers.end() )
							{
								return pointer_derived_iter->second;
							}

							const Loki::TypeInfo &most_derived_type_info = most_derived_type_definition->Get_Type_Info();
							auto derived_iter = XMLSerializers.find( most_derived_type_info );
							if ( derived_iter == XMLSerializers.end() )
							{
								auto derived_serializer = Build_Composite_Serializer( most_derived_type_info );
								FATAL_ASSERT( derived_serializer != nullptr );
								XMLSerializers[ most_derived_type_info ] = derived_serializer;

								derived_iter = XMLSerializers.find( most_derived_type_info );
							}

							FATAL_ASSERT( derived_iter != XMLSerializers.end() );

							auto derived_serializer = derived_iter->second;
							FATAL_ASSERT( derived_serializer != nullptr );

							auto pointer_serializer = new CPointerXMLSerializer( derived_serializer, most_derived_type_definition->Get_Pointer_Prep_Delegate() );
							FATAL_ASSERT( pointer_serializer != nullptr );

							XMLSerializers[ most_derived_pointer_type_info ] = pointer_serializer;

							return pointer_serializer;
						}
					}

					Loki::TypeInfo pointer_type_info( typeid( T * ) );

					auto iter = XMLSerializers.find( pointer_type_info );
					if( iter != XMLSerializers.end() )
					{
						return iter->second;
					}

					IXMLSerializer *serializer = CGetOrBuildXMLSerializer< T >()( allow_polymorphism );
					IXMLSerializer *pointer_serializer = new CPointerXMLSerializer( serializer, Prep_Pointer_For_Read< T > );

					FATAL_ASSERT( pointer_serializer != nullptr );

					XMLSerializers[ pointer_type_info ] = pointer_serializer;

					return pointer_serializer;
				}
		};

		static bool Inherits_From( const Loki::TypeInfo &base_type_info, const Loki::TypeInfo &derived_type_info );

		static bool Has_Polymorphic_Enum_Mapping( const Loki::TypeInfo &type_info );
		static IXMLSerializer *Get_Or_Build_Polymorphic_Enum_Serializer( const Loki::TypeInfo &type_info );
		static CTypeSerializationDefinition *Get_Most_Derived_Type_Singleton( const Loki::TypeInfo &type_info );

		static void Build_Derived_Type_List( const Loki::TypeInfo &type_info, std::vector< Loki::TypeInfo > &derived_types );

		using TypeSerializationDefinitionTableType = std::unordered_map< Loki::TypeInfo, CTypeSerializationDefinition *, STypeInfoContainerHelper >;
		using FactorySetTableType = std::unordered_map< Loki::TypeInfo, CSerializerFactorySet *, STypeInfoContainerHelper >;
		using XMLSerializerTableType = std::unordered_map< Loki::TypeInfo, IXMLSerializer *, STypeInfoContainerHelper >;
		using DerivedClassTableType = std::unordered_map< Loki::TypeInfo, std::vector< Loki::TypeInfo >, STypeInfoContainerHelper >;

		using PolymorphicTypesTableType = std::unordered_map< Loki::TypeInfo, Loki::TypeInfo, STypeInfoContainerHelper >;
		using PolymorphicEnumTableType = std::unordered_map< uint64_t, Loki::TypeInfo >;
		using PolymorphicEnumTablesTableType = std::unordered_map< Loki::TypeInfo, PolymorphicEnumTableType *, STypeInfoContainerHelper >;

		static TypeSerializationDefinitionTableType TypeSerializationDefinitions;
		static FactorySetTableType FactorySets;
		static XMLSerializerTableType XMLSerializers;
		static DerivedClassTableType DerivedClasses;

		static PolymorphicTypesTableType PolymorphicTypes;
		static PolymorphicEnumTablesTableType PolymorphicEnumTables;
		static XMLSerializerTableType PolymorphicSerializers;
};


#define BEGIN_ROOT_TYPE_DEFINITION( t ) CTypeSerializationDefinition *definition = CTypeSerializationDefinition::Create< t >();
#define BEGIN_DERIVED_TYPE_DEFINITION( t, bt ) CTypeSerializationDefinition *definition = CTypeSerializationDefinition::Create< t >( Loki::TypeInfo( typeid( bt ) ) );
#define REGISTER_MEMBER_BINDING( n, mp ) CSerializationRegistrar::Build_Binding_Set( definition, n, mp, true );
#define REGISTER_LITERAL_MEMBER_BINDING( n, mp ) CSerializationRegistrar::Build_Binding_Set( definition, n, mp, false );
#define END_TYPE_DEFINITION( t ) CSerializationRegistrar::Register_Type_Serialization_Definition< t >( definition ); 

#define REGISTER_PRIMITIVE_XML_SERIALIZER( t, s ) CSerializationRegistrar::Register_Primitive_XML_Serializer( Loki::TypeInfo( typeid( t ) ), s );

#define REGISTER_POLYMORPHIC_ENUM_ENTRY( e, t ) CSerializationRegistrar::Register_Polymorphic_Enum_Entry( e, Loki::TypeInfo( typeid( t ) ) );

#define REGISTER_ENUM_SERIALIZER( e ) CSerializationRegistrar::Register_Primitive_XML_Serializer( Loki::TypeInfo( typeid( e ) ), new CEnumXMLSerializer< e > );

template< typename T >
void CSerializationRegistrar::Register_Type_Serialization_Definition( CTypeSerializationDefinition *definition )
{
	FATAL_ASSERT( definition != nullptr );

	const Loki::TypeInfo &type_info = definition->Get_Type_Info();

	TypeSerializationDefinitions[ type_info ] = definition;
	Register_Factory_Set_For_Type< T >();
			
	if ( definition->Has_Base_Class() )
	{
		Add_Inheritance_Relationship( type_info, definition->Get_Base_Type_Info() );
	}
}

template< typename T, typename U >
void CSerializationRegistrar::Build_Binding_Set( CTypeSerializationDefinition *definition, const std::wstring &name, U T::* member_pointer, bool allow_polymorphism )
{
	definition->Add_Binding( Make_Data_Binding< T, U >( name, member_pointer, allow_polymorphism ) );

	Register_Factory_Set_For_Type< U >();
}

template< typename T >
IXMLSerializer *CSerializationRegistrar::Get_XML_Serializer( bool allow_polymorphism )
{
	return CGetOrBuildXMLSerializer< T >()( allow_polymorphism );
}

template < typename T >
void CSerializationRegistrar::Register_Polymorphic_Enum_Entry( T enum_entry, const Loki::TypeInfo &type_info )
{
	Loki::TypeInfo enum_type( typeid( T ) );
	PolymorphicTypes[ type_info ] = enum_type;

	auto iter = PolymorphicEnumTables.find( enum_type );
	if ( iter == PolymorphicEnumTables.end() )
	{
		auto enum_table = new PolymorphicEnumTableType;
		PolymorphicEnumTables[ enum_type ] = enum_table;

		iter = PolymorphicEnumTables.find( enum_type );
	}

	FATAL_ASSERT( iter != PolymorphicEnumTables.end() );
			
	auto enum_table = iter->second;
	FATAL_ASSERT( enum_table != nullptr );

	( *enum_table )[ static_cast< uint64_t >( enum_entry ) ] = type_info;
}

#endif // SERIALIZATION_REGISTRAR_H