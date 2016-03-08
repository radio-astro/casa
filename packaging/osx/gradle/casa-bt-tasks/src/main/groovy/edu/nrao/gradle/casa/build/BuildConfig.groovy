package edu.nrao.gradle.casa.build

import java.util.ArrayList;

/**
 * 
 * Utility class for easy access to configuration data.
 * 
 * */
class BuildConfig {

	private Properties props;
	
	private String osName;
	
	// Root directory for builds
	//private String buildDir;
	
	// Branch information
	private String testBranch ;
	private String releaseBranch;
	
	// Git SVN information
	private String svnroot;
	private String gitRepoSsh;
	private String gitRepoHttp;
	
	// CMake config
	private String cmakePrefix="casa.build." + osName +".";
	private String [] casaCoreCmake;
	private String [] codeCmake;
	private String [] gcwrapCmake;
	private String [] asapCmake;
	
	
	/**
	 * OsName is required for the constructor as it is need to read the rest of the properties
	 * */
	BuildConfig (osName) {
		osName=osName;
	}
	
	/**
	 * Load configuration from a property file
	 * */
	public void loadConfig (File propertyFile) {
		
		//propertyFileName = "build.properties";
		props = new Properties()
		
		    propertyFile.withInputStream {
			stream -> props.load(stream)
		}
	
		//buildDir = props["builddir"]
		testBranch = props["casa.build.testbranch"]
		releaseBranch = props["casa.build.prereleasebranch"]
		
		svnroot = props["casa.build.svnroot"]
		gitRepoSsh = props["casa.build.gitrepossh"]
		gitRepoHttp = props["casa.build.gitrepohttp"]
		
		casaCoreCmake=props[cmakePrefix + "casacorecmake"].split(" ")
		codeCmake=props[cmakePrefix + "codecmake"].split(" ")
		gcwrapCmake=props[cmakePrefix + "gcwrapcmake"].split(" ")
		asapCmake=props[cmakePrefix + "asapcmake"].split(" ")
	
	}
	
	public String [] getCmakeCmdBySubSystem (String subSystemName) {
		switch (subSystemName) {
			case "casacore": 
				return casaCoreCmake;
			case "code":
				return codeCmake;
			case "gcwrap":
				return gcwrapCmake;
			case "asap":
				return asapCmake;
			default: 
				return null;
		}
	}

	// Get the latest tag for test/release builds
	public String getTag (String tag, String buildType, ArrayList env, String sourceDir) {
		if (tag==null || tag=="")  {
			String latestTag;
			if (buildType == "test") {
				def outputStream = new StringBuffer();
				def proc1 = "git tag --list".execute(env, new File(sourceDir))
				def proc2 = ("grep " + testBranch + "- ").execute(env, new File(sourceDir))
				def proc3 = "sort -r".execute(null, new File(sourceDir))
				def proc4 = "head -n 1".execute(null, new File(sourceDir))
				proc1 | proc2 | proc3 | proc4
				latestTag=proc4.text//waitForProcessOutput(System.out, System.err)
			}
			else {
				def outputStream = new StringBuffer();
				def proc1 = "git tag --list".execute(env, new File(sourceDir))
				def proc2 = ("grep " + releaseBranch + "- ").execute(env, new File(sourceDir))
				def proc3 = "sort -r".execute(null, new File(sourceDir))
				def proc4 = "head -n 1".execute(null, new File(sourceDir))
				proc1 | proc2 | proc3 | proc4
				latestTag=proc4.text//waitForProcessOutput(System.out, System.err)
			}
			tag = latestTag
		}
		return tag.trim()
	}

	public String getTestBranch() {
		return testBranch;
	}


	public void setTestBranch(String testBranch) {
		this.testBranch = testBranch;
	}


	public String getReleaseBranch() {
		return releaseBranch;
	}


	public void setReleaseBranch(String releaseBranch) {
		this.releaseBranch = releaseBranch;
	}


	public String getSvnroot() {
		return svnroot;
	}


	public void setSvnroot(String svnroot) {
		this.svnroot = svnroot;
	}


	public String getGitRepoSsh() {
		return gitRepoSsh;
	}


	public void setGitRepoSsh(String gitRepoSsh) {
		this.gitRepoSsh = gitRepoSsh;
	}


	public String getGitRepoHttp() {
		return gitRepoHttp;
	}


	public void setGitRepoHttp(String gitRepoHttp) {
		this.gitRepoHttp = gitRepoHttp;
	}


	public String[] getCasaCoreCmake() {
		return casaCoreCmake;
	}


	public void setCasaCoreCmake(String[] casaCoreCmake) {
		this.casaCoreCmake = casaCoreCmake;
	}


	public String[] getCodeCmake() {
		return codeCmake;
	}


	public void setCodeCmake(String[] codeCmake) {
		this.codeCmake = codeCmake;
	}


	public String[] getGcwrapCmake() {
		return gcwrapCmake;
	}


	public void setGcwrapCmake(String[] gcwrapCmake) {
		this.gcwrapCmake = gcwrapCmake;
	}


	public String[] getAsapCmake() {
		return asapCmake;
	}


	public void setAsapCmake(String[] asapCmake) {
		this.asapCmake = asapCmake;
	}

	public String getOsName() {
		return osName;
	}

	public void setOsName(String osName) {
		this.osName = osName;
	}

	//public String getBuildDir() {
	//	return buildDir;
	//}

	//public void setBuildDir(String buildDir) {
	//	this.buildDir = buildDir;
	//}
	
	
	
}
