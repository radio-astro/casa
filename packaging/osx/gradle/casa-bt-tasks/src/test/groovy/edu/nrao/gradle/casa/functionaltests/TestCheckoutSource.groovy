package edu.nrao.gradle.casa.functionaltests
import edu.nrao.gradle.casa.build.GitRepository
import edu.nrao.gradle.casa.build.SourceRepository
import edu.nrao.gradle.casa.build.SvnRepository

import java.util.ArrayList;

import edu.nrao.gradle.casa.build.RepositoryType

class TestCheckoutSource extends GroovyTestCase  {

	void testGetSvnSource() {
		SourceRepository sp = new SvnRepository();
		ArrayList env = System.getenv().collect { k, v -> "$k=$v" }
		sp.getSource("/tmp/casasources","https://svn.cv.nrao.edu/svn/casa/", "trunk", env);
	}
	
	void testGetGitSource() {
		SourceRepository sp = new GitRepository();
		ArrayList env = System.getenv().collect { k, v -> "$k=$v" }
		sp.getSource("/tmp/casasources","git@gitlab.nrao.edu:casa/casa-integration.git", "master", env);
	}
}
