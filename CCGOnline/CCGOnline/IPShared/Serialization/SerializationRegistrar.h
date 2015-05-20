#ifndef PLAYGROUND2_H
#define PLAYGROUND2_H

#include <functional>
#include "IPShared/Serialization/XML/XMLSerializerInterface.h"
#include "IPShared/Serialization/XML/PrimitiveXMLSerializers.h"


using XMLSerializerFactory = std::function< IXMLSerializer*( void ) >;

class CSerializerFactorySet
{
	public:
		CSerializerFactorySet(const XMLSerializerFactory &xml_factory) :
			XMLFactory( xml_factory )
		{}

		const XMLSerializerFactory &Get_XML_Factory( void ) const { return XMLFactory; }


	private:

		XMLSerializerFactory XMLFactory;
};

class IDataBinding
{
	public:

		virtual ~IDataBinding() {}

		virtual const std::wstring& Get_Name() const = 0;
		virtual uint64_t Get_Member_Offset() const = 0;
		virtual const Loki::TypeInfo& Get_Member_Type() const = 0;
		virtual const Loki::TypeInfo& Get_Polymorphic_Type() const = 0;

};

template< typename T, typename U, typename P >
class TDataBinding : public IDataBinding
{
	public:

		TDataBinding(const std::wstring& name, U T::* offset ) :
			Name( name ),
			Offset( offset ),
			MemberType( typeid( U ) ),
			PolymorphicType( typeid( P ) )
		{
		}

		virtual const std::wstring& Get_Name() const override { return Name; }
		virtual uint64_t Get_Member_Offset() const override
		{
			T dummy_object;
			U *member_pointer = &( dummy_object.*Offset );

			return reinterpret_cast< char * >( member_pointer ) - reinterpret_cast< char * >( &dummy_object );
		}

		virtual const Loki::TypeInfo&  Get_Member_Type() const override
		{
			return MemberType;
		}

		virtual const Loki::TypeInfo& Get_Polymorphic_Type() const override
		{
			return PolymorphicType;
		}

	private:

		std::wstring Name;
		U T::* Offset;
		Loki::TypeInfo MemberType;
		Loki::TypeInfo PolymorphicType;
};


template< typename T, typename U, typename P = U >
IDataBinding *Make_Data_Binding( const std::wstring& name, U T::* member_offset )
{
	return new TDataBinding< T, U, P >( name, member_offset );
}

class CTypeSerializationDefinition
{
	public:

		CTypeSerializationDefinition( void );
		CTypeSerializationDefinition( const Loki::TypeInfo &base_class_type );
		~CTypeSerializationDefinition();

		void Add_Binding( IDataBinding *binding ) { DataBindings.push_back( binding ); }

		const Loki::TypeInfo &Get_Base_Class( void ) const { return BaseClassType; }
		bool Has_Base_Class( void ) const { return HasBaseClass; }

		const std::vector< IDataBinding * > &Get_Data_Bindings( void ) const { return DataBindings; }

	private:

		std::vector< IDataBinding * > DataBindings;
		Loki::TypeInfo BaseClassType;
		bool HasBaseClass;
};

class CSerializationRegistrar
{
	public:
		
		template< typename T >
		static void Register_Primitive_XML_Serializer( IXMLSerializer *serializer )
		{
			Register_XML_Serializer( Loki::TypeInfo( typeid( T ) ), serializer );
		}

		template< typename T >
		static void Register_Type_Serialization_Definition( CTypeSerializationDefinition *definition )
		{
			Loki::TypeInfo type_info( typeid ( T ) );

			TypeSerializationDefinitions[ type_info ] = definition;
			Register_Factory_Set_For_Type< T >();
			
			if ( definition->Has_Base_Class() )
			{
				InheritanceRelationships[ type_info ] = definition->Get_Base_Class();
			}
		}

		static CTypeSerializationDefinition *Get_Type_Serialization_Definition( const Loki::TypeInfo& type_info );

		template< typename T, typename U = T >
		static void Register_Factory_Set_For_Type()
		{
			TypeInfoPair type_info_pair( Loki::TypeInfo( typeid( T ) ), Loki::TypeInfo( typeid( U ) ) );
			if ( FactorySets.find( type_info_pair ) != FactorySets.end() )
			{
				return;
			}

			CSerializerFactorySet *factory_set = new CSerializerFactorySet( CGetOrBuildXMLSerializer< T, U >() );
			FactorySets[ type_info_pair ] = factory_set;
		}

		template< typename T, typename U, typename P = U >
		static void Build_Binding_Set( CTypeSerializationDefinition *definition, const std::wstring &name, U T::* member_pointer )
		{
			definition->Add_Binding( Make_Data_Binding< T, U, U >( name, member_pointer ) );

			Register_Factory_Set_For_Type< U >();
		}

		template< typename T, typename U, typename P >
		static void Build_Polymorphic_Binding_Set( CTypeSerializationDefinition *definition, const std::wstring &name, U T::* member_pointer, const P* /*dummy_pointer*/ )
		{
			definition->Add_Binding( Make_Data_Binding< T, U, P >( name, member_pointer ) );

			FATAL_ASSERT( Loki::TypeInfo( typeid( U ) ) != Loki::TypeInfo( typeid( P ) ) );

			Register_Factory_Set_For_Type< U >();
			Register_Factory_Set_For_Type< P >();
			Register_Factory_Set_For_Type< U, P >();
		}

		static void Cleanup( void );

		template< typename T, typename U = T >
		static IXMLSerializer *Get_XML_Serializer( void )
		{
			return CGetOrBuildXMLSerializer< T, U >()();
		}

	private:

		static CSerializerFactorySet *Get_Factory_Set( const Loki::TypeInfo& type_info_base, const Loki::TypeInfo& type_info_derived );

		static void Register_XML_Serializer( const Loki::TypeInfo& type_info, IXMLSerializer *serializer );

		static void Add_Binding_Set_To_Serializer( CCompositeXMLSerializer *serializer, CTypeSerializationDefinition *type_definition )
		{
			auto binding_set = type_definition->Get_Data_Bindings();

			for ( uint32_t i = 0; i < binding_set.size(); ++i )
			{
				IDataBinding *binding = binding_set[ i ];
				auto factory_set = Get_Factory_Set( binding->Get_Member_Type(), binding->Get_Polymorphic_Type() );
				FATAL_ASSERT( factory_set != nullptr );

				serializer->Add( binding->Get_Name(), binding->Get_Member_Offset(), factory_set->Get_XML_Factory()() );
			}
		}

		template< typename T, typename U >
		class CGetOrBuildXMLSerializer
		{
			public:

				IXMLSerializer* operator()( void ) const {
					Loki::TypeInfo type_info( typeid( T ) );
					auto iter = XMLSerializers.find( type_info );
					if( iter != XMLSerializers.end() )
					{
						return iter->second;
					}

					auto type_definition_iter = TypeSerializationDefinitions.find( type_info );
					FATAL_ASSERT( type_definition_iter != TypeSerializationDefinitions.end() );

					CTypeSerializationDefinition *type_definition = type_definition_iter->second;

					auto serializer = new CCompositeXMLSerializer;
					Add_Binding_Set_To_Serializer( serializer, type_definition );

					while( type_definition->Has_Base_Class() )
					{
						auto base_definition_iter = TypeSerializationDefinitions.find( type_definition->Get_Base_Class() );
						FATAL_ASSERT( base_definition_iter != TypeSerializationDefinitions.end() );

						Add_Binding_Set_To_Serializer( serializer, base_definition_iter->second );

						type_definition = base_definition_iter->second;
					}

					return serializer;
				}
		};

		template< typename T, typename U >
		class CGetOrBuildXMLSerializer< std::vector< T >, U >
		{
			public:

				IXMLSerializer* operator()( void ) const {
					Loki::TypeInfo vector_type_info( typeid( std::vector< T > ) );
					auto iter = XMLSerializers.find( vector_type_info );
					if( iter != XMLSerializers.end() )
					{
						return iter->second;
					}

					IXMLSerializer *serializer = CGetOrBuildXMLSerializer< T, U >()();
					IXMLSerializer *vector_serializer = new CVectorXMLSerializer( serializer, Prep_Vector_For_Read< T > );
					XMLSerializers[ vector_type_info ] = vector_serializer;

					return vector_serializer;
				}
		};

		template< typename T, typename U >
		class CGetOrBuildXMLSerializer< T *, U >
		{
			public:

				IXMLSerializer* operator()( void ) const {
					Loki::TypeInfo pointer_type_info;
					bool is_inheritance = Inherits_From( Loki::TypeInfo( typeid( T ) ), Loki::TypeInfo( typeid( U ) ) );
					if( is_inheritance )
					{
						pointer_type_info = Loki::TypeInfo( typeid( U * ) );
					}
					else
					{
						pointer_type_info = Loki::TypeInfo( typeid( T * ) );
					}

					auto iter = XMLSerializers.find( pointer_type_info );
					if( iter != XMLSerializers.end() )
					{
						return iter->second;
					}

					IXMLSerializer *serializer = nullptr;
					IXMLSerializer *pointer_serializer = nullptr;

					if ( is_inheritance )
					{
						serializer = CGetOrBuildXMLSerializer< U, U >()();
						pointer_serializer = new CPointerXMLSerializer( serializer, Prep_Pointer_For_Read< U > );
					}
					else
					{
						serializer = CGetOrBuildXMLSerializer< T, U >()();
						pointer_serializer = new CPointerXMLSerializer( serializer, Prep_Pointer_For_Read< T > );
					}

					FATAL_ASSERT( pointer_serializer != nullptr );

					XMLSerializers[ pointer_type_info ] = pointer_serializer;

					return pointer_serializer;
				}
		};

		static bool Inherits_From( const Loki::TypeInfo &base_type_info, const Loki::TypeInfo &derived_type_info );

		using TypeSerializationDefinitionTableType = std::unordered_map< Loki::TypeInfo, CTypeSerializationDefinition *, STypeInfoContainerHelper >;
		using FactorySetTableType = std::unordered_map< TypeInfoPair, CSerializerFactorySet *, STypeInfoPairContainerHelper >;
		using XMLSerializerTableType = std::unordered_map< Loki::TypeInfo, IXMLSerializer *, STypeInfoContainerHelper >;
		using InheritanceTableType = std::unordered_map< Loki::TypeInfo, Loki::TypeInfo, STypeInfoContainerHelper >;

		static TypeSerializationDefinitionTableType TypeSerializationDefinitions;
		static FactorySetTableType FactorySets;
		static XMLSerializerTableType XMLSerializers;
		static InheritanceTableType InheritanceRelationships;
};


#define BEGIN_ROOT_DATA_BINDING_SET CTypeSerializationDefinition *definition = new CTypeSerializationDefinition;
#define BEGIN_DERIVED_DATA_BINDING_SET( bt ) CTypeSerializationDefinition *definition = new CTypeSerializationDefinition( Loki::TypeInfo( typeid( bt ) ) );
#define REGISTER_MEMBER_BINDING( n, mp ) CSerializationRegistrar::Build_Binding_Set( definition, n, mp );
#define REGISTER_POLYMORPHIC_MEMBER_BINDING(n, mp, pt) CSerializationRegistrar::Build_Polymorphic_Binding_Set( definition, n, mp, static_cast< const pt * >( nullptr ) );
#define END_DATA_BINDING_SET( t ) CSerializationRegistrar::Register_Type_Serialization_Definition< t >( definition ); 

#endif // PLAYGROUND2_H