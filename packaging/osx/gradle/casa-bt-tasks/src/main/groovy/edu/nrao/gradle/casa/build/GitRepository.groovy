package edu.nrao.gradle.casa.build;
import java.util.ArrayList;

import edu.nrao.gradle.casa.build.RepositoryType;

public class GitRepository implements SourceRepository {

	private RepositoryType repositoryType = RepositoryType.GIT;
		
	@Override
	public RepositoryType getType() {
		return repositoryType;
	}

	@Override
	public void getSource(String localDir, String repositoryLocation, String branch, ArrayList env) {
		
		File checkoutDir;
		if (branch.startsWith("rel")) {
			checkoutDir = new File (localDir + "/"+branch.replaceAll("rel-", "")).getCanonicalFile();
		}
		else {
			checkoutDir = new File (localDir + "/"+branch).getCanonicalFile();
		}
	
			String cloneCommand= "git clone --recursive " + repositoryLocation + " " + checkoutDir.toString();
		String updateCommand= "git pull"
		String cmd;
		
		println checkoutDir
		if (!checkoutDir.exists()) {
			checkoutDir.mkdirs()
			cmd = cloneCommand;
			println "Cloning repository."
			println "Executing: "
		}
		else {
			cmd = updateCommand;
			println "Updating repository."
			println "Executing: "
		}
		println cmd
		def outputStream = new StringBuffer();
		def proc= cmd.execute(env, checkoutDir)
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
		
		println "Checking out " + branch
		cmd = "git checkout " +  branch
		println  cmd
		outputStream = new StringBuffer();
		proc= cmd.execute(env, checkoutDir )
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
		
		
	}

	public void checkoutVersion (String repositoryLocation, commitId, ArrayList env) {
		
		/**
		 * commitId can be tag, branch or sha
		 * */
		
		String cmd;
		println "Updating repository"
		cmd = "git checkout " +  commitId
		println  cmd
	
		def outputStream = new StringBuffer();
		outputStream = new StringBuffer();
		def proc= cmd.execute(env, new File (repositoryLocation))
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
		
	}

	public void getPackaging (String repositoryLocation, String localDir , ArrayList env) {
		
		/**
		 * commitId can be tag, branch or sha
		 * */
		
		String cmd;
		println "Updating repository"
		cmd = "git clone " +  repositoryLocation
		println  cmd
	
		def outputStream = new StringBuffer();
		def proc= cmd.execute(env, new File (localDir))
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
	
		println "Moving packaging to top dir"
		cmd = "mv ./development_tools/packaging ./packaging" 
		println  cmd
	
		outputStream = new StringBuffer();
		proc= cmd.execute(env, new File (localDir))
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
	}

		
	@Override
	public void updateSource() {
		// TODO Auto-generated method stub
		
	}


	// tagType refers to prefix f.e. master or release-4.5
	public String getLastTag(String sourceDir, String tagType,ArrayList env) {
		println "Getting the last tag from Git:"
		println "tagType: " + tagType
		println "sourceDir: " + sourceDir
		def result;
		
		if (tagType.startsWith("dev") || tagType.startsWith("master") ) {
			def proc1 = "git tag -l".execute(env, new File(sourceDir))
			def proc2 = "grep $tagType".execute(env, new File(sourceDir))
			def proc3 = "sort -t. -k2,2nr -k3,3nr".execute(env, new File(sourceDir))
			def proc4 = "head -n 1".execute(env, new File(sourceDir))
			
			result = (proc1 | proc2 | proc3 | proc4).text
		}
		else if (tagType.startsWith("rel")) { 
			def proc1 = "git tag -l".execute(env, new File(sourceDir))
			def proc2 = "grep $tagType".execute(env, new File(sourceDir))
			def proc3 = "sort -t. -k2,2nr -k3,3nr".execute(env, new File(sourceDir))
			def proc4 = "sort -t- -k3,3nr".execute(env, new File(sourceDir))
			def proc5 = "head -n 1".execute(env, new File(sourceDir))
			
			result = (proc1 | proc2 | proc3 | proc4 | proc5).text
		}
		else {
			throw new Exception ("Not a standard format tag. Can't sort." + " tagType: " + tagType);
		}
		
		//println "Result: " +result
		return result
	}
}
