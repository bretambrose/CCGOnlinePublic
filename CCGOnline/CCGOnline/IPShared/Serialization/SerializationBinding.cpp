#include "stdafx.h"

#include "IPShared/Serialization/SerializationBinding.h"

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

