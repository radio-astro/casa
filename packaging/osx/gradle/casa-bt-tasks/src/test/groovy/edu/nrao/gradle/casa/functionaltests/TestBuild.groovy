package edu.nrao.gradle.casa.functionaltests
import java.util.ArrayList;
import edu.nrao.gradle.casa.build.BuildConfig
import edu.nrao.gradle.casa.build.RepositoryType
import edu.nrao.gradle.casa.build.SubSystem
import edu.nrao.gradle.casa.Sys

class TestBuild extends GroovyTestCase  {

	void testBuildCasacore() {
		
		def sourceDir =  "/tmp/casasources"
		
		ArrayList env = System.getenv().collect { k, v -> "$k=$v" }
				
		BuildConfig buildConfig = new BuildConfig(Sys.os());
		buildConfig.loadConfig("../build.properties")
		
		// def subSystems = ["casacore", "code", "asap", "gcwrap"]
		// Unfortunately code can't be configured before casacore has been build
		def subSystemId = "casacore"
		
		SubSystem  subSystem = new SubSystem();
		subSystem.setId (subSystemId);
		println "Configuring " + subSystemId
		subSystem.runCMake(env, buildConfig, sourceDir)
			
		println "Building Casacore"
		subSystem.runMake(sourceDir, 8, env)
		// Casacore is the only one with the separate install target
		println "make install".execute(env, new File(sourceDir+"/casacore/build")).text
		
	}	
	
	void testBuildAll() {
		
		def sourceDir =  "/tmp/casasources"
		Integer numCores=8
		
		ArrayList env = System.getenv().collect { k, v -> "$k=$v" }
				
		BuildConfig buildConfig = new BuildConfig(Sys.os());
		buildConfig.loadConfig("../build.properties")
		
		// def subSystems = ["casacore", "code", "asap", "gcwrap"]
		// Unfortunately code can't be configured before casacore has been build
		String casaCoreId = "casacore"
		
		SubSystem  subSystem = new SubSystem();
		subSystem.setId (casaCoreId);
		println "Configuring " + casaCoreId
		subSystem.runCMake(env, buildConfig, sourceDir)
			
		println "Building Casacore"
		subSystem.runMake(sourceDir, numCores, env)
		// Casacore is the only one with the separate install target
		println "make install".execute(env, new File(sourceDir+"/casacore/build")).text
		
		def subSystems = ["code", "asap", "gcwrap"] 
		
		for (String subSystemId: subSystems) {
			subSystem = new SubSystem();
			subSystem.setId (subSystemId);
			println "Configuring " + subSystemId
			subSystem.runCMake(env, buildConfig, sourceDir)
				
			println "Building " + subSystemId
			subSystem.runMake(sourceDir, numCores, env)
		}
		
	}
}