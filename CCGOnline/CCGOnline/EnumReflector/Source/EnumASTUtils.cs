﻿/**********************************************************************************************************************

	EnumASTUtils.cs
		A set of utility functions for processing the abstract syntax tree as generated by the enum grammar parser
		Creates and fills out a CEnumRecord object.

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

using System;
using System.Globalization;
using System.Text;

using Antlr.Runtime;
using Antlr.Runtime.Misc;
using Antlr.Runtime.Tree;

namespace EnumReflector
{
	public static class CEnumASTUtils
	{
		static CEnumASTUtils() {}

		// Methods
		// Public interface
		static public CEnumRecord Parse_Enum_Definition( string parse_string, string file_name_with_path )
		{
			var char_stream = new ANTLRStringStream( parse_string );

			EnumReflectorLexer lexer = new EnumReflectorLexer( char_stream );

			CommonTokenStream tokens = new CommonTokenStream();
			tokens.TokenSource = lexer;

			EnumReflectorParser parser = new EnumReflectorParser( tokens );
			AstParserRuleReturnScope< object, IToken > result = parser.parse();

			object tree = result.Tree;
			ITreeAdaptor tree_adapter = parser.TreeAdaptor;

			return Walk_Enum_AST( tree, tree_adapter, file_name_with_path );
		}

		// Private interface
		static private string Concatenate_Depth2_Subtree_Name( ITreeAdaptor tree_adapter, object node )
		{
			StringBuilder builder = new StringBuilder();

			builder.Append( tree_adapter.GetText( node ) );

			for ( int i = 0; i < tree_adapter.GetChildCount( node ); i++ )
			{
				object child_node = tree_adapter.GetChild( node, i );
				builder.Append( tree_adapter.GetText( child_node ) );
			}

			return builder.ToString();
		}

		static private CEnumRecord Walk_Enum_AST( object root_node, ITreeAdaptor tree_adapter, string file_name_with_path )
		{
			string name_space = String.Empty;
			string extension_enum_name = String.Empty;

			if ( tree_adapter.GetChildCount( root_node ) != 3 )
			{
				throw new Exception( "Parse Error: Enum parse tree root does not have three children" );
			}

			EEnumFlags flags = EEnumFlags.None;
			object begin_settings_node = tree_adapter.GetChild( root_node, 0 );

			for ( int i = 0; i < tree_adapter.GetChildCount( begin_settings_node ); i++ )
			{
				object setting_node = tree_adapter.GetChild( begin_settings_node, i );
				switch ( tree_adapter.GetToken( setting_node ).Type )
				{ 
					case EnumReflectorParser.BITFIELD:
						flags |= EEnumFlags.IsBitfield;
						break;

					case EnumReflectorParser.EXTENDS:
					{
						if ( tree_adapter.GetChildCount( setting_node ) != 1 )
						{
							throw new Exception( "Parse Error: extension clause does not have a single child qualified identifier node" );
						}

						object extension_name_node = tree_adapter.GetChild( setting_node, 0 );
						extension_enum_name = Concatenate_Depth2_Subtree_Name( tree_adapter, extension_name_node );
						break;
					}

					default:
						break;
				}
			}

			object enum_entry_point_node = tree_adapter.GetChild( root_node, 1 );
			object enum_definition_node = enum_entry_point_node;
			if ( tree_adapter.GetToken( enum_entry_point_node ).Type == EnumReflectorParser.NAMESPACE )
			{
				object namespace_name_node = tree_adapter.GetChild( enum_entry_point_node, 0 );
				name_space = Concatenate_Depth2_Subtree_Name( tree_adapter, namespace_name_node ); 

				enum_definition_node = tree_adapter.GetChild( enum_entry_point_node, 1 );
			}

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

			CLogInterface.Write_Line( "Processing Enum " + name_space + "::" + enum_name + ", Flags = " + flags.ToString() );

			CEnumRecord enum_record = new CEnumRecord( enum_name, file_name_with_path, name_space, extension_enum_name, flags );
			enum_record.HeaderFileID = CEnumReflector.HeaderFileTracker.Get_Header_ID_By_File_Path( file_name_with_path ).ID;

			int current_value = 0;
			object enum_entry_list_node = tree_adapter.GetChild( enum_definition_node, 1 );
			for ( int i = 0; i < tree_adapter.GetChildCount( enum_entry_list_node ); i++ )
			{
				object enum_entry_node = tree_adapter.GetChild( enum_entry_list_node, i );

				string qualified_entry_name = tree_adapter.GetText( enum_entry_node );
				if ( name_space.Length > 0 )
				{
					qualified_entry_name = name_space + "::" + qualified_entry_name;
				}

				string entry_conversion_name = "";
				bool can_bind_value = extension_enum_name.Length == 0;
				bool bound_value = false;
				string bound_name = "";

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
						bound_value = true;
					}
					else if ( sub_node_token_type == EnumReflectorParser.POSITIVE_INTEGER10 )
					{
						current_value = int.Parse( tree_adapter.GetText( enum_entry_sub_node ) );
						bound_value = true;
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
						bound_value = true;
					}
					else if ( sub_node_token_type == EnumReflectorParser.ID )
					{
						bound_name = Concatenate_Depth2_Subtree_Name( tree_adapter, enum_entry_sub_node );
					}
					else
					{
						throw new Exception( "Invalid node type embedded within an enum entry" );
					}

					if ( !can_bind_value && bound_value )
					{
						throw new Exception( "Extension enum " + enum_record.FullName + " has an illegally bound entry: " + qualified_entry_name );
					}
				}

				if ( can_bind_value )
				{
					CLogInterface.Write_Line( "Enum " + enum_record.FullName + ": Register bound entry " + entry_conversion_name + " with value " + current_value.ToString() );
					enum_record.Add_Bound_Entry( qualified_entry_name, entry_conversion_name, (ulong)current_value );
				}
				else
				{
					CLogInterface.Write_Line( "Enum " + enum_record.FullName + ": Register unbound entry " + entry_conversion_name );
					enum_record.Add_Unbound_Entry( qualified_entry_name, entry_conversion_name, bound_name );						
				}

				current_value++;
			}

			return enum_record;
		}
	}
}