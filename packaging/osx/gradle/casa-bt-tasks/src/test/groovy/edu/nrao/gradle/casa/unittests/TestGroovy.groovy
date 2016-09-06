package edu.nrao.gradle.casa.unittests
import edu.nrao.gradle.casa.Sys

class TestGroovy extends GroovyTestCase  {
	
	void testSomething() {
		println ""
		println "Hi"
		println Sys.os();
	}
}
