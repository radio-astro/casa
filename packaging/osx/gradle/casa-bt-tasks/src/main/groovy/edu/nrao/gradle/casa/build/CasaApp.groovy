package edu.nrao.gradle.casa.build;
import edu.nrao.gradle.casa.SvnUtils;

public class CasaApp {
	
	    String version;
		String revision;
		
		enum buildType {REL, DEV}

		public void createApp (File appDir, File srcDir, String workspace, ArrayList env) {
			println "Deleting directory: " + appDir;
			if (appDir.exists()) {
				appDir.deleteDir();
			}
			StringBuffer outputStream = new StringBuffer();
			println "Creating application"
			String cmd="./packaging/scripts/make-app" + " -v -ni ws="+workspace+" arch=darwin out=/tmp"
			println cmd
			def proc = cmd.execute(env, srcDir)
			proc.waitForProcessOutput(System.out, System.err)
			println outputStream.toString()
		}
		
		public void createApp (File appDir, File srcDir, String workspace, File templateDir, String templateRevision,  ArrayList env) {
			
			println "Deleting directory: " + appDir;
			if (appDir.exists()) {
				appDir.deleteDir();
			}
			
			StringBuffer outputStream = new StringBuffer();
			
			println "Checking out template revision " +  templateRevision
			println "To: " +  templateDir
			if (!templateDir.exists()) {
				templateDir.mkdirs()
			}
			
			String cmd = "svn co -r" + templateRevision + " https://svn.cv.nrao.edu/svn/casa/development_tools/packaging/template/osx/CASA.app " + templateDir 
			def proc = cmd.execute(env, templateDir)
			proc.waitForProcessOutput(System.out, System.err)
			println outputStream.toString()
			
			println "Creating application"
			cmd="./packaging/scripts/make-app" + " -v -ni "+ "template=" + templateDir + " ws="+workspace+" arch=darwin out=/tmp"
			println cmd
			proc = cmd.execute(env, srcDir)
			proc.waitForProcessOutput(System.out, System.err)
			println outputStream.toString()
		}
		
		public void packageApp(File appDir, File srcDir, File dmgTempDir, ArrayList env) {
			println "Deleting directory: " + dmgTempDir;
			if (dmgTempDir.exists()) {
				dmgTempDir.deleteDir();
			}
			String cmd ="./packaging/scripts/make-dmg " + appDir
			println "Executing " + cmd
			StringBuffer outputStream = new StringBuffer();
			def proc = cmd.execute(env, srcDir)
			proc.waitForProcessOutput(System.out, System.err)
			println outputStream.toString()
		}
		
		public void replaceVersion(File srcDir, String branch, String svnPath, String version, String revision, env) {
			File workingDir = new File(srcDir.toString() + "/" + branch)
			println "ReplaceVersion: workingDir: " + workingDir.toString()
			String cmd = '../packaging/scripts/replace-version -v ver=' + version + ' rev=' + revision + ' url=' + svnPath + " ."
			println "ReplaceVersion command: " + cmd
			StringBuffer outputStream = new StringBuffer();
			def proc = cmd.execute(env, workingDir)
			proc.waitForProcessOutput(System.out, System.err)
			println outputStream.toString()
		}
		
		public setBuildId(String svnRoot, String branch)  {
			
			def buildType="trunk"
			if (branch!="trunk" && branch!="packaging") {
				buildType="prerelease"
			}
			
			if  (buildType=="trunk") {
			   
			    println "buildType=trunk"
			    def versionUrl = "https://svn.cv.nrao.edu/cgi-bin/casa-version".toURL( )
			   
			    versionUrl.eachLine {
				    if (it.trim()!="") {
				        println "Casa version: " + it
					    version=it.split()[0].trim()
					    revision=it.split()[1].trim()
				    }
			    }
				
				println "Version: " + version
				println "Revision: " + revision
			}
			else  if (buildType=="prerelease") {
			    println "&&& buildType=prerelease"
				def branchSvnPath=svnRoot+"/branches/"+branch
				println "Branch: " + branchSvnPath
				SvnUtils svnUtils = new SvnUtils();
				revision = svnUtils.getRevisionForBranch(branchSvnPath);
				println "&&&: Revision: " + revision
		    }
			else {
			    println "Unknown build type"
		 	    System.exit(1);
			}
		}
		
		public setBuildId(String version, Integer revision)  {
			this.version = version
			this.revision = revision.toString()
		}

		public void setVersion(String version) {
			this.version = version.trim();
		}

		public void setRevision(String revision) {
			println "Setting revision"
			this.revision = revision.trim();
			println this.revision
		}

		public String getVersion() {
			return version;
		}
		
		public String getRevision() {
			return revision;
		}
		
}
