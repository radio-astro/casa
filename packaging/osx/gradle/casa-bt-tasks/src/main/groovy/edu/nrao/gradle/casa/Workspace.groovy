package edu.nrao.gradle.casa

import org.gradle.api.DefaultTask
import org.gradle.api.tasks.TaskAction
import org.htmlcleaner.*

class Workspace { 

	Workspace( p ) { path_ = p }

	private def path_
	private def revision_

	def revision( ) throws Exception { 
		return revision_
	}

}