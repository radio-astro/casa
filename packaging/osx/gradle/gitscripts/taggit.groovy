/**
* This script takes a branch as an argument, attempts to find the latest tag
* associated with that branch, creates a new tag and pushes that upstream.
*
* The intended use case is to create tags for the nightly builds.
*
* Only a single instance of this script should be run at any given time.
*
* The tag naming convention is as follows:
*
* 4.6.0-rel-1      <--- Release tags are special
* 4.6-val-1
* 4.6-mas-1
*
* The branch naming convention is as follows:
* 4.6.0-release    <--- Release branches are special
* 4.6-validation
* master
*
*/

File workDir = new File("/Users/vsuorant/casa-git")
File sourceDir = new File("/Users/vsuorant/casa-git/casa")
String gitRemote = "http://vsuorant@localhost:7990/scm/casa/casa.git"

ArrayList env = System.getenv().collect { k, v -> "$k=$v" }
StringBuffer outputStream = new StringBuffer();

String cmd = ""
def proc
String branch
String lastTag

if (args.length<1 || args[0] == null) {
  println "Please provide a branch name";
  System.exit(0)
}
else {
  branch = args[0]
  println "Branch = " + branch
}

if (!workDir.exists()) {
			workDir.mkdirs()
      cmd = "git clone " + gitRemote + " casa"
      proc = cmd.execute(env, workDir)
      proc.waitForProcessOutput(System.out, System.err)
      println outputStream.toString()
}

// Start from a known state
cmd = "git checkout master"
proc = cmd.execute(env, sourceDir)
proc.waitForProcessOutput(System.out, System.err)
println outputStream.toString()

// Update all branches
cmd = "git pull --all"
proc = cmd.execute(env, sourceDir)
proc.waitForProcessOutput(System.out, System.err)
println outputStream.toString()

if ( branch.equals("master")) {
    def proc1 = "git tag -l".execute(env, sourceDir)
    def proc2 = "grep \\-mas-".execute(env, sourceDir)
    def proc3 = "sort -t- -k1,1nr -k2,2nr -k3,3nr".execute(env, sourceDir)
    def proc4 = "head -n 1".execute(env, sourceDir)
    lastTag = (proc1 | proc2 | proc3 | proc4).text
    println "Last master tag: " + lastTag
}
else {
    String [] branchParts = branch.split("-")
    String tagId
    if (branch.contains("validation")){
      tagId = "-val-"
      println "Validation branch"
      String tagGroup = branchParts[0] + tagId
      println "tagGroup " + tagGroup
    }
    else if (branch.contains("release")){
      tagId = "-rel-"
      println "Release branch"
      String tagGroup = branchParts[0] + tagId
      println "tagGroup " + tagGroup
    }

    cmd = "git checkout " + branch
    proc = cmd.execute(env, sourceDir)
    proc.waitForProcessOutput(System.out, System.err)
    println outputStream.toString()

    String tagGrep = "grep " + branchParts[0] + tagId
    def proc1 = "git tag -l".execute(env, sourceDir)
    def proc2 = tagGrep.execute(env, sourceDir)
    def proc3 = "sort -t- -k1,1nr -k2,2nr -k3,3nr".execute(env, sourceDir)
    def proc4 = "head -n 1".execute(env, sourceDir)

    lastTag = (proc1 | proc2 | proc3 | proc4).text
    println "Last tag: " + lastTag
}

// Attempt to construct a new tag based on an existing one
print "Constructing a new tag based on " + lastTag
try {
  def (prefix, id, serial) = lastTag.split("-")
  serial = Integer.valueOf(serial.trim())+1
  newTag = prefix + "-" + id + "-" + serial
  println newTag
} catch (Exception e){
  String message =  "Couldn't determine next tag id. Either there are no" +
    "previous tags or the latest tag doesn't conform to the naming convention."
  throw (new Exception (message))
}

// Check for diffs, if none, bail
String diff ="git diff --exit-code $lastTag $branch"
println "Comparing tag to branch with:"
proc = diff.execute(env, sourceDir)
proc.waitForProcessOutput(System.out, System.err)
println outputStream.toString()
def exitValue = proc.exitValue()
println "Comparison returned: " + exitValue
if (exitValue == 0) {
  println "There are no changes compared to the latest tag. Nothing to do."
  System.exit(0)
}

// Create tag
message = "Automatic tag"
String [] tagCmd= ["git", "tag", "-a", newTag, "-m '$message'"]
println tagCmd
proc = tagCmd.execute(env, sourceDir)
proc.waitForProcessOutput(System.out, System.err)
println outputStream.toString()
println "Done tagging " + newTag

// Push tag to origin
cmd = "git push origin " + newTag
proc = cmd.execute(env, sourceDir)
proc.waitForProcessOutput(System.out, System.err)
println outputStream.toString()
println "Pushed $newTag to origin "
