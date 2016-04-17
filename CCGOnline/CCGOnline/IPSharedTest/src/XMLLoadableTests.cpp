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

#include <IPSharedTest/XMLLoadableTests.h>

#include <IPShared/Serialization/XML/XMLLoadableTable.h>
#include <IPShared/Serialization/SerializationRegistrar.h>
#include <IPShared/Serialization/XML/PrimitiveXMLSerializers.h>
#include <gtest/gtest.h>
#include <pugixml/pugixml.h>

using namespace IP::Serialization;
using namespace IP::Serialization::XML;

class XMLLoadableTests : public testing::Test 
{
	protected:  

		virtual void SetUp( void )
		{
		}

		virtual void TearDown( void )
		{
		}

	private:

};


struct SUnorderedCompositeXMLTest
{
	public:

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( SUnorderedCompositeXMLTest );

			REGISTER_MEMBER_BINDING( "UShort", &SUnorderedCompositeXMLTest::UShort );
			REGISTER_MEMBER_BINDING( "BigintPointer", &SUnorderedCompositeXMLTest::Bigint );
			REGISTER_MEMBER_BINDING( "String", &SUnorderedCompositeXMLTest::String );
			REGISTER_MEMBER_BINDING( "Float", &SUnorderedCompositeXMLTest::Float );
			REGISTER_MEMBER_BINDING( "Bool", &SUnorderedCompositeXMLTest::Bool );

			END_TYPE_DEFINITION( SUnorderedCompositeXMLTest );
		}

		uint16_t UShort;
		uint64_t *Bigint;
		IP::String String;
		float Float;
		bool Bool;
};

TEST_F( XMLLoadableTests, Unordered_Composite_Serializer )
{
	SUnorderedCompositeXMLTest::Register_Type_Definition();
	CSerializationRegistrar::Finalize();

	IXMLSerializer *serializer = CSerializationRegistrar::Get_XML_Serializer< SUnorderedCompositeXMLTest >();

	SUnorderedCompositeXMLTest test;
	IP::String xml_blob( "<Test><Bool>true</Bool><BigintPointer>40960</BigintPointer><String>google</String><UShort>65535</UShort><Float>5.5</Float></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	serializer->Load_From_XML( doc.first_child(), &test );

	ASSERT_TRUE( test.UShort == 65535 );
	ASSERT_TRUE( *test.Bigint == 40960 );
	ASSERT_TRUE( test.String == "google" );
	ASSERT_TRUE( test.Float == 5.5 );
	ASSERT_TRUE( test.Bool == true );

	IP::String xml_blob2( "<Test><Bool>false</Bool><Float>3.14</Float></Test>" );

	pugi::xml_document doc2;
	doc2.load( xml_blob2.c_str() );

	serializer->Load_From_XML( doc2.first_child(), &test );

	ASSERT_TRUE( test.UShort == 65535 );
	ASSERT_TRUE( *test.Bigint == 40960 );
	ASSERT_TRUE( test.String == "google" );
	ASSERT_TRUE( test.Float == 3.14f );
	ASSERT_TRUE( test.Bool == false );
}

struct SInnerCompositeXMLTest1
{
	public:

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( SInnerCompositeXMLTest1 );

			REGISTER_MEMBER_BINDING( "UShort", &SInnerCompositeXMLTest1::UShort );
			REGISTER_MEMBER_BINDING( "Bigint", &SInnerCompositeXMLTest1::Bigint );

			END_TYPE_DEFINITION( SInnerCompositeXMLTest1 );
		}

		uint16_t UShort;
		uint64_t Bigint;
};

struct SInnerCompositeXMLTest2
{
	public:

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( SInnerCompositeXMLTest2 );

			REGISTER_MEMBER_BINDING( "String", &SInnerCompositeXMLTest2::String );
			REGISTER_MEMBER_BINDING( "Float", &SInnerCompositeXMLTest2::Float );

			END_TYPE_DEFINITION( SInnerCompositeXMLTest2 );
		}

		IP::String String;
		float Float;
};

struct SOuterCompositeXMLTest
{
	public:

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( SOuterCompositeXMLTest );

			REGISTER_MEMBER_BINDING( "Inner1", &SOuterCompositeXMLTest::Inner1 );
			REGISTER_MEMBER_BINDING( "Double", &SOuterCompositeXMLTest::Double );
			REGISTER_MEMBER_BINDING( "Inner2", &SOuterCompositeXMLTest::Inner2 );

			END_TYPE_DEFINITION( SOuterCompositeXMLTest );
		}

		SInnerCompositeXMLTest1 *Inner1;
		double Double;
		SInnerCompositeXMLTest2 Inner2;
};


TEST_F( XMLLoadableTests, Nested_Composite_Serializer )
{
	SInnerCompositeXMLTest1::Register_Type_Definition();
	SInnerCompositeXMLTest2::Register_Type_Definition();
	SOuterCompositeXMLTest::Register_Type_Definition();
	CSerializationRegistrar::Finalize();

	SOuterCompositeXMLTest test;
	IP::String xml_blob( "<Test><Inner1><UShort>3</UShort><Bigint>15</Bigint></Inner1><Double>1.0</Double><Inner2><String>Hey</String><Float>2.0</Float></Inner2></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CSerializationRegistrar::Get_XML_Serializer< SOuterCompositeXMLTest >();
	serializer->Load_From_XML( doc.first_child(), &test );

	ASSERT_TRUE( test.Inner1->UShort == 3 );
	ASSERT_TRUE( test.Inner1->Bigint == 15 );
	ASSERT_TRUE( test.Double == 1.0 );
	ASSERT_TRUE( test.Inner2.String == "Hey" );
	ASSERT_TRUE( test.Inner2.Float == 2.0f );
}


class CBaseXMLTest
{
	public:

		CBaseXMLTest( void ) :
			BaseString( "" ),
			BaseInt32( 0 )
		{}

		virtual ~CBaseXMLTest() {}

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( CBaseXMLTest );

			REGISTER_MEMBER_BINDING( "BaseString", &CBaseXMLTest::BaseString );
			REGISTER_MEMBER_BINDING( "BaseInt32", &CBaseXMLTest::BaseInt32 );

			END_TYPE_DEFINITION( CBaseXMLTest );
		}

		const IP::String &Get_Base_String( void ) const { return BaseString; }
		int32_t Get_Base_Int32( void ) const { return BaseInt32; }

	private:

		IP::String BaseString;
		int32_t BaseInt32;
};

class CDerivedXMLTest : public CBaseXMLTest
{
	public:

		using BASECLASS = CBaseXMLTest;

		CDerivedXMLTest( void ) :
			BASECLASS(),
			DerivedString( "" ),
			DerivedInt32( 0 )
		{}

		static void Register_Type_Definition( void )
		{
			BEGIN_DERIVED_TYPE_DEFINITION( CDerivedXMLTest, BASECLASS );

			REGISTER_MEMBER_BINDING( "DerivedString", &CDerivedXMLTest::DerivedString );
			REGISTER_MEMBER_BINDING( "DerivedInt32", &CDerivedXMLTest::DerivedInt32 );

			END_TYPE_DEFINITION( CDerivedXMLTest );
		}

		const IP::String &Get_Derived_String( void ) const { return DerivedString; }
		int32_t Get_Derived_Int32( void ) const { return DerivedInt32; }

	private:

		IP::String DerivedString;
		int32_t DerivedInt32;
};

TEST_F( XMLLoadableTests, Derived_Serializer )
{
	CBaseXMLTest::Register_Type_Definition();
	CDerivedXMLTest::Register_Type_Definition();
	CSerializationRegistrar::Finalize();

	CDerivedXMLTest test;
	IP::String xml_blob( "<Test><BaseString>base</BaseString><BaseInt32>-1</BaseInt32><DerivedString>derived</DerivedString><DerivedInt32>1</DerivedInt32></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CSerializationRegistrar::Get_XML_Serializer< CDerivedXMLTest >();
	serializer->Load_From_XML( doc.first_child(), &test );

	ASSERT_TRUE( test.Get_Base_String() == "base" );
	ASSERT_TRUE( test.Get_Base_Int32() == -1 );
	ASSERT_TRUE( test.Get_Derived_String() == "derived" );
	ASSERT_TRUE( test.Get_Derived_Int32() == 1 );
}

class CPrimitiveVectorXMLTest
{
	public:

		CPrimitiveVectorXMLTest( void ) :
			Strings(),
			Integers()
		{}

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( CPrimitiveVectorXMLTest );

			REGISTER_MEMBER_BINDING( "Strings", &CPrimitiveVectorXMLTest::Strings );
			REGISTER_MEMBER_BINDING( "Integers", &CPrimitiveVectorXMLTest::Integers );

			END_TYPE_DEFINITION( CPrimitiveVectorXMLTest );
		}

		const IP::Vector< IP::String > &Get_Strings( void ) const { return Strings; }
		const IP::Vector< int32_t * > &Get_Integers( void ) const { return Integers; }

	private:

		IP::Vector< IP::String > Strings;
		IP::Vector< int32_t * > Integers;
};

TEST_F( XMLLoadableTests, Primitive_Vector_Serializers )
{
	CPrimitiveVectorXMLTest::Register_Type_Definition();
	CSerializationRegistrar::Finalize();

	CPrimitiveVectorXMLTest test;
	IP::String xml_blob( "<Test><Strings><Entry>string1</Entry><Entry>string2</Entry></Strings><Integers><Entry>1</Entry><Entry>2</Entry></Integers></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CSerializationRegistrar::Get_XML_Serializer< CPrimitiveVectorXMLTest >();
	serializer->Load_From_XML( doc.first_child(), &test );

	ASSERT_TRUE( test.Get_Strings().size() == 2 );
	ASSERT_TRUE( test.Get_Strings()[ 0 ] == "string1" );
	ASSERT_TRUE( test.Get_Strings()[ 1 ] == "string2" );
	ASSERT_TRUE( test.Get_Integers().size() == 2 );
	ASSERT_TRUE( *test.Get_Integers()[ 0 ] == 1 );
	ASSERT_TRUE( *test.Get_Integers()[ 1 ] == 2 );
}


class CVectorEntry
{
	public:

		CVectorEntry( void ) :
			String( "" ),
			Integer( 0 )
		{}

		virtual ~CVectorEntry() {}

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( CVectorEntry );

			REGISTER_MEMBER_BINDING( "String", &CVectorEntry::String );
			REGISTER_MEMBER_BINDING( "Integer", &CVectorEntry::Integer );

			END_TYPE_DEFINITION( CVectorEntry );
		}

		const IP::String &Get_String( void ) const { return String; }
		int32_t Get_Integer( void ) const { return Integer; }

	private:

		IP::String String;
		int32_t Integer;
};

class CDerivedVectorEntry : public CVectorEntry
{
	public:

		using BASECLASS = CVectorEntry;

		CDerivedVectorEntry( void ) :
			BASECLASS(),
			Bool( false )
		{}

		static void Register_Type_Definition( void )
		{
			BEGIN_DERIVED_TYPE_DEFINITION(CDerivedVectorEntry, BASECLASS);

			REGISTER_MEMBER_BINDING( "Bool", &CDerivedVectorEntry::Bool );

			END_TYPE_DEFINITION( CDerivedVectorEntry );
		}

		bool Get_Bool( void ) const { return Bool; }

	private:

		bool Bool;
};


class CVectorXMLTest
{
	public:

		CVectorXMLTest( void ) :
			Entries(),
			EntryPointers()
		{}

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( CVectorXMLTest );

			REGISTER_MEMBER_BINDING( "Entries", &CVectorXMLTest::Entries );
			REGISTER_MEMBER_BINDING( "EntryPointers", &CVectorXMLTest::EntryPointers );

			END_TYPE_DEFINITION( CVectorXMLTest );
		}

		const IP::Vector< CVectorEntry > &Get_Entries( void ) const { return Entries; }
		const IP::Vector< CVectorEntry * > &Get_Entry_Pointers( void ) const { return EntryPointers; }

	private:

		IP::Vector< CVectorEntry > Entries;
		IP::Vector< CVectorEntry * > EntryPointers;

};

TEST_F( XMLLoadableTests, Compound_Vector_Serializers )
{
	CVectorEntry::Register_Type_Definition();
	CDerivedVectorEntry::Register_Type_Definition();
	CVectorXMLTest::Register_Type_Definition();
	CSerializationRegistrar::Finalize();

	CVectorXMLTest test;
	IP::String xml_blob( "<Test><Entries><Entry><String>string1</String><Integer>1</Integer></Entry></Entries><EntryPointers><Entry><String>DerivedString</String><Integer>42</Integer><Bool>true</Bool></Entry></EntryPointers></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CSerializationRegistrar::Get_XML_Serializer< CVectorXMLTest >();
	serializer->Load_From_XML( doc.first_child(), &test );

	ASSERT_TRUE( test.Get_Entries().size() == 1 );
	ASSERT_TRUE( test.Get_Entries()[ 0 ].Get_String() == "string1" );
	ASSERT_TRUE( test.Get_Entries()[ 0 ].Get_Integer() == 1 );

	ASSERT_TRUE( test.Get_Entry_Pointers().size() == 1 );

	CDerivedVectorEntry *entry = static_cast< CDerivedVectorEntry * >( test.Get_Entry_Pointers()[ 0 ] );
	ASSERT_TRUE( entry->Get_String() == "DerivedString" );
	ASSERT_TRUE( entry->Get_Integer() == 42 );
	ASSERT_TRUE( entry->Get_Bool() == true );
}


class CPolyBase
{
	public:

		CPolyBase( void ) :
			String( "" )
		{}

		virtual ~CPolyBase() {}

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( CPolyBase );

			REGISTER_MEMBER_BINDING( "String", &CPolyBase::String );

			END_TYPE_DEFINITION( CPolyBase );
		}

		const IP::String &Get_String( void ) const { return String; }

	private:

		IP::String String;
};

class CPolyDerived1 : public CPolyBase
{
	public:

		using BASECLASS = CPolyBase;

		CPolyDerived1( void ) :
			BASECLASS(),
			Bool( false )
		{}

		static void Register_Type_Definition( void )
		{
			BEGIN_DERIVED_TYPE_DEFINITION( CPolyDerived1, CPolyBase );

			REGISTER_MEMBER_BINDING( "Bool", &CPolyDerived1::Bool );

			END_TYPE_DEFINITION( CPolyDerived1 );

			REGISTER_POLYMORPHIC_ENUM_ENTRY( PSTT_CLASS1, CPolyDerived1 );
		}

		bool Get_Bool( void ) const { return Bool; }

	private:

		bool Bool;
};

class CPolyDerived2 : public CPolyBase
{
	public:

		using BASECLASS = CPolyBase;

		CPolyDerived2( void ) :
			BASECLASS(),
			Integer( 0 )
		{}

		static void Register_Type_Definition( void )
		{
			BEGIN_DERIVED_TYPE_DEFINITION( CPolyDerived2, CPolyBase );

			REGISTER_MEMBER_BINDING( "Integer", &CPolyDerived2::Integer );

			END_TYPE_DEFINITION( CPolyDerived2 );

			REGISTER_POLYMORPHIC_ENUM_ENTRY( PSTT_CLASS2, CPolyDerived2 );
		}

		int32_t Get_Integer( void ) const { return Integer; }

	private:

		int32_t Integer;
};

class CPolyVectorTest
{
	public:

		CPolyVectorTest( void ) :
			Entries()
		{}

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( CPolyVectorTest );

			REGISTER_MEMBER_BINDING( "Entries", &CPolyVectorTest::Entries );

			END_TYPE_DEFINITION( CPolyVectorTest );
		}

		const IP::Vector< CPolyBase * > &Get_Entries( void ) const { return Entries; }

	private:

		IP::Vector< CPolyBase * > Entries;

};

TEST_F( XMLLoadableTests, Polymorphic_Serializer )
{
	CPolyBase::Register_Type_Definition();
	CPolyDerived1::Register_Type_Definition();
	CPolyDerived2::Register_Type_Definition();
	CPolyVectorTest::Register_Type_Definition();
	CSerializationRegistrar::Finalize();

	CPolyVectorTest test;
	IP::String xml_blob( "<Test><Entries><Entry Type=\"CPolyDerived1\"><String>poly1</String><Bool>1</Bool></Entry><Entry Type=\"CPolyDerived2\"><String>poly2</String><Integer>42</Integer></Entry></Entries></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CSerializationRegistrar::Get_XML_Serializer< CPolyVectorTest >();
	serializer->Load_From_XML( doc.first_child(), &test );

	ASSERT_TRUE( test.Get_Entries().size() == 2 );

	CPolyDerived1 *poly1 = static_cast< CPolyDerived1 * >( test.Get_Entries()[ 0 ] );
	ASSERT_TRUE( poly1->Get_String() == "poly1" );
	ASSERT_TRUE( poly1->Get_Bool() == true );	

	CPolyDerived2 *poly2 = static_cast< CPolyDerived2 * >( test.Get_Entries()[ 1 ] );
	ASSERT_TRUE( poly2->Get_String() == "poly2" );
	ASSERT_TRUE( poly2->Get_Integer() == 42 );	
}


class CTableTest
{
	public:

		CTableTest( void ) :
			Name( "" ),
			HitPoints( 0 ),
			Class( ETTC_INVALID )
		{}

		virtual ~CTableTest() {}

		static void Register_Type_Definition( void )
		{
			BEGIN_ROOT_TYPE_DEFINITION( CTableTest );

			REGISTER_MEMBER_BINDING( "Name", &CTableTest::Name );
			REGISTER_MEMBER_BINDING( "HitPoints", &CTableTest::HitPoints );
			REGISTER_MEMBER_BINDING( "Class", &CTableTest::Class );

			END_TYPE_DEFINITION( CTableTest );
		}

		const IP::String &Get_Name( void ) const { return Name; }
		uint32_t Get_Hit_Points( void ) const { return HitPoints; }
		ETableTestClass Get_Class( void ) const { return Class; }

	private:

		IP::String Name;

		uint32_t HitPoints;

		ETableTestClass Class;
};


TEST_F( XMLLoadableTests, Loadable_Table )
{
	REGISTER_ENUM_SERIALIZER(ETableTestClass);

	CTableTest::Register_Type_Definition();
	CSerializationRegistrar::Finalize();

	CXMLLoadableTable< IP::String, CTableTest > loadable_table( &CTableTest::Get_Name );

	IP::String xml_blob( "<Objects><Object><Name>Bret</Name><HitPoints>5</HitPoints><Class>Janitor</Class></Object><Object><Name>Peti</Name><HitPoints>50</HitPoints><Class>Berserker</Class></Object></Objects>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	loadable_table.Load( doc );

	const CTableTest *test1 = loadable_table.Get_Object( "Bret" );
	ASSERT_TRUE( test1->Get_Name() == "Bret" );
	ASSERT_TRUE( test1->Get_Hit_Points() == 5 );
	ASSERT_TRUE( test1->Get_Class() == ETTC_JANITOR );

	const CTableTest *test2 = loadable_table.Get_Object( "Peti" );
	ASSERT_TRUE( test2->Get_Name() == "Peti" );
	ASSERT_TRUE( test2->Get_Hit_Points() == 50 );
	ASSERT_TRUE( test2->Get_Class() == ETTC_BERSERKER );

}


