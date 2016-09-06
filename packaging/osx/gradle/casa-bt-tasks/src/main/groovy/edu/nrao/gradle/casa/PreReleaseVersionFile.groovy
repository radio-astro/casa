package edu.nrao.gradle.casa

/**
 * 
 *  The purpose of this class is to track (prerelease) build version numbers with 
 * 
 * */
class PreReleaseVersionFile {
	
	 
	
	public PreReleaseVersionFile(String versionFileName) {
		this.versionFileName = versionFileName;
		this.versionFile = new File(versionFileName)
	}
	
	public Integer getLastRevision() {
		return Integer.valueOf(this.getLastRow(this.getVersionFileRows()).split()[0])
	}
	
	public Integer getLastCounter() {
		return Integer.valueOf(this.getLastRow(this.getVersionFileRows()).split()[1])
	}
	
	public void updateCounter (Integer revision) {
		this.addRow(revision, this.getLastCounter()+1)
	}

	String versionFileName = null;
	File versionFile =null; 
	
	
	
	public String [] getVersionFileRows () {
		def rows = []
		return new File( versionFileName ).eachLine { line ->
			rows << line
		}
		return rows
	}
	
	public String getLastRow(String [] rows) {
		return rows[rows.length-1]
	}
	
	
	private void addRow (Integer revision, Integer counter) {
		versionFile.append("\n"+revision+" "+counter)
	}
	
	public String getVersionFileName() {
		return versionFileName;
	}
	public String getVersionFile() {
		return versionFile;
	}
	
	/*public static void main (String [] args) {
		
		PreReleaseVersionFile vf = new PreReleaseVersionFile("/Users/vsuorant/Documents/workspace/development_tools/gradle/casa-bt-tasks/src/main/groovy/edu/nrao/gradle/casa/vf.txt");
		println vf.getVersionFileName();
		println vf.getVersionFile();
		
		Integer lastRevision = Integer.valueOf(vf.getLastRow(vf.getVersionFileRows()).split()[0])
		Integer lastCounter = Integer.valueOf(vf.getLastRow(vf.getVersionFileRows()).split()[1])
		
		Integer newRevision=446;
		
		if (newRevision>vf.getLastRevision()){
		  vf.updateCounter(newRevision)
		}
		else {
			println "New revision is less than the existing one. Not updating the counter."
		}
		//vf.addRow(lastRevision+1,lastCounter+1);
		println Integer.valueOf(vf.getLastRow(vf.getVersionFileRows()).split()[0])
		println Integer.valueOf(vf.getLastRow(vf.getVersionFileRows()).split()[1])
	} */
	
}
