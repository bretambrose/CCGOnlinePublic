using System;
using System.Globalization;

using Antlr.Runtime;
using Antlr.Runtime.Misc;
using Antlr.Runtime.Tree;

namespace EnumReflector
{
	public static class CEnumASTUtils
	{
		static CEnumASTUtils() {}

		static public CEnumRecord Parse_Enum_Definition( string parse_string, string header_file_name )
		{
			var char_stream = new ANTLRStringStream( parse_string );

			EnumReflectorLexer lexer = new EnumReflectorLexer( char_stream );

			CommonTokenStream tokens = new CommonTokenStream();
			tokens.TokenSource = lexer;

			EnumReflectorParser parser = new EnumReflectorParser( tokens );
			AstParserRuleReturnScope< object, IToken > result = parser.parse();

			object tree = result.Tree;
			ITreeAdaptor tree_adapter = parser.TreeAdaptor;

			return Walk_Enum_AST( tree, tree_adapter, header_file_name );
		}

		static private CEnumRecord Walk_Enum_AST( object root_node, ITreeAdaptor tree_adapter, string header_file_name )
		{
			if ( tree_adapter.GetChildCount( root_node ) != 3 )
			{
				throw new Exception( "Parse Error: Enum parse tree root does not have three children" );
			}

			EEnumFlags flags = EEnumFlags.None;
			object begin_settings_node = tree_adapter.GetChild( root_node, 0 );

			for ( int i = 0; i < tree_adapter.GetChildCount( begin_settings_node ); i++ )
			{
				object setting_node = tree_adapter.GetChild( begin_settings_node, i );
				if ( tree_adapter.GetToken( setting_node ).Type == EnumReflectorParser.BITFIELD )
				{
					flags |= EEnumFlags.IsBitfield;
				}
			}

			object enum_definition_node = tree_adapter.GetChild( root_node, 1 );
			if ( tree_adapter.GetChildCount( enum_definition_node ) != 2 )
			{
				throw new Exception( "Parse Error: Enum definition node does not have two children" );
			}

			object enum_name_node = tree_adapter.GetChild( enum_definition_node, 0 );
			if ( tree_adapter.GetToken( enum_name_node ).Type != EnumReflectorParser.ID )
			{
				throw new Exception( "Parse Error: Enum definition missing name" );
			}

			string enum_name = tree_adapter.GetText( enum_name_node );
			Console.WriteLine( "Processing Enum " + enum_name + ", Flags = " + flags.ToString() );

			CEnumRecord enum_record = new CEnumRecord( enum_name, header_file_name, flags );

			int current_value = 0;
			object enum_entry_list_node = tree_adapter.GetChild( enum_definition_node, 1 );
			for ( int i = 0; i < tree_adapter.GetChildCount( enum_entry_list_node ); i++ )
			{
				object enum_entry_node = tree_adapter.GetChild( enum_entry_list_node, i );

				string entry_name = tree_adapter.GetText( enum_entry_node );
				string entry_conversion_name = null;

				for ( int j = 0; j < tree_adapter.GetChildCount( enum_entry_node ); j++ )
				{
					object enum_entry_sub_node = tree_adapter.GetChild( enum_entry_node, j );
					int sub_node_token_type = tree_adapter.GetToken( enum_entry_sub_node ).Type;

					if ( sub_node_token_type == EnumReflectorParser.ENUM_ENTRY )
					{
						if ( tree_adapter.GetChildCount( enum_entry_sub_node ) != 1 )
						{
							throw new Exception( "Enum entry conversion tag has invalid child count" );
						}

						string quoted_string = tree_adapter.GetText( tree_adapter.GetChild( enum_entry_sub_node, 0 ) );
						entry_conversion_name = quoted_string.Substring( 1, quoted_string.Length - 2 );
					}
					else if ( sub_node_token_type == EnumReflectorParser.POSITIVE_INTEGER16 )
					{
						current_value = int.Parse( tree_adapter.GetText( enum_entry_sub_node ).Substring( 2 ), NumberStyles.AllowHexSpecifier );
					}
					else if ( sub_node_token_type == EnumReflectorParser.POSITIVE_INTEGER10 )
					{
						current_value = int.Parse( tree_adapter.GetText( enum_entry_sub_node ) );
					}
					else if ( sub_node_token_type == EnumReflectorParser.LEFT_SHIFT )
					{
						if ( tree_adapter.GetChildCount( enum_entry_sub_node ) != 2 )
						{
							throw new Exception( "Enum entry with left shift expression has invalid expression" );
						}

						int val1 = int.Parse( tree_adapter.GetText( tree_adapter.GetChild( enum_entry_sub_node, 0 ) ) );
						int val2 = int.Parse( tree_adapter.GetText( tree_adapter.GetChild( enum_entry_sub_node, 1 ) ) );

						if ( val1 != 1 )
						{
							throw new Exception( "Left shift expression not using 1 as the shift basis" );
						}

						if ( val2 > 31 )
						{
							throw new Exception( "Left shift integer constant has shift value greater than 31" );
						}

						current_value = val1 << val2;
					}
					else
					{
						throw new Exception( "Invalid node type embedded within an enum entry" );
					}
				}

				if ( entry_conversion_name != null )
				{
					Console.WriteLine( "Enum " + enum_name + ": Register entry " + entry_conversion_name + " with value " + current_value.ToString() );
					enum_record.Add_Entry( (ulong)current_value, entry_conversion_name );
				}

				current_value++;
			}

			return enum_record;
		}

		public static void Parsing_Test()
		{
			try
			{
				string test_input = @"//:EnumBegin()
	enum EParserTest {
		EPT_INVALID = 0,

		EPT_VAL1,			//:EnumEntry( ""Val1"" )
		EPT_VAL2 = 3,		//:EnumEntry( ""Val2"" )
		EPT_VAL3				//:EnumEntry( ""Val3"" )
	};
	//:EnumEnd
	";

				Parse_Enum_Definition( test_input, "" );
			}
			catch ( Exception e )
			{
				Console.WriteLine( e.ToString() );
			}
		}

		public static void Parsing_Test2()
		{
			try
			{
				string test_input = @"//:EnumBegin( BITFIELD )
	enum EBitfieldTest {
		EBT_INVALID = 0,

		EBT_VAL1 = 1 << 0,		//:EnumEntry( ""Val1"" )
		EBT_VAL2 = 0x02,			//:EnumEntry( ""Val2"" )
		EBT_VAL3 = 1 << 2			//:EnumEntry( ""Val3"" )
	};
	//:EnumEnd
	";
				Parse_Enum_Definition( test_input, "" );
			}
			catch ( Exception e )
			{
				Console.WriteLine( e.ToString() );
			}
		} 
	}
}