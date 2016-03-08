package edu.nrao.gradle.casa.unittests
import edu.nrao.gradle.casa.build.SourceRepository
import edu.nrao.gradle.casa.build.SvnRepository
import edu.nrao.gradle.casa.build.RepositoryType

class TestSourceRepository extends GroovyTestCase  {
	
	void testGetSourceType() {
		SourceRepository sp = new SvnRepository();
		assert sp.getType()==RepositoryType.SVN;
	}

}
