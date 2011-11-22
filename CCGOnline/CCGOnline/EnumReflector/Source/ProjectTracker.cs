using System;
using System.IO;
using System.Collections.Generic;

namespace EnumReflector
{
	public enum EProjectID
	{
		Invalid = 0
	}

	public class CProject
	{
		public CProject( EProjectID id, string name )
		{
			ID = id;
			Name = name;
		}

		public void Initialize( FileInfo project_file )
		{
			??;
		}

		public EProjectID ID { get; private set; }
		public string Name { get; private set; }
	}

	public class CProjectTracker
	{
		public CProjectTracker()
		{
		}

		public void Register_Project( FileInfo project_file )
		{
			string project_name = Path.GetFileNameWithoutExtension( project_file.Name );
			string upper_project_name = project_name.ToUpper();

			if ( m_ProjectIDMap.ContainsKey( upper_project_name ) )
			{
				throw new Exception( "Duplicate project name: " + upper_project_name );
			}

			EProjectID id = Allocate_Project_ID();
			CProject project = new CProject( id, upper_project_name );
			m_ProjectIDMap.Add( upper_project_name, id );
			m_Projects.Add( id, project );

			project.Initialize( project_file );
		}

		private EProjectID Allocate_Project_ID()
		{
			return m_NextAllocatedID++;
		}

		private Dictionary< EProjectID, CProject > m_Projects = new Dictionary< EProjectID, CProject >();
		private Dictionary< string, EProjectID > m_ProjectIDMap = new Dictionary< string, EProjectID >();
		private EProjectID m_NextAllocatedID = EProjectID.Invalid + 1;
	}
}