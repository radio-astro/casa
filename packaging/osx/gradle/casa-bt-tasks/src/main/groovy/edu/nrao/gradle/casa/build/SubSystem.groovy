package edu.nrao.gradle.casa.build

import java.util.ArrayList;

class SubSystem {
	

	String id; // Casacore, code, gcwrap, asap...
	
	
	// Run CMake for a subSystem. The subsystem options are defined in BuildConfig
	
	public Integer runCMake (ArrayList env, BuildConfig buildConfig, String sourceDir) {
		if (id == null) {
			println "Subsystem Id not set. Can't run cmake"
			System.exit(1);
		}
		
		StringBuffer outputStream = new StringBuffer();
		StringBuilder strBuilder = new StringBuilder();
		println "Environment for cmake: " + env
		def proc ="mkdir build".execute(env, new File(sourceDir+"/" + id + "/"))
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
		
		print buildConfig.getCmakeCmdBySubSystem(id)
		for (int i = 0; i < buildConfig.getCmakeCmdBySubSystem(id).length; i++) {
		   strBuilder.append(buildConfig.getCmakeCmdBySubSystem(id)[i]);
		   strBuilder.append(" ");
		}
		
		println "Configuring " + id
		String cmakeCmd = strBuilder.toString();
		println cmakeCmd
		proc = cmakeCmd.execute(env, new File(sourceDir+"/" + id + "/build"))
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
		
		if (proc.exitValue()!=0) {
			println "Error configuring "+ id
			System.exit(1)
		}
	}
	
	
	// Run make for a subsystem.
	
	public void runMake(String sourceDir, Integer cores, ArrayList env) {
		if (id == null) {
			println "Subsystem Id not set. Can't run make"
			System.exit(1);
		}
		
		def outputStream = new StringBuffer();
		String makeDirectory=sourceDir+"/"+id+"/build"
		println "Make directory: " + makeDirectory
		def proc=("make -j " + cores).execute(env, new File(makeDirectory))
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
		   if (proc.exitValue()!=0) {
			println "Error configuring "+ id
			System.exit(1)
		}
	}
	
	// Make docs
	public void makeDoc(String sourceDir, ArrayList env) {
		if (id == null) {
			println "Subsystem Id not set. Can't run make"
			System.exit(1);
		}
		
		def outputStream = new StringBuffer();
		String makeDirectory=sourceDir+"/"+id+"/build"
		println "Make directory: " + makeDirectory
		def proc=("make doc" ).execute(env, new File(makeDirectory))
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
		   if (proc.exitValue()!=0) {
			println "Error configuring "+ id
			System.exit(1)
		}
	}


	public String getId() {
		return id;
	}


	public void setId(String id) {
		this.id = id;
	}
	
	
}
