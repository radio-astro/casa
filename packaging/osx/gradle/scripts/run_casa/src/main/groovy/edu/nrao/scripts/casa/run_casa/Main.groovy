#!/usr/bin/env groovy
package edu.nrao.scripts.casa.run_casa
import java.util.concurrent.Executors
import java.util.concurrent.ExecutorService
import java.util.concurrent.Callable
import java.lang.reflect.Field

// watches with 'ps' for the exit of the process
class ProcWatch implements Callable<String> {
	int pid = 0
	String user = "nobody"
	ProcWatch(Process proc) {
		// http://www.golesny.de/p/code/javagetpid
		if( proc.getClass().getName().equals("java.lang.UNIXProcess") ) {
			try {
				Field f = proc.getClass().getDeclaredField("pid")
				f.setAccessible(true)
				pid = f.getInt(proc)
			} catch (Throwable e) {
				throw new RuntimeException("cannot determine process pid" )
			}
		} else {
			throw new RuntimeException("cannot determine process pid" )
		}
		// get username for later...
		try { user = System.getProperty("user.name") }
		catch ( Throwable e) { throw new RuntimeException("cannot determine username") }
	}

	String call( ) { 
		final int delay = 10000
		println "starting watch for ${user}/${pid}..."
		Thread.sleep(delay)
		try {
			while ( alive( ) ) { Thread.sleep(delay) }
			return "process exited abnormally"
		} catch ( InterruptedException e ) { 
			return ""
		} catch ( Exception e ) { 
			e.printStackTrace();
			return "exception received"
		} 
	}
	private def boolean alive( ) {
		boolean result = false
		def proc = "ps -U ${user}".execute( )
		proc.in.eachLine {
			if ( it.trim( ).startsWith("${pid}") ) result = true
		}
		return result
	}
}

// comsumes process output
class ConsumeOutput implements Callable<String> {
	java.io.InputStream input_stream = null;
	private def action = null;
	private def buffer = new StringBuffer( );

	ConsumeOutput(java.io.InputStream is,Closure a) {
		action = a
		input_stream = is
	}
	String call( ) {
		def count = 0                                                       // quick checks until threshold
		def timeout = 0                                                     // return after timeout iterations
		final def timeout_multiple = 360
		println "starting consume..."
		try { 
			while ( this != null && timeout < timeout_multiple ) {
				int cached = process_output( )
				if ( cached < -1 ) return "python error"
				else if ( cached == -1 ) return "stream closed"
				else if ( cached == 0 ) {
					if ( count < 300 ) { 
						count += 1                                          // rapid check
						Thread.sleep(40)
					} else {
						timeout += 1                                        // timeout check
						Thread.sleep(5000)
					}
				} else {
					count = 0                                               // clear counters
					timeout = 0
				}
			}
			return "output timeout"
		} catch ( InterruptedException e ) { 
			return ""
		} catch ( Exception e ) { 
			e.printStackTrace();
			return "exception received"
		} 
	}
	void flush( ) {
		try { while ( process_output( ) > 0 ); }
		catch (Throwable t) { }
	}

	private int process_output( ) { 
		if ( input_stream.available( ) > 0 ) {
			int ch = input_stream.read();
			if ( ch != -1 ) buffer.append((char)ch)
			else return -1
		} else {
			if ( buffer.length( ) > 0 ) { 
				if ( action != null ) { 
					if ( action(buffer.toString( )) ) { 
						buffer.setLength(0)
						return -2
					}
				}
				buffer.setLength(0)                                 // clear StringBuffer
			}
		}
		return buffer.length( )
	}
}

class ProcWait implements Callable<String> {
	// waits for the process exit status
	Process proc = null
	ProcWait( Process p ) { proc = p }
	String call( ) {
		try { 
			println "process has exited... " + proc.exitValue( )
			return "process exited"
		} catch ( IllegalThreadStateException e ) { println "starting wait" }
		try {
			Thread.sleep(300)
			proc.waitFor( )
			return "process exited normally"
		} catch ( InterruptedException e ) { 
			return ""
		} catch ( Exception e ) {
			e.printStackTrace()
			return "exception received"
		}
	}
}

def os = System.getProperty("os.name").toLowerCase().contains('os x') ? 'osx' : 'linux'
def xvfb = null
def xauth_file = null
def env = [:]
def start_time = new Date( )

def tmpdir = null               // temporary directory                         *** NOT USED***
def evalfile = null             // eval file                                   *** NOT USED***
def distro = null               // root directory of the casa distribution
boolean force_xvfb = false      // run Xvfb even if $DISPLAY is already set
def working_dir = null          // directory to run the command in
def command = null              // command to run in casa


def randomHash(Range... rges) {
	def alphabet = rges.inject(new EmptyRange()) { acc, r -> acc + r }.join( )
	return { int n -> new Random().with { (1..n).collect { alphabet[ nextInt( alphabet.length() ) ] }.join() } }
}

addShutdownHook {
	if ( xvfb != null ) {
		println "killing xvfb..."
//		xvfb.waitForOrKill(1)
		xvfb.destroy( )
	}
	if ( xauth_file != null ) xauth_file.delete( )
	def end_time = new Date( )
	println "exiting  ${end_time}"
	use ( groovy.time.TimeCategory ) {
		println end_time - start_time
	}
}

println "starting ${start_time}"

for ( i=0; i < args.size( ); ++i ) {
	def arg = args[i]
	if ( arg ==~ /^--tmpdir=.+/ ) {
		tmpdir = arg[9..-1]
		if ( ! new File(tmpdir).isDirectory( ) ) throw new RuntimeException("temporary directory does not exist: ${tmpdir}")
	} else if ( arg ==~ /^--eval=.+/ ){
		evalfile = arg[7..-1]
		if ( ! new File(evalfile).isFile( ) ) throw new RuntimeException("evaluation file does not exist: ${evalfile}")
	} else if ( arg ==~ /^--distro=.+/ ) {
		distro = arg[9..-1]
		if ( ! new File(distro).isDirectory( ) ) throw new RuntimeException("distro directory does not exist: ${distro}")
	} else if ( arg == "--xvfb" ) {
		force_xvfb = true
	} else if ( arg == "-cd" ) {
		if ( i >= args.size( ) ) throw new RuntimeException("no directory supplied after '-cd' argument")
		working_directory = args[++i]
		if ( ! new File(working_directory).isDirectory( ) ) throw new RuntimeException("working directory does not exist: ${working_directory}")
	} else if ( arg == "-c" ) {
		if ( i >= args.size( ) ) throw new RuntimeException("no python/casa command supplied after '-c' argument")
		command = args[++i]
	}
}

if ( command == null ) {
	println "no command specified..."
	System.exit(0)
}

if ( distro == null ) {
	println "no casa installation root specified..."
	System.exit(0)
}

if ( os == 'linux' ) {
	if ( force_xvfb ||
		 ! System.getenv().containsKey('DISPLAY') ||
		 System.getenv( )['DISPLAY'].length( ) == 0 ) {
		xauth_file = File.createTempFile('temp','.xauth')

		println "xauthority file: ${xauth_file}"
		for ( display in 4..99 ) {
			println "trying to start xvfb on display :${display}..."
			def cookie = randomHash('a'..'f','0'..'9')(32)
			def xauth = new ProcessBuilder("xauth", "-f", "${xauth_file}", "add", ":${display}", ".", cookie).redirectErrorStream(true).start( )
			xauth.inputStream.eachLine { println "        xauth> ${it}" }
			xvfb = new ProcessBuilder("Xvfb",":${display}", "-screen", "0", "2048x2048x24+32", "-auth", "${xauth_file}").redirectErrorStream(true).start( )

			Thread.sleep(1500)
			// first see if it's still alive
			try {
				int exitValue = xvfb.exitValue( )
				continue
			} catch (IllegalThreadStateException e) {  }

			// wait for some output to be available...
			// if Xvfb starts successfully, there may be no output...
			for ( c in 0..20 ) {
				if ( xvfb.inputStream.available( ) ) break
				else Thread.sleep(500)
			}

			if ( xvfb.inputStream.available( ) ) {
				def success = true
				def reader = new BufferedReader(new InputStreamReader(xvfb.inputStream))
				for ( def line = reader.readLine( ); line != null && reader.ready( ); line = reader.readLine( ) ) {
					if ( line ==~ /.*?already active.*/ ) success = false
					else if ( line ==~ /(?i).*?Could not create server lock file.*/ ) success = false
					else if ( line ==~ /(?i).*?Initializing built-in extension.*/ ) break
					else if ( line.startsWith("_XSERVTransmkdir") ) continue
					else println "         xvfb> ${line}"
				}
				if ( success ) { 
					env += [ "XAUTHORITY": xauth_file.toString(), "DISPLAY": ":" + display.toString() + ".0" ]
					break
				}
			}

			//******** on RHEL5... there is no output when startup succeeds... ********
			println "\tno xvfb output available..."
			Thread.sleep(1500)
			try {
				int exitValue = xvfb.exitValue( )
				println "\txvfb has exited..."
			} catch (IllegalThreadStateException e) {
				println "\txvfb is still alive, so we'll carry on..."
				env += [ "XAUTHORITY": xauth_file.toString(), "DISPLAY": ":" + display.toString() + ".0" ]
				break
			}

			xvfb.waitForOrKill(1)
			xvfb = null
		}
	} else if ( System.getenv( ).containsKey('DISPLAY') ) {
		def disp = System.getenv( )['DISPLAY']
		println "using existing display ${disp}"
	}
}

println "running command:  ${command}"

def casaConfigCmd = null

if (os=="osx")
    casaConfigCmd = "${distro}/Contents/MacOS/casa-config"
else { 
    casaConfigCmd = ["bin/casa-config","casa-config"].inject(null) {
        result,i -> result != null ? result : new File("${distro}/${i}").exists( ) ? "${distro}/${i}" : null
    }
}

assert casaConfigCmd != null && new File(casaConfigCmd).exists( )

println "           with: " + casaConfigCmd


final ExecutorService executor = Executors.newCachedThreadPool( )
def casa = new ProcessBuilder( casaConfigCmd, "--exec", "casa", "--nologger", "--log2term", "--colors=NoColor", "-c", command ).redirectErrorStream(true)
casa.directory( new File("${working_directory}") )
def process_env = casa.environment( )
process_env.putAll(env)
def casa_proc = casa.start( )

def consumer = new ConsumeOutput(casa_proc.getInputStream( ),{
									 print it
									 if ( it ==~ /(?s).*\QOops, IPython crashed. We do our best to make it stable, but...\E.*/ ) return true
									 return false
								 })

String result = executor.invokeAny([ new ProcWatch(casa_proc), consumer, new ProcWait(casa_proc) ])
consumer.flush( )
println "result ${result}"
System.exit(0)
