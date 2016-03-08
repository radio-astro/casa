package edu.nrao.gradle.casa
class SvnUtils {
 
	
	/*public static void main (String [] args ) {
		String branch = "https://svn.cv.nrao.edu/svn/casa/branches/release-4_3/"
		String trunk = "https://svn.cv.nrao.edu/svn/casa/trunk/"
		SvnUtils util = new SvnUtils();
		
		println util.getRevisionForBranch(branch)
		println util.getRevisionForBranch(trunk)
	    println "isBranchChanged: " + util.isBranchChanged(branch,1)
	    println "isBranchChanged: " + util.isBranchChanged(branch,32491)
	    println "isBranchChanged: " + util.isBranchChanged(branch,null)
	    println "isBranchChanged: " + util.isBranchChanged(branch,60000)
	} */
	
	Boolean isBranchChanged (String branch, Integer previousBuildRevision) {
		println "previousBuildRevision: " + previousBuildRevision
		Integer currentRevision = getRevisionForBranch (branch)
		println "Current revision: " + currentRevision 
		if (previousBuildRevision<currentRevision) {
			return true;
		}
		return false;
	}
	
	/**
	 * Get svn revision information for a given branch
	 *  */
	Integer getRevisionForBranch (String branch) {
		String svnCommand = "svn info "+ branch
		Process getSvnInfo=svnCommand.execute()
		def g=["grep","Last Changed Rev"]
		Process grepForLastRev=g.execute()
		println svnCommand
		def svnInfoResult =(getSvnInfo|grepForLastRev).text.split()
		Integer revision = Integer.valueOf(svnInfoResult[svnInfoResult.size()-1])
		return revision
	}
	
}
