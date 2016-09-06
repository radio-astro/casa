package edu.nrao.gradle.casa.functionaltests
import java.util.ArrayList;
import edu.nrao.gradle.casa.build.BuildConfig
import edu.nrao.gradle.casa.build.RepositoryType
import edu.nrao.gradle.casa.build.SubSystem
import edu.nrao.gradle.casa.Sys

class TestConfigure extends GroovyTestCase  {

	void testConfigureCasacore() {
		
		def sourceDir =  "/tmp/casasources"
		
		ArrayList<String> env = ["a"]//System.getenv().collect { k, v -> "$k=$v" }
				
		BuildConfig buildConfig = new BuildConfig(Sys.os());
		buildConfig.loadConfig(new File("../build.properties"))
		
		// def subSystems = ["casacore", "code", "asap", "gcwrap"]
		// Unfortunately code can't be configured before casacore has been build
		def subSystems = ["casacore"]
		
		for (String subSystemId: subSystems) {
			SubSystem  subSystem = new SubSystem();
			subSystem.setId (subSystemId);
			println "Configuring " + subSystemId
			subSystem.runCMake(env, buildConfig, sourceDir)
		}
	}
	
}