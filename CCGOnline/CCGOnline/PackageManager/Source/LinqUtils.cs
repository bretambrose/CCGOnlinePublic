/**********************************************************************************************************************

	[Placeholder for eventual source license]

	LinqUtils.cs
		Miscellaneous LINQ collection extension functions.  Not all used.  Pulled from Catalogue Raisonne project.

	(c) Copyright 2011, Bret Ambrose.  All rights reserved.

**********************************************************************************************************************/

using System;
using System.Collections.Generic;

namespace PackageManager
{
	public static class CLinqUtils
	{
		public static void Apply< T >( this IEnumerable< T > collection, Action< T > action )
		{
			foreach ( T element in collection )
			{
				action( element );
			}
		}

		public static void ShallowCopy< T >( this IEnumerable< T > collection, ICollection< T > destination )
		{
			collection.Apply( n => destination.Add( n ) );
		}

		public static void ShallowCopy< T1, T2 >( this IEnumerable< KeyValuePair< T1, T2 > > collection, Dictionary< T1, T2 > destination )
		{
			collection.Apply( n => destination.Add( n.Key, n.Value ) );
		}

		public static List< T > Duplicate_As_List< T >( this IEnumerable< T > collection )
		{
			List< T > new_list = new List< T >();
			collection.Apply( n => new_list.Add( n ) );
			return new_list;
		}

		public static Dictionary< T1, T2 > Duplicate< T1, T2 >( this Dictionary< T1, T2 > collection )
		{
			Dictionary< T1, T2 > new_dict = new Dictionary< T1, T2 >();
			collection.Apply( n => new_dict.Add( n.Key, n.Value ) );
			return new_dict;
		}
	}
}
