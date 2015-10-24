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

#include <functional>

#include "IPShared/Serialization/XML/XMLSerializerInterface.h"

namespace IP
{
namespace Serialization
{

using DPrepDestinationForRead = std::function< void *(void *) >;

template< typename T >
void *Prep_Vector_For_Read( void *destination )
{
	std::vector< T > *dest = reinterpret_cast< std::vector< T > * >( destination );
	
	dest->push_back( T() );
	return static_cast< void * >( &( *dest )[ dest->size() - 1 ] );
}

template< typename T >
void *Prep_Pointer_For_Read( void *destination )
{
	T **dest = reinterpret_cast< T ** >( destination );
	
	*dest = new T;

	return static_cast< void * >( *dest );
}

using XMLSerializerFactory = std::function< XML::IXMLSerializer*( bool ) >;

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

		virtual const std::wstring &Get_Name( void ) const = 0;
		virtual uint64_t Get_Member_Offset( void ) const = 0;
		virtual const Loki::TypeInfo &Get_Member_Type( void ) const = 0;
		virtual bool Allow_Polymorphism( void ) const = 0;

};

template< typename T, typename U >
class TDataBinding : public IDataBinding
{
	public:

		TDataBinding(const std::wstring& name, U T::* offset, bool allow_polymorphism ) :
			Name( name ),
			Offset( offset ),
			MemberType( typeid( U ) ),
			AllowPolymorphism( allow_polymorphism )
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

		virtual bool Allow_Polymorphism( void ) const override { return AllowPolymorphism; }

	private:

		std::wstring Name;
		U T::* Offset;
		Loki::TypeInfo MemberType;
		bool AllowPolymorphism;
};


template< typename T, typename U >
IDataBinding *Make_Data_Binding( const std::wstring& name, U T::* member_offset, bool allow_polymorphism )
{
	return new TDataBinding< T, U >( name, member_offset, allow_polymorphism );
}

using VoidFactory = std::function< void *( void ) >;

class CTypeSerializationDefinition
{
	public:

		template < typename T >
		static CTypeSerializationDefinition *Create( void )
		{
			CTypeSerializationDefinition *type_definition = new CTypeSerializationDefinition;

			Initialize_Type_Members< T >( type_definition );

			return type_definition;
		}

		template < typename T >
		static CTypeSerializationDefinition *Create( const Loki::TypeInfo &base_class_type )
		{
			CTypeSerializationDefinition *type_definition = new CTypeSerializationDefinition;
			type_definition->BaseType = base_class_type;
			type_definition->HasBaseClass = true;

			Initialize_Type_Members< T >( type_definition );

			return type_definition;
		}

		~CTypeSerializationDefinition();

		const Loki::TypeInfo &Get_Type_Info( void ) const { return Type; }
		const Loki::TypeInfo &Get_Pointer_Type_Info() const { return PointerType; }

		const Loki::TypeInfo &Get_Base_Type_Info( void ) const { return BaseType; }
		bool Has_Base_Class( void ) const { return HasBaseClass; }

		const std::vector< IDataBinding * > &Get_Data_Bindings( void ) const { return DataBindings; }
		void Add_Binding( IDataBinding *binding ) { DataBindings.push_back( binding ); }

		const DPrepDestinationForRead &Get_Vector_Prep_Delegate( void ) const { return VectorPrepDelegate; }
		const DPrepDestinationForRead &Get_Pointer_Prep_Delegate( void ) const { return PointerPrepDelegate; }
		const VoidFactory &Get_Factory_Delegate( void ) const { return FactoryDelegate; }

	private:

		CTypeSerializationDefinition( void );

		template < typename T >
		static void Initialize_Type_Members( CTypeSerializationDefinition *type_definition )
		{
			type_definition->Type = Loki::TypeInfo( typeid( T ) );
			type_definition->PointerType = Loki::TypeInfo( typeid( T * ) );
			type_definition->VectorPrepDelegate = Prep_Vector_For_Read< T >;
			type_definition->PointerPrepDelegate = Prep_Pointer_For_Read< T >;
			type_definition->FactoryDelegate = []( void ){ return static_cast< void * >( new T ); };
		}

		Loki::TypeInfo Type;
		Loki::TypeInfo PointerType;
		Loki::TypeInfo BaseType;
		bool HasBaseClass;

		std::vector< IDataBinding * > DataBindings;

		DPrepDestinationForRead VectorPrepDelegate;
		DPrepDestinationForRead PointerPrepDelegate;
		VoidFactory FactoryDelegate;
};

} // namespace Serialization
} // namespace IP