package edu.nrao.gradle.casa.build

class BundleFixer {
	
	 // Note: sign.sh is used as somehow the codesign won't find an existing signature
	//  For 
	
	 String appDir  = "/Applications/CASA.app"
	 
	 public static void main (String [] args) {
		 // For testing only
		 String appDir = "/Applications/CASA.app"
		 String signScript = "../casa-build/sign.sh"
		 BundleFixer bf = new BundleFixer()
		 bf.fixBundle(appDir)
		 bf.sign(signScript, appDir)
	 }
	 
	 public void sign(String signScript, String appDir) {		
		
		signScript = new File(signScript).absolutePath
			String signCmd = signScript + " " + appDir
		println "Executing: "
		println signCmd 
		
		ArrayList env = System.getenv().collect { k, v -> "$k=$v" }
		
		def outputStream = new StringBuffer();
		def proc =signCmd.execute(env,null)
		proc.waitForProcessOutput(System.out, System.err)
		println outputStream.toString()
	}
	 
	 public void fixBundle(String appDirectory) {
		 
		 File packageTemplateSvnDir = new File (appDirectory+"/.svn")
		 if  (packageTemplateSvnDir.exists()) {
			 println "Removing .svn from CASA.app root: " + packageTemplateSvnDir.toString()
			 packageTemplateSvnDir.deleteDir()
		 }
		 
		 File frameWorkDir = new File (appDirectory + "/Contents/Frameworks/") 
		 // Fix python
		 println "Deleting symlinks from Python bundle"
		 String [] symlinks = ["Headers", "Resources", "Python"]
		 for (String symlink: symlinks) {
			 File f = new File (frameWorkDir.toString()+"/Python.framework/"+symlink)
			 println "Deleting: " + f
			 f.delete()
			 println "Recreating symlink for " + f
			 ['ln', '-s', "Versions/Current/"+symlink, f].execute().waitFor()
		 }
		 
		 // Fix Qt
		 ArrayList<File> qtDirs = new ArrayList<File>();
		 println "Fixing bundle"
		 for (File f: frameWorkDir.listFiles()) {
			 if (f.name.startsWith("Qt")) {
				 qtDirs.add(f)
				 println f
			 }
		 }
		 
		 for (File qtDir: qtDirs) {
			File resourcesDir = new File (qtDir.toString()+"/Versions/Current/Resources")
			if (!resourcesDir.exists()) {
				 println "Creating: " + resourcesDir
				 resourcesDir.mkdirs()
			 }
			File headerDir = new File (qtDir.toString()+"/Versions/Current/Headers")
			if (!headerDir.exists()) {
				 println "Creating: " + headerDir
				 headerDir.mkdirs()
			}
			
		 	for (File f: qtDir.listFiles()) {
				 // Delete .prl files and Headers symlink
				 if (f.name.contains(".prl") ) {
					 println "Deleting:" + f
					 f.delete()
				 }
				 if (f.name=="Headers" || f.name =="Resources") {
					 println "Deleting:" + f
					 f.delete()
					 println "Recreating symlink for " + f
					 println "Symlink name " + f.name
					 
					 ['ln', '-s', "Versions/Current/"+f.name, f].execute().waitFor()
				 }
			}
			 

			 
			File contentsDir = new File (qtDir.toString()+"/Contents/")
			File infoPlist = new File (contentsDir.toString()+"/Info.plist")
			File newInfoPlist =  new File (resourcesDir.toString()+"/Info.plist")
			infoPlist.renameTo(newInfoPlist)
			contentsDir.delete() 
		 }
	}
}
	 
