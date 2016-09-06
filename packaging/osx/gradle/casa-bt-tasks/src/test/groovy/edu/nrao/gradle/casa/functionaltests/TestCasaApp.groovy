package edu.nrao.gradle.casa.functionaltests
import java.io.File;
import java.util.ArrayList;
import edu.nrao.gradle.casa.build.BuildConfig
import edu.nrao.gradle.casa.build.CasaApp
import edu.nrao.gradle.casa.build.RepositoryType
import edu.nrao.gradle.casa.build.SubSystem
import edu.nrao.gradle.casa.SvnUtils;
import edu.nrao.gradle.casa.Sys

class TestCasaApp extends GroovyTestCase  {

	void testGetVersionForTrunk() {
		
		CasaApp app = new CasaApp();
		app.setBuildId( "https://svn.cv.nrao.edu/svn/casa/","trunk");
		println "App version " + app.getVersion();
		println "App revision " + app.getRevision();
		
	}	
	
	void testGetVersionForRelease() {
		CasaApp app = new CasaApp();
		app.setBuildId( "https://svn.cv.nrao.edu/svn/casa/", "release-4_5");
		println "App version " + app.getVersion();
		println "App revision " + app.getRevision();
		SvnUtils su = new SvnUtils();
		println "Is branch changed: "+ su.isBranchChanged("https://svn.cv.nrao.edu/svn/casa/branches/release-4_5", 34427)
    }
	
	void testReplaceRevision() {
		CasaApp app = new CasaApp();
		File srcDir = new File("/tmp/casasources")
		String branch = "release-4_5"
		String svnPath = "https://svn.cv.nrao.edu/svn/casa/branches/release-4_5"
		String version = "4.5.0"
		String revision = "222222"
		ArrayList env = System.getenv().collect { k, v -> "$k=$v" }
	    app.replaceVersion(srcDir, branch, svnPath, version, revision, env)
	}
	
	void testPackageApp() {
		
		CasaApp app = new CasaApp();
		ArrayList env = System.getenv().collect { k, v -> "$k=$v" }
		app.packageApp(new File("/tmp/casasources"), new File("/tmp/CASA.app"), new File("/tmp/CASA"), env)
		
	}

}