/**********************************************************************************************************************

	LogInterface.cs		
 		A static class used to write out progress, errors, and diagnostics to a text file
 
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
using System.Collections.Generic;

namespace IPCodeGen
{
	public class CCodeGenTask
	{
		public CCodeGenTask()
		{
			Finished = false;
		}

		public bool Finished { get; set; }
	}

	public class CEnumCodeGenTask : CCodeGenTask
	{
		public CEnumCodeGenTask() {}
	}

	public class CCodeGenTaskTracker
	{
		public CCodeGenTaskTracker()
		{
		}

		public void Run() {}

		private List< CCodeGenTask > m_PendingTasks = new List< CCodeGenTask >();
		private List< CCodeGenTask > m_InProgressTasks = new List< CCodeGenTask >();
		private List< CCodeGenTask > m_FinishedTasks = new List< CCodeGenTask >();
	}
}

