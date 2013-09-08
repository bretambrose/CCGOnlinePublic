/**********************************************************************************************************************

	XMLLoadableTest.cpp
		defines unit tests for xml loadable related functionality

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

#include "XMLLoadableTests.h"
#include "IPShared/XML/XMLLoadableTable.h"
#include "IPShared/XML/XMLSerializationRegistrar.h"
#include "IPShared/XML/PrimitiveXMLSerializers.h"

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

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"UShort", &SUnorderedCompositeXMLTest::UShort );
			serializer->Add( L"Bigint", &SUnorderedCompositeXMLTest::Bigint );
			serializer->Add( L"String", &SUnorderedCompositeXMLTest::String );
			serializer->Add( L"Float", &SUnorderedCompositeXMLTest::Float );
			serializer->Add( L"Bool", &SUnorderedCompositeXMLTest::Bool );

			return serializer;
		}

		uint16 UShort;
		uint64 *Bigint;
		std::string String;
		float Float;
		bool Bool;
};

TEST_F( XMLLoadableTests, Unordered_Composite_Serializer )
{
	CXMLSerializationRegistrar::Register_Serializer< SUnorderedCompositeXMLTest >( SUnorderedCompositeXMLTest::Create_Serializer );

	IXMLSerializer *serializer = CXMLSerializationRegistrar::Create_Serializer< SUnorderedCompositeXMLTest >();

	SUnorderedCompositeXMLTest test;
	std::wstring xml_blob( L"<Test><Bool>true</Bool><Bigint>40960</Bigint><String>google</String><UShort>65535</UShort><Float>5.5</Float></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	serializer->Load_From_XML( doc.first_child(), &test );

	ASSERT_TRUE( test.UShort == 65535 );
	ASSERT_TRUE( *test.Bigint == 40960 );
	ASSERT_TRUE( test.String == "google" );
	ASSERT_TRUE( test.Float == 5.5 );
	ASSERT_TRUE( test.Bool == true );

	std::wstring xml_blob2( L"<Test><Bool>false</Bool><Float>3.14</Float></Test>" );

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

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"UShort", &SInnerCompositeXMLTest1::UShort );
			serializer->Add( L"Bigint", &SInnerCompositeXMLTest1::Bigint );

			return serializer;
		}

		uint16 UShort;
		uint64 Bigint;
};

struct SInnerCompositeXMLTest2
{
	public:

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"String", &SInnerCompositeXMLTest2::String );
			serializer->Add( L"Float", &SInnerCompositeXMLTest2::Float );

			return serializer;
		}

		std::string String;
		float Float;
};

struct SOuterCompositeXMLTest
{
	public:

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"Inner1", &SOuterCompositeXMLTest::Inner1 );
			serializer->Add( L"Double", &SOuterCompositeXMLTest::Double );
			serializer->Add( L"Inner2", &SOuterCompositeXMLTest::Inner2 );

			return serializer;
		}

		SInnerCompositeXMLTest1 *Inner1;
		double Double;
		SInnerCompositeXMLTest2 Inner2;
};


TEST_F( XMLLoadableTests, Nested_Composite_Serializer )
{
	CXMLSerializationRegistrar::Register_Serializer< SInnerCompositeXMLTest1 >( SInnerCompositeXMLTest1::Create_Serializer );
	CXMLSerializationRegistrar::Register_Serializer< SInnerCompositeXMLTest2 >( SInnerCompositeXMLTest2::Create_Serializer );
	CXMLSerializationRegistrar::Register_Serializer< SOuterCompositeXMLTest >( SOuterCompositeXMLTest::Create_Serializer );

	SOuterCompositeXMLTest test;
	std::wstring xml_blob( L"<Test><Inner1><UShort>3</UShort><Bigint>15</Bigint></Inner1><Double>1.0</Double><Inner2><String>Hey</String><Float>2.0</Float></Inner2></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CXMLSerializationRegistrar::Create_Serializer< SOuterCompositeXMLTest >();
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

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"BaseString", &CBaseXMLTest::BaseString );
			serializer->Add( L"BaseInt32", &CBaseXMLTest::BaseInt32 );

			return serializer;
		}

		const std::string &Get_Base_String( void ) const { return BaseString; }
		int32 Get_Base_Int32( void ) const { return BaseInt32; }

	private:

		std::string BaseString;
		int32 BaseInt32;
};

class CDerivedXMLTest : public CBaseXMLTest
{
	public:

		typedef CBaseXMLTest BASECLASS;

		CDerivedXMLTest( void ) :
			BASECLASS(),
			DerivedString( "" ),
			DerivedInt32( 0 )
		{}

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = static_cast< CCompositeXMLSerializer * >( CXMLSerializationRegistrar::Create_Serializer< BASECLASS >() );

			serializer->Add( L"DerivedString", &CDerivedXMLTest::DerivedString );
			serializer->Add( L"DerivedInt32", &CDerivedXMLTest::DerivedInt32 );

			return serializer;
		}

		const std::string &Get_Derived_String( void ) const { return DerivedString; }
		int32 Get_Derived_Int32( void ) const { return DerivedInt32; }

	private:

		std::string DerivedString;
		int32 DerivedInt32;
};

TEST_F( XMLLoadableTests, Derived_Serializer )
{
	CXMLSerializationRegistrar::Register_Serializer< CBaseXMLTest >( CBaseXMLTest::Create_Serializer );
	CXMLSerializationRegistrar::Register_Serializer< CDerivedXMLTest >( CDerivedXMLTest::Create_Serializer );

	CDerivedXMLTest test;
	std::wstring xml_blob( L"<Test><BaseString>base</BaseString><BaseInt32>-1</BaseInt32><DerivedString>derived</DerivedString><DerivedInt32>1</DerivedInt32></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CXMLSerializationRegistrar::Create_Serializer< CDerivedXMLTest >();
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

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"Strings", &CPrimitiveVectorXMLTest::Strings, new CVectorXMLSerializer< std::string > );
			serializer->Add( L"Integers", &CPrimitiveVectorXMLTest::Integers, new CVectorXMLSerializer< int32 * > );

			return serializer;
		}

		const std::vector< std::string > &Get_Strings( void ) const { return Strings; }
		const std::vector< int32 * > &Get_Integers( void ) const { return Integers; }

	private:

		std::vector< std::string > Strings;
		std::vector< int32 * > Integers;
};

TEST_F( XMLLoadableTests, Primitive_Vector_Serializers )
{
	CXMLSerializationRegistrar::Register_Serializer< CPrimitiveVectorXMLTest >( CPrimitiveVectorXMLTest::Create_Serializer );

	CPrimitiveVectorXMLTest test;
	std::wstring xml_blob( L"<Test><Strings><Entry>string1</Entry><Entry>string2</Entry></Strings><Integers><Entry>1</Entry><Entry>2</Entry></Integers></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CXMLSerializationRegistrar::Create_Serializer< CPrimitiveVectorXMLTest >();
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

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"String", &CVectorEntry::String );
			serializer->Add( L"Integer", &CVectorEntry::Integer );

			return serializer;
		}

		const std::string &Get_String( void ) const { return String; }
		int32 Get_Integer( void ) const { return Integer; }

	private:

		std::string String;
		int32 Integer;
};

class CDerivedVectorEntry : public CVectorEntry
{
	public:

		typedef CVectorEntry BASECLASS;

		CDerivedVectorEntry( void ) :
			BASECLASS(),
			Bool( false )
		{}

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = static_cast< CCompositeXMLSerializer * >( CXMLSerializationRegistrar::Create_Serializer< BASECLASS >() );

			serializer->Add( L"Bool", &CDerivedVectorEntry::Bool );

			return serializer;
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

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"Entries", &CVectorXMLTest::Entries, new CVectorXMLSerializer< CVectorEntry > );
			serializer->Add( L"EntryPointers", &CVectorXMLTest::EntryPointers, new CVectorXMLSerializer< CVectorEntry * >( CXMLSerializationRegistrar::Create_Serializer< CDerivedVectorEntry * >() ) );

			return serializer;
		}

		const std::vector< CVectorEntry > &Get_Entries( void ) const { return Entries; }
		const std::vector< CVectorEntry * > &Get_Entry_Pointers( void ) const { return EntryPointers; }

	private:

		std::vector< CVectorEntry > Entries;
		std::vector< CVectorEntry * > EntryPointers;

};

TEST_F( XMLLoadableTests, Compound_Vector_Serializers )
{
	CXMLSerializationRegistrar::Register_Serializer< CVectorEntry >( CVectorEntry::Create_Serializer );
	CXMLSerializationRegistrar::Register_Serializer< CDerivedVectorEntry >( CDerivedVectorEntry::Create_Serializer );
	CXMLSerializationRegistrar::Register_Serializer< CVectorXMLTest >( CVectorXMLTest::Create_Serializer );

	CVectorXMLTest test;
	std::wstring xml_blob( L"<Test><Entries><Entry><String>string1</String><Integer>1</Integer></Entry></Entries><EntryPointers><Entry><String>DerivedString</String><Integer>42</Integer><Bool>true</Bool></Entry></EntryPointers></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CXMLSerializationRegistrar::Create_Serializer< CVectorXMLTest >();
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

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"String", &CPolyBase::String );

			return serializer;
		}

		const std::string &Get_String( void ) const { return String; }

	private:

		std::string String;
};

class CPolyDerived1 : public CPolyBase
{
	public:

		typedef CPolyBase BASECLASS;

		CPolyDerived1( void ) :
			BASECLASS(),
			Bool( false )
		{}

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = static_cast< CCompositeXMLSerializer * >( CXMLSerializationRegistrar::Create_Serializer< BASECLASS >() );

			serializer->Add( L"Bool", &CPolyDerived1::Bool );

			return serializer;
		}

		bool Get_Bool( void ) const { return Bool; }

	private:

		bool Bool;
};

class CPolyDerived2 : public CPolyBase
{
	public:

		typedef CPolyBase BASECLASS;

		CPolyDerived2( void ) :
			BASECLASS(),
			Integer( 0 )
		{}

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = static_cast< CCompositeXMLSerializer * >( CXMLSerializationRegistrar::Create_Serializer< BASECLASS >() );

			serializer->Add( L"Integer", &CPolyDerived2::Integer );

			return serializer;
		}

		int32 Get_Integer( void ) const { return Integer; }

	private:

		int32 Integer;
};

class CPolyVectorTest
{
	public:

		CPolyVectorTest( void ) :
			Entries()
		{}

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			CEnumPolymorphicXMLSerializer< EPolySerializerTestTypes > *entry_serializer = new CEnumPolymorphicXMLSerializer< EPolySerializerTestTypes >;
			entry_serializer->Add( PSTT_CLASS1, CXMLSerializationRegistrar::Create_Serializer< CPolyDerived1 * >() );
			entry_serializer->Add( PSTT_CLASS2, CXMLSerializationRegistrar::Create_Serializer< CPolyDerived2 * >() );

			serializer->Add( L"Entries", &CPolyVectorTest::Entries, new CVectorXMLSerializer< CPolyBase * >( entry_serializer ) );

			return serializer;
		}

		const std::vector< CPolyBase * > &Get_Entries( void ) const { return Entries; }

	private:

		std::vector< CPolyBase * > Entries;

};

TEST_F( XMLLoadableTests, Polymorphic_Serializer )
{
	CXMLSerializationRegistrar::Register_Serializer< CPolyBase >( CPolyBase::Create_Serializer );
	CXMLSerializationRegistrar::Register_Serializer< CPolyDerived1 >( CPolyDerived1::Create_Serializer );
	CXMLSerializationRegistrar::Register_Serializer< CPolyDerived2 >( CPolyDerived2::Create_Serializer );
	CXMLSerializationRegistrar::Register_Serializer< CPolyVectorTest >( CPolyVectorTest::Create_Serializer );

	CPolyVectorTest test;
	std::wstring xml_blob( L"<Test><Entries><Entry Type=\"CPolyDerived1\"><String>poly1</String><Bool>1</Bool></Entry><Entry Type=\"CPolyDerived2\"><String>poly2</String><Integer>42</Integer></Entry></Entries></Test>" );

	pugi::xml_document doc;
	doc.load( xml_blob.c_str() );

	IXMLSerializer *serializer = CXMLSerializationRegistrar::Create_Serializer< CPolyVectorTest >();
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

		static IXMLSerializer *Create_Serializer( void )
		{
			CCompositeXMLSerializer *serializer = new CCompositeXMLSerializer;

			serializer->Add( L"Name", &CTableTest::Name );
			serializer->Add( L"HitPoints", &CTableTest::HitPoints );
			serializer->Add( L"Class", &CTableTest::Class );

			return serializer;
		}

		const std::string &Get_Name( void ) const { return Name; }
		uint32 Get_Hit_Points( void ) const { return HitPoints; }
		ETableTestClass Get_Class( void ) const { return Class; }

	private:

		std::string Name;

		uint32 HitPoints;

		ETableTestClass Class;
};


TEST_F( XMLLoadableTests, Loadable_Table )
{
	CXMLSerializationRegistrar::Register_Serializer< CTableTest >( CTableTest::Create_Serializer );
	CXMLSerializationRegistrar::Register_Enum_Serializer< ETableTestClass >();
	
	CXMLLoadableTable< std::string, CTableTest > loadable_table( &CTableTest::Get_Name );

	std::wstring xml_blob( L"<Objects><Object><Name>Bret</Name><HitPoints>5</HitPoints><Class>Janitor</Class></Object><Object><Name>Peti</Name><HitPoints>50</HitPoints><Class>Berserker</Class></Object></Objects>" );

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

