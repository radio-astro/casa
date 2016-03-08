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
		
	}
}
