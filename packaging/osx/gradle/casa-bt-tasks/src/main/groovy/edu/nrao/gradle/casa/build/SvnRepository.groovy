package edu.nrao.gradle.casa.build

import java.util.ArrayList;

public class SvnRepository implements SourceRepository{

	private RepositoryType repositoryType = RepositoryType.SVN;

	@Override
	public RepositoryType getType() {
		return repositoryType;
	}

	@Override
	public void getSource(String localDir, String repositoryLocation, String branch, ArrayList env) {
		
		def branchOrig=branch;
		// Fix the path for branches
		if (branch!="trunk" && branch!="packaging") {
			branch="branches/"+branch
		}
		
		String checkoutCommand= "svn checkout " + repositoryLocation + "/" + branch +  " " + localDir + "/" + branchOrig
		println "Executing: "
		println checkoutCommand 
		
		def outputStream = new StringBuffer();
		def proc=checkoutCommand.execute()
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
		
	}

	@Override
	public void updateSource() {
		// TODO Auto-generated method stub
		
	}

	public void getSourceFromRevision(String localDir, String repositoryLocation, String branch, String revision, ArrayList env){
		
		def branchOrig=branch;
		// Fix the path for branches
		if (branch!="trunk" && branch!="packaging") {
			branch="branches/"+branch
		}
		
		String checkoutCommand= "svn checkout -r" +revision+" "+ repositoryLocation + "/" + branch +  " " + localDir + "/" + branchOrig
		println "Executing: "
		println checkoutCommand 
		
		def outputStream = new StringBuffer();
		def proc=checkoutCommand.execute()
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
		
		// Pull casacore from GitHub
		if (branch.equals("trunk") && !branch.equals("packaging")) {
			  println "Replacing casacore with the version from Git"
			File casacoreDir= new File (localDir + "/" + branchOrig + "/casacore")
			if (casacoreDir.exists()) {
				println "Removing casacore directory " + casacoreDir.toString()
				casacoreDir.deleteDir()
			}
			else {
				println "Couldn't find an existing casacore directory " + casacoreDir.toString()
				println "Continuing with checkout from Git"
			}
			
			checkoutCommand= "git clone https://github.com/casacore/casacore.git"
			println "Executing: "
			println checkoutCommand
			
			outputStream = new StringBuffer();
			proc=checkoutCommand.execute(env, new File (localDir+ "/" +  branchOrig))
			proc.waitForProcessOutput(System.out, System.err)
			println outputStream.toString()
		}
		
	}
}
