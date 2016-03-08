package edu.nrao.gradle.casa
import groovy.io.FileType
import edu.nrao.gradle.casa.Gen
import java.nio.file.Path
import java.nio.file.Paths
import java.nio.file.Files
import java.nio.file.FileVisitResult
import java.nio.file.SimpleFileVisitor
import java.nio.file.attribute.BasicFileAttributes
import java.net.InetAddress
import java.net.UnknownHostException

// with java 1.7 Files can be used for copying (options for the
// copy come from StandardCopyOption...
//import java.nio.file.Files
//import static java.nio.file.StandardCopyOption.*

class Sys {

	static final def timeout = 120000

	static def os( ) {
		return osver_
	}
	static def arch( ) {
		return osarch_
	}

    static def osLabel( ) {
		return oslabel_
	}

    static def env( String key ) { return System.getenv( )[key] }

	static def home( ) {
		return home_
	}

    static String home( String...subdirs ) {
        return [home_,subdirs].flatten( ).join(File.separator)
    }

    static def hostname( ) { 
        if ( hostname_ == null ) { 
            InetAddress iAddress = InetAddress.getLocalHost( )
            hostname_ = iAddress.getHostName( ).takeWhile { it != '.' }
        }
        return hostname_
    }
    static def username( ) {  return username_ }

	static def rcdir( ) {
		def dir = new File("${home_}/.casa/gradle")
		dir.mkdirs( )
		return dir
	}

    static void rm( File p ) { rm(p.toPath( )) }
    static void rm( Path p ) {
        if ( Files.isRegularFile(p) ) Files.delete(p)
        else if ( Files.isSymbolicLink(p) ) Files.delete(p)
        else if ( Files.isDirectory(p) ) rmdir(p)
        else throw new RuntimeException("Sys.rm( ) called with unexpected path")
    }
	static void rmdir( File p ) { rmdir(p.toPath( )) }
	static void rmdir( Path p ) {
		if ( Files.exists(p) == false ) return
		assert Files.isDirectory(p) : "rmdir(...) called with a non-directory argument"

        try {
			// does not follow symbolic links to directories (by default)...
            Files.walkFileTree( p, new SimpleFileVisitor<Path>( ) {
									@Override
									public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
										Files.deleteIfExists(dir);
										return super.postVisitDirectory(dir, exc);
									}

									@Override
									public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
										Files.deleteIfExists(file);
										return super.visitFile(file, attrs);
									}
								} )
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
	}

    static def cwd( ) {
		return System.getProperty( "user.dir" )
	}

	static def copy( File from, File to ) {
	// copies only the directory (not the contents:    Files.copy( from.toPath( ), to.toPath( ), REPLACE_EXISTING )
        // java implementation:                            http://stackoverflow.com/questions/17641706/
        // irrepressibly verbose:                          new AntBuilder().copy( todir: to ) { fileset( dir: from ) }
	return copyDir( from, to, from, to )
    }

    static def copy( String from, String to ) {
		copy( new File(from), new File(to) )
	}

    static def execute( String line, String wd, ArrayList env_param=null, time_out=timeout, throwexcep=false ){
		return execute( line, new File(wd), env_param, time_out, throwexcep )
	}

	static def getOsVersionNumber() {
		def version = (System.getProperty("os.version").split("\\.")[0..1]).join('.');
		return version;
	}

    //----
    //---- does not handle strings with pipes: "echo t | svn co ....."
    //----
    static def execute( String line, File wd=null, ArrayList env_param=null, long time_out=timeout, throwexcep=false ) {
        def ret = [ out: new StringBuffer( ), err: new StringBuffer( ) ]
        def env
        if ( env_param != null ) {
            env = System.getenv().collect { k, v -> "$k=$v" }
            env = env + env_param
        }

        def proc = line.execute(env,wd)
        proc.consumeProcessOutput( ret['out'], ret['err'] )
	if ( time_out > 0 ) {
	    proc.waitForOrKill(time_out)
	} else { proc.waitFor( ) }

	ret['status'] = proc.exitValue( )
	if ( ret['status'] != 0 ) {
	    if ( throwexcep ) throw new RuntimeException("execution of '${line}' exited abnormally")
	}

        return ret
        // proc.text.eachLine { x -> println "<hdiutil> ${x}" }
        //            proc.err.text.eachLine { x -> println "<hdiutil> ${x}" }
    }

	static void move( String from, String to, Boolean delete=false ) {
		move( Paths.get(from), Paths.get(to), delete )
	}
	static void move( File from, File to, Boolean delete=false ) {
		move( Paths.get("${from}"), Paths.get("${to}"), delete )
	}
	static void move( Path from, Path to, Boolean delete=false ) {
		assert Files.exists(from)
		// first try the simple, java move... if it succeeds great...
		// if it fails... it could mean that 'from' is on a different
		// filesystem than 'to'...
                if ( new File(from.toString()).renameTo(new File(to.toString( ))) ) { return }
		if ( Files.isDirectory(to) ) to = Paths.get(to.toString( ),from.getFileName( ).toString( ))
		if ( Files.exists(to) && delete ) {
			if ( Files.delete(to) == false ) throw new Exception("deletion of ${to} failed")
		}
		assert ! Files.exists(to)
		Files.copy(from,to)
		Files.delete(from)
	}
    static def createSymlink( File old_file, File new_file ) {

		assert old_file.exists( )
        assert ! new_file.exists( )

		if ( old_file.getParent( ) == new_file.getParent( ) ) {
            execute( "ln -s ${old_file.getName( )} ${new_file.getName( )}", old_file.getParent( ) )
        } else {
            execute( "ln -s ${old_file} ${new_file}" )
        }

		assert new_file.exists( )
    }

    static def createSymlink( String old_file, String new_file ) {
        createSymlink( new File(old_file), new File(new_file) )
    }

    static def createSymlink( String old_file, File new_file ) {
        createSymlink( new File(old_file), new_file )
    }

    static def createSymlink( File old_file, String new_file ) {
        createSymlink( old_file, new File(new_file) )
    }

	static def startXvfb( boolean debug=false ) {

		if ( osver_ == 'osx' ) throw new RuntimeException( "no Xvfb on OSX" )

		def xvfb = null
		def display = null
		def xauth_file = File.createTempFile('temp','.xauth')
		if ( debug ) println "xauthority file: ${xauth_file}"

		for ( disp in 4..99 ) {
			if ( debug ) println "trying to start xvfb on display :${disp}..."
			def cookie = Gen.randomHash('a'..'f','0'..'9')(32)
			def xauth = new ProcessBuilder("xauth", "-f", "${xauth_file}", "add", ":${disp}", ".", cookie).redirectErrorStream(true).start( )
			xauth.inputStream.eachLine { if ( debug ) println "        xauth> ${it}" }
			xvfb = new ProcessBuilder("Xvfb",":${disp}", "-screen", "0", "2048x2048x24+32", "-auth", "${xauth_file}").redirectErrorStream(true).start( )

			Thread.sleep(500)
			// first see if it's still alive
			try {
				int exitValue = xvfb.exitValue( )
				continue
			} catch (IllegalThreadStateException e) {  }

			// wait for some output to be available...
			// if Xvfb starts successfully, there may be no output...
			for ( c in 0..10 ) {
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
					else if ( debug ) println "         xvfb> ${line}"
				}
				if ( success ) {
					display = ":" + disp.toString() + ".0"
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
				display = ":" + disp.toString() + ".0"
				break
			}
			xvfb.waitForOrKill(1)
			xvfb = null
		}

		if ( xvfb != null ) addShutdownHook {
			println ""
			println "closing display ${display}"
			println "removing auth file ${xauth_file}"
			xvfb.waitForOrKill(1)
			xauth_file.delete( )
		}
		return [ 'proc': xvfb,  'auth': xauth_file, 'display': display ]
	}



    static def md5( File source, File output=null ) {
	assert source.isFile( )
        byte[] buffer = new byte[8192];
        def hash = java.security.MessageDigest.getInstance("MD5");

        def stream = new java.security.DigestInputStream(new FileInputStream(source), hash);
        try { while ( stream.read(buffer) != -1 ); }
	finally { stream.close( ) }

	def str = hash.digest( ).encodeHex( ).toString( ) + "  " + source.getName( )
	if ( output != null ) {
	    def w = output.newWriter( )
	    w << str << "\n"
	    w.close()
	}
	return str
    }

    // implicit all (output) => false
    static def untar( File src_distro, File output_dir, Closure action ) {
	return untar( src_distro, output_dir, false, action )
    }
    static def untar( File src_distro, File output_dir=null, Boolean all=false, Closure action=null ) {
	def proc = execute("tar -vxf ${src_distro}", output_dir, null, 0)
	(all ? proc.out + proc.err : proc.err).eachLine(action)
	return proc.status
    }


    static def tar( String src_dir, String output_file, String output_dir, Boolean all = false, Closure action=null ) {
	return tar( new File(src_dir), new File(output_file), output_dir != null ? new File(output_dir) : null, all, action )
    }

    static def tar( String src_dir, String output_file, File output_dir, Boolean all = false, Closure action=null ) {
	return tar( new File(src_dir), new File(output_file), output_dir, all, action )
    }

    // implicit all (output) => false
    static def tar( File src_dir, String output_file, File output_dir, Closure action ) {
	return tar( src_dir, new File(output_file), output_dir, false, action )
    }
    static def tar( File src_dir, String output_file, File output_dir, Boolean all = false, Closure action=null ) {
	return tar( src_dir, new File(output_file), output_dir, all, action )
    }

    static def tar( String src_dir, File output_file, File output_dir, Boolean all = false, Closure action=null ) {
	return tar( new File(src_dir), output_file, output_dir, all, action )
    }


    static def tar( File src_dir, File output_file, File output_dir=null, Boolean all = false, Closure action=null ) {

	assert src_dir.isDirectory( )
	def srcdir = src_dir.getParent( )
	def src = src_dir.getName( )

	assert output_file.getParent( ) != null || output_dir != null
	def out = output_file.getName( )
	def outdir = null
	if ( output_file.getParent( ) != null ) {
	    outdir = output_file.getParent( )
	    assert new File(outdir).isDirectory( )
	} else {
	    assert output_dir.isDirectory( )
	    outdir = output_dir.toString( )
        }

	def flags = [ {x->x.endsWith(".gz")?"-vzcf":null},
		      {x->x.endsWith(".tar")?"-vcf":null},
                      {x->x.endsWith(".bz2")?"-vjcf":null},
                      {x->x.endsWith(".xz")?"-vJcf":null}   ].inject(null) { result, check -> result != null ? result : check(output_file.toString( )) }

	assert flags != null

	println "                    ]]]]]]========>>> run: tar -C ${srcdir} ${flags} ${out} ${src}"
	println "                                       in: ${outdir}"
	def proc = execute("tar -C ${srcdir} ${flags} ${out} ${src}", outdir, null, 0)
	(all ? proc.out + proc.err : proc.err).eachLine(action)
	return proc.status
    }


    // should include a check to see if linux seems reasonable
	private static final Map arch_map_ = [ amd64: 'x86_64', x86_64: 'x86_64' ]
	private static final String osarch_ = arch_map_[System.getProperty("os.arch")]
    private static final def home_ = System.getProperty("user.home")
    private static final def username_ = System.getProperty("user.name")
    private static String hostname_ = null
    private static final String osver_ = System.getProperty("os.name").toLowerCase().contains('os x') ? 'osx' : 'linux'
    private static final String oslabel_ = getLabel();

	private static String getLabel( ver = os( ) ) {

                String label =""
                if  (ver == 'osx') {
                         label = ver + (System.getProperty("os.version").split("\\.")[0..1]).join('_')
                }
                else {
					// Parse release information from lsb_release command
					def getReleaseCmd = "lsb_release -r"// Create the String
					def getDescCmd = "lsb_release -d"// Create the String
					def relProc = getReleaseCmd.execute()                 // Call *execute* on the string
					def descProc = getDescCmd.execute()                 // Call *execute* on the string

					relProc.waitFor()                               // Wait for the command to finish
					descProc.waitFor()

					String releaseRaw=relProc.in.text
					String [] splitRelease = releaseRaw.split()[1].split("\\.")
					String majorVersion=splitRelease[0]
					String minorVersion=splitRelease[1]

					String descRaw=descProc.in.text
					String[] splitDesc = descRaw.split()

					if (splitDesc[1]=="Red" && splitDesc[2]=="Hat") {
							//println "Red Hat detected "
							//println majorVersion
							//println minorVersion
							if (majorVersion=="6") {
								label ="el6"
							}
							else if (majorVersion=="5") {
								label ="el5"
							}
					}
					else if (splitDesc[1]=="CentOS") {
						if (majorVersion=="6") {
							label ="el6"
						}
						else if (majorVersion=="5") {
							label ="el5"
						}

					}
					else {
						print "Currently only Red Hat is supported"
						label ="Unknown"
					}


                }
                return label;
    }

    private static def copyDir( File from, File to, File orig_from, File orig_to ) {
        if ( from.isDirectory( ) ) {
            if ( ! to.exists( ) ) {
                to.mkdir( )
            } else if ( ! to.isDirectory( ) ) {
                to.delete( )
                to.mkdir( )
            }

            def result = [ ]
	    from.eachFile { def match = it.getPath( ) =~ ~/^${orig_from.getPath( )}(.*)/
			    if ( match.size( ) > 0 ) {
			        def sub = match[0][1]
                                result += copyDir( it, new File( orig_to, sub ), orig_from, orig_to )
                            } }
            return result

        } else {

            to.createNewFile( )
            to.bytes = from.bytes
            return [ "${to}" ]

        }
    }
}
