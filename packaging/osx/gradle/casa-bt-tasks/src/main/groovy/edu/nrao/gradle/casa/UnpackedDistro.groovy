package edu.nrao.gradle.casa
import groovy.io.FileType
import edu.nrao.gradle.casa.Sys
import edu.nrao.gradle.casa.Convert
import edu.nrao.gradle.casa.Repo
import edu.nrao.gradle.casa.PackedDistro

import java.nio.ByteBuffer
import java.nio.CharBuffer
import java.io.ByteArrayOutputStream
import java.nio.charset.Charset
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharacterCodingException;
import java.nio.file.*;
import static groovy.io.FileType.*
class UnpackedDistro {

	def testStatus( boolean print_output = false ) {
		if ( regression_result_.containsKey('execution_result') ) {
			if ( regression_result_['execution_result'].containsKey('status') ) {
				if ( regression_result_['execution_result']['status'] == 0 ) {
					if ( testOK( ) ) {
						if ( print_output ) println "successful test run"
						return [true,"success"]
					} else {
						if ( print_output ) println "failed test run"
						return [false,"failure"]
					}
				} else {
					if ( print_output ) println "bad exit status from test run"
					return [false,"bad exit"]
				}
			} else {
				if ( print_output ) println "no status found for test run"
				return [false,"no status"]
			}
		} else {
			if ( print_output ) println "no info found for test run"
			return [false,"no info"]
		}
	}

    static UnpackedDistro create( Object app, Object work_dir=null ) {

        def os = Sys.os( )
        if ( app == null ) {
            def binPath;
            if (os == "osx") {
                binPath="https://svn.cv.nrao.edu/casa/${os}_distro/test/${Sys.getOsVersionNumber()}"
            } else {
                binPath="https://svn.cv.nrao.edu/casa/${os}_distro/test/${Sys.osLabel( )}"
            }

            def binary = new Repo(binPath).current( )
            return binary.unpack( work_dir )
        } else if ( app instanceof String ) {
            String s = (String) app
            File f = new File(s)
            if ( f.exists() ) {
                return create(f)
            } else {
                URL u = new URL(s)
                return create(u)
            }
        } else if ( app instanceof File ) {
            File f = (File) app
            assert f.isDirectory( )
            if ( os == "osx" && f.getName( ) == "CASA.app" ) {
                return new UnpackedDistro(f,"test")
            } else if ( os == "linux" && (new File(f,"bin/casa-config").exists( ) || new File(f,"casa-config").exists( )) ) {
                def config = new File(f,"bin/casa-config")
                if ( ! config.exists( ) )
                    config = new File(f,"casa-config")
                assert config.canExecute( )
                return new UnpackedDistro(f,"test")
            } else {
                def binary = new Repo(f).current( )
                return binary.unpack( work_dir )
            }
        } else if ( app instanceof URL ) {
            def binary = new Repo((URL)app)
            return binary.unpack( work_dir )
        } else {
            throw new RuntimeException("UnpackedDistro.create( ) passed unknown type")
        }
    }

    UnpackedDistro( File p, String type = "prerelease", String os_root_ = Sys.osLabel( ), os = Sys.os( ) ) {
        assert p.isDirectory( );
        distro_type_ = type
        distro_path_ = p
        distro_os_ = os
        regression_os_root_ = os_root_
        def config = null
        if ( distro_os_ == 'osx' )
            config = "${distro_path_}/Contents/MacOS/casa-config"
        else {
            // linux...
            config = ["bin/casa-config","casa-config"].inject(null) {
                result,i -> result != null ? result : new File("${distro_path_}/${i}").exists( ) ? "${distro_path_}/${i}" : null
            }
            assert config != null
        }
        distro_config_ = new File(config)
        assert distro_config_.canExecute( )
        distro_testdir_ = new File("${distro_path_}/testing")
        load_revstate_( )
        install_pybot_( )
    }
    UnpackedDistro( String s, String type = "prerelease", os = Sys.os( ) ) { this( new File(s), os ) }

    // Hack to add different style of revision number to the link
    String prereleaseRevisionNumber=0;
    public setPrereleaseRevisionNumber (String prereleaseRevisionNumber ) {
            this.prereleaseRevisionNumber=prereleaseRevisionNumber
	}
    //end hack

    def runRegression( String tag="smoke",
                       Object output_root=distro_testdir_,
                       File testing_root = distro_testdir_,
                       File work_root = new File( "${distro_testdir_}/work" ) ) {
		return runTestAction( tag, output_root, testing_root, work_root ) {
			test, ver, rev, wrapper, test_dir, output_dir, env ->
				println "::1        test: ${test}"
				println "::2         ver: ${ver}"
				println "::3         rev: ${rev}"
				println "::4     wrapper: ${wrapper}"
				println "::5    test_dir: ${test_dir}"
				println "::6  output_dir: ${output_dir}"
				println "::7         env: ${env}"

				// trying to use single quotes to make the titles a single argument is hopeless...
				def title = "CASA ${ver} (${rev})"
				if ( distro_type_=="prerelease" ) {
					title = "CASA ${distro_ver_}-${prereleaseRevisionNumber} (${distro_rev_})"
				}
				println ">>>>>>-----> cmd: ${wrapper} --exec pybot --outputdir ${output_dir} --logtitle \"${title} Log\" --reporttitle \"${title} Report\" --include ${test} casa"
				return execute( "${wrapper} --exec pybot --outputdir ${output_dir} --logtitle \"${title} Log\" --reporttitle \"${title} Report\" --include ${test} casa", test_dir, env, 0 )
		}

	}

    // required for runTests("vishead,foo,bar".split(/\s*,\s*/))
    def runTests( String [] tests,
				  Object output_root=distro_testdir_,
				  File testing_root = distro_testdir_,
				  File work_root = new File( "${distro_testdir_}/work" ) ) {
        return runTests( tests.inject(new ArrayList()) { acc,v -> acc.push(v); acc },
                         output_root, testing_root, work_root )
    }

    def runTests( List tests,
				  Object output_root=distro_testdir_,
				  File testing_root = distro_testdir_,
				  File work_root = new File( "${distro_testdir_}/work" ) ) {
		return runTestAction( tests, output_root, testing_root, work_root ) {
			test, ver, rev, wrapper, test_dir, output_dir, env ->
                def os = Sys.os( )
				def test_args = null
                if ( os == 'linux' ) test_args = tests.collect { "--test \\\\*${it}\\\\*" }.join(" ")
                else test_args = tests.collect { "--test \\*${it}\\*" }.join(" ")
                assert test_args != null
				println "::1        test: ${test}"
				println "::2         ver: ${ver}"
				println "::3         rev: ${rev}"
				println "::4     wrapper: ${wrapper}"
				println "::5    test_dir: ${test_dir}"
				println "::6  output_dir: ${output_dir}"
				println "::7         env: ${env}"
				println "::    test args: ${test_args}"

				// trying to use single quotes to make the titles a single argument is hopeless...
				def title = "CASA ${ver} (${rev})"
				println ">>>>>>-----> cmd: ${wrapper} --exec pybot --outputdir ${output_dir} --logtitle \"${title} Log\" --reporttitle \"${title} Report\" ${test_args} casa"
				return execute( "${wrapper} --exec pybot --outputdir ${output_dir} --logtitle \"${title} Log\" --reporttitle \"${title} Report\" ${test_args} casa", test_dir, env, 0 )
		}

	}

    //$ENV{'__PYBOT_CASA_PRECONFIG__'} = "$ver $rev https://svn.cv.nrao.edu/prerelease-tests/$rhver/$version$extra_num prerelease";
    //$apache_output_root = '/home/svn.cv.nrao.edu/spool/prerelease-tests';
    //$casa_output_root = '/home/rpmbuild/rpmbuild/apache/content/casa';
    //$tar_output_dir = "$casa_output_root/linux_distro/prerelease/$rhver";
    //$rpm_output_dir = "$casa_output_root/repo/$rhver/x86_64";
    //my $outputdir = "$apache_output_root/$rhver/$ver$extra_num" . ($tag ? ".$tag" : "");
    //$title  = "CASA $ver ($rev)";
    //@titles = ( '--logtitle',"$title Log",'--reporttitle',"$title Report" );
    //@tags = ( $tag ? ('--include',$tag) : ('--include','regression') );
    //@cmd = ('casa-config','--exec','pybot','--outputdir',"$root/output",@titles,@tags,'casa');
    private def runTestAction( def tag, Object output_root, File testing_root,
							   File work_root, Closure action ) {

        output_root = Convert.asDir(output_root)
		println '>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<'
		println ">>>>----A> runTestAction(${regression_result_.dump()})"
        //regression_result_ = [:] //<<<<<<<<<< this would loose our original object (as does regression_result_.clear())
        regression_result_['tag'] = tag
        regression_result_['output_root'] = output_root

		println ">>>>----B> runTestAction(${regression_result_.dump()})"

        create_dir_( testing_root )
        //----------------------------------------------------------------------------------------------
        // directories needed by the pybot for regression runs...
        create_dir_( new File("${work_root}/run/build") )
        create_dir_( new File("${work_root}/run/testing") )
        //----------------------------------------------------------------------------------------------
		String oDir="${output_root}/${regression_os_root_}/${distro_ver_}"
		if (distro_type_=="prerelease") {
		    oDir="${output_root}/${regression_os_root_}/${distro_ver_}-${distro_rev_}"
		}
        def outputdir = new File(oDir)

        create_dir_( outputdir )
        def casa_regress = checkout_regression_( testing_root )
		['linux-casa','CasaRegression.py','osx-casa','linux-casa','run-casa','casapy.py'].each {
			def from = new File("${Sys.home( )}/scripts/${it}")
			if ( from.isFile( ) ) {
				def to = ['bin','lib','lib/python'].findResult {
					sub ->  ["${testing_root}/casa","${distro_path_}"].findResult {
						root -> def to = new File("${root}/${sub}/${from.getName( )}")
						        to.isFile( ) ? to : null
					}
				}
				if ( to != null ) {
					println "substituting ${from}"
					Files.copy(from.toPath( ),to.toPath( ),StandardCopyOption.REPLACE_EXISTING)
				}
			}
		}

        def regression_env = [ "__PYBOT_CASA_PRECONFIG__=${distro_ver_} ${distro_rev_} https://svn.cv.nrao.edu/${distro_type_}-tests/${regression_os_root_}/${distro_ver_} ${distro_type_}", "CASAROOT=${distro_path_}" ] + execute_env_.collect { k, v -> "$k=$v" }
		// Add the revision for prerelease
		if (distro_type_=="prerelease") {
			 regression_env = [ "__PYBOT_CASA_PRECONFIG__=${distro_ver_} ${distro_rev_} https://svn.cv.nrao.edu/${distro_type_}-tests/${regression_os_root_}/${distro_ver_}-${prereleaseRevisionNumber} ${distro_type_}", "CASAROOT=${distro_path_}" ] + execute_env_.collect { k, v -> "$k=$v" }
		}
		println ">>>>>>-----> env: ${regression_env}"
        def replacement = new File( "${Sys.home( )}/CasaRegression.py" )
        if ( replacement.isFile() ) {
            find( casa_regress, FileType.FILES ) {
                it ==~ ~/.*\/CasaRegression.py/
            } { x -> x.delete( )
                     x << replacement.text
            }
        }

        // replace the hard-coded work root...
        find( casa_regress, FileType.FILES ) {
            it ==~ ~/.*\/__init__.html/
        } { x ->    def text = x.text
                    text = text.replaceAll( '/home/warp/casa/jenkins/work-pybot', work_root as String )
                    x.write(text)
        }

		def result = action( tag, distro_ver_, distro_rev_, distro_config_, testing_root, outputdir, regression_env )

        result.out.eachLine {
            x -> println "${x}"
                 def match = x =~ ~/^(Output|Log|Report):\s+(.*)/
                     if ( match.size( ) > 0 ) { regression_result_[match[0][1].toLowerCase( )] = match[0][2] }
                 else {
                     match = x =~ ~/^\s*(\d+)\s+tests\s+total\s*,\s+(\d+)\s+passed\s*,\s+(\d+)\s+failed\s*/
                     if ( match.size( ) > 0 ) {
                         regression_result_['total'] = match[0][1]
                         regression_result_['pass'] = match[0][2]
                         regression_result_['fail'] = match[0][3]
                     }
                 }
        }
		if ( result.err.length( ) > 0 ) {
			println "---stderr------------------------------------------------------------------"
			result.err.eachLine { println "error: test>\t${it}" }
			println "---stderr------------------------------------------------------------------"
		}

		def report = new File(outputdir,"report.html")
		if ( report.isFile( ) ) {
			def link = new File(outputdir,"index.html")
			if ( ! link.exists( ) ) {
				println "creating symbolic link: ${link}"
				Sys.createSymlink(report,link)
			} else println "symbolic link already exists: ${link}"
		}

		regression_result_['execution_result'] = result
        return result['status'] == 0
    }


    def installRegressionLog( File path ) {
	    println "Installing logs to: " + path.toString()
	    println "Distro version: ${distro_ver_} ";

		if ( regression_result_.size( ) <= 2
			 || ! regression_result_.containsKey('tag')
			 || ! regression_result_.containsKey('output_root') ) {
			 return false
	    }

		def output_root = regression_result_['output_root']
	    println "output_root: " + output_root

		// Default logdirName
		String logdirPath = "${output_root}/${regression_os_root_}/${distro_ver_}"

		// If we are running pre-release, add the revision number in the end
		if (distro_type_=="prerelease") {
		    logdirPath = "${output_root}/${regression_os_root_}/${distro_ver_}-${distro_rev_}";
		}

		def logdir = new File(logdirPath)

        if (!logdir.isDirectory()) {
			return false
		}

        def tag = regression_result_['tag']
        def root = new File( "${path}/${regression_os_root_}" )
        if ( root.exists( ) && ! root.isDirectory( ) ) { root.delete( ) }
        if ( ! root.exists( ) ) { root.mkdirs( ) }
		String cpPath = "${root}/${distro_ver_}"
		if (distro_type_=="prerelease") {
			cpPath = "${root}/${distro_ver_}-${distro_rev_}";
		}
        Sys.copy( logdir, new File( cpPath  + (tag == 'regression' ? '' : ".${tag}") ) )/*.each {
            def match = it =~ ~/(.*)\/report.html$/
            if ( match.size( ) > 0 ) { println ">>>>>>>>>>>> ${it}"; Sys.createSymlink( it, "${match[0][1]}/index.html" )  } }*/
        return true
    }

    def installRegressionLog( String path ) { installRegressionLog( new File(path) ) }

	def installPrereleaseRegressionLog( File path, String inRevision ) {
		println "Installing logs to: " + path.toString()
		println "Distro version: ${distro_ver_} ";

		if ( regression_result_.size( ) <= 2
			 || ! regression_result_.containsKey('tag')
			 || ! regression_result_.containsKey('output_root') ) {
			 return false
		}

		def output_root = regression_result_['output_root']
		println "output_root: " + output_root

		// Default logdirName
		String logdirPath  = "${output_root}/${regression_os_root_}/${distro_ver_}-${distro_rev_}";

		def logdir = new File(logdirPath)

		if (!logdir.isDirectory()) {
			return false
		}

		def tag = regression_result_['tag']
		def root = new File( "${path}/${regression_os_root_}" )
		if ( root.exists( ) && ! root.isDirectory( ) ) { root.delete( ) }
		if ( ! root.exists( ) ) { root.mkdirs( ) }
		String cpPath = "${root}/${distro_ver_}"
		if (distro_type_=="prerelease") {
			cpPath = "${root}/${distro_ver_}-${inRevision}";
		}
		Sys.copy( logdir, new File( cpPath  + (tag == 'regression' ? '' : ".${tag}") ) )/*.each {
			def match = it =~ ~/(.*)\/report.html$/
			if ( match.size( ) > 0 ) { println ">>>>>>>>>>>> ${it}"; Sys.createSymlink( it, "${match[0][1]}/index.html" )  } }*/
		return true
	}

	def installPrereleaseRegressionLog( String path, String inRevision ) { installPrereleaseRegressionLog( new File(path), inRevision ) }

	def testOK() {

		def output_root = regression_result_['output_root']
		ResultXml xml = new ResultXml();
		println "        output_root: ${output_root}"
		println "regression_os_root_: ${regression_os_root_}"
		println "        distro_ver_: ${distro_ver_}"
		String logXmlFile = "${output_root}/${regression_os_root_}/${distro_ver_}/output.xml"

		// If we are running pre-release, add the revision number in the end
		if (distro_type_=="prerelease") {
			logXmlFile = "${output_root}/${regression_os_root_}/${distro_ver_}-${distro_rev_}/output.xml";
		}
		println "Checking logXmlFile: " + logXmlFile
		return xml.testOK(logXmlFile)
	}

    def useData( File complete_repository ) {
        assert complete_repository.isDirectory( )
        def oldData = new File(distro_os_ == 'osx' ? "${distro_path_}/Contents/Resources/casa-data" : "${distro_path_}/data")
        if (oldData.exists()) {
			Path p = oldData.toPath()
			println "Checking if data path is a symbolic link."
			Boolean isLink =Files.isSymbolicLink(p)
			if (isLink) {
				println "Deleting symbolic link: " + oldData
				oldData.delete()
			}
            else if (oldData.isDirectory()) {
				println "Deleting directory: " + oldData
				oldData.deleteDir()
			}
            else {
				println "Deleting file: " + oldData
				oldData.delete()
			}
        }
        println "using ${complete_repository} as the data for running regressions"
        Sys.createSymlink( complete_repository, oldData )
        return this
    }

    def useData( String complete_repository ) { useData( new File(complete_repository) ) }

    private void fixShebangOverflow( ) {
        find( distro_path_, FileType.FILES ) {
            FileInputStream stream = new FileInputStream(it);
            // i.e. #!
            boolean is_script = stream.read( ) == 0x23 && stream.read( ) == 0x21;
            def line = new ByteArrayOutputStream( )
            while ( is_script ) {
                int cur = stream.read( )
                // i.e. \n
                if ( cur == 0x0a ) break
                // i.e. eof
                if ( cur == -1 ) { 
                    is_script = false
                    break
                }
                line.write(cur)
            }
            stream.close( )

            // make sure the first line (at least) is all ascii...
            // in case binary begins with '#!'...
            try {
                CharsetDecoder d = Charset.forName("US-ASCII").newDecoder();
                CharBuffer r = d.decode(ByteBuffer.wrap(line.toByteArray( )));
                r.toString();
            } catch(CharacterCodingException e) { return null }

            return is_script && line.size( ) > 127 ? new Tuple(it,line.toString( )) : null
        }.each {
		   	def wrapped = new File("${it[0]}__")
			// Added a check to see if the wrapped file already exists.
			// This is to stop the file being overwritten with the parent's contents
			// when traversing through the circular symlinks on OS X
			if (!wrapped.exists()) {
	           	it[0].renameTo("${wrapped}")
	           	it[0] << """#!/bin/bash
	           	           |exec "${it[1]}" "${wrapped}" "\$@"
	           		       |""".stripMargin( )
	           	it[0].setExecutable(true)
	           	wrapped.setExecutable(false)
			}
        }
    }

    def find( Closure predicate, Closure action = null, File path = distro_path_, file_type = null ) {
        def result = [ ]
        if ( file_type == null ) {
            distro_path_.eachFileRecurse( ) {
                Object v = predicate(it)
                if ( v instanceof java.lang.Boolean ) {
                    if ( v ) {
                        result << it
                        if ( action != null ) { action(it) }
                    }
                }  else if ( v != null ) { 
                    result << v
                    if ( action != null ) { action(it) }
                }
            }
        } else {
			// Using traverse with maxDepth to fix a problem with OS X circular symlinks.
			// eachFileRecurse will not stop at circular symlinks at all and traverse
			// seems to only stop at a given depth.
		    distro_path_.traverse(type: FILES, maxDepth:20) {
            //distro_path_.eachFileRecurse(file_type) {
                Object v = predicate(it)
                if ( v instanceof java.lang.Boolean ) {
                    if ( v ) {
                        result << it
                        if ( action != null ) { action(it) }
                    }
                } else if ( v != null ) {
                    result << v
                    if ( action != null ) { action(it) }
                }
            }
        }
        return result
    }

    def find( File path, Closure predicate, Closure action = null, file_type = null ) {
        find( predicate, action, path )
    }

    def find( File path, file_type, Closure predicate, Closure action = null ) {
        find( predicate, action, path, file_type )
    }

    def execute( def line, File wd=null, ArrayList env=null, time_out=Sys.timeout ) {
        Sys.execute( "${distro_config_} --sh-exec ${line}", wd, env, time_out )
    }

    def delete( ) { assert distro_path_.deleteDir( ) }

    Object asType(Class clazz) {
        if (clazz == File ) {
            new File(distro_path_.getPath())
        }
        else { super.asType(clazz) }
    }

    String toString( ) { return distro_path_.toString( ) }

    private def create_dir_( File path ) {
        if ( ! path.exists( ) ) {
            path.mkdirs( )
        }
        assert path.isDirectory( )
    }

	UnpackedDistro env( Map env_vars ) {
		def ret = new UnpackedDistro( this, env_vars )
		println ">>>>----> env(${ret.regression_result_.dump( )}): ${regression_result_.dump( )}"
		return ret
	}


    private def checkout_regression_( File testdir ) {
		if (regression_url_.startsWith("http")) {
			Sys.execute( "svn co --trust-server-cert --non-interactive ${regression_url_} casa", testdir )
		}
		else {
			println "regression_url_ does not start with http. Assuming this is a local directory. "
			File destinationDir = new File (testdir.toString() + "/casa")
			File sourceDir = new File (regression_url_ + "/casa")
			destinationDir.mkdirs()
			new AntBuilder().copy(todir: destinationDir) {
				fileset(dir: sourceDir, includes: "**")
			}
		}
		
		def regressiondir = new File( "${testdir}/casa" )
        assert regressiondir.isDirectory( )
        return regressiondir
    }
    private def load_revstate_( ) {
        def revstate = new File( distro_os_ == "osx" ? "${distro_path_}/Contents/Resources/revstate" : "${distro_path_}/.revstate" )
        assert revstate.isFile( )
        def ver, rev
        revstate.eachLine {
            x -> def match = x =~ ~/revision\s*=\s*"(\d+)"/
                 if ( match.size( ) > 0 ) { rev = match[0][1] }
                 match = x =~ /version\s*=\s*"(\d+\.\d+\.\d+)"/
                 if ( match.size( ) > 0 ) { ver = match[0][1] }
        }
        assert ver != null && rev != null
        distro_ver_ = ver
        distro_rev_ = rev
        // Originally, the svn revision was used as the prerelease rev... this was retrieved
        // from the "revstate" file, which is created when the RPM is created...
        // Now, the RPM revision count is used... this can be recovered on linux from the
        // path to the unpacked release... however, I don't know how this should be done
        // on OSX... prehaps ths count sould be included in the "revstate" file... and it
        // just should seldom change for the typical test/stable versions...
        if ( distro_os_ == "linux" && "${distro_path_}" ==~ ~/.*-\d+\.\d+\.\d+-\d+$/ ) {
            def match = "${distro_path_}" =~ ~/.*-\d+\.\d+\.\d+-(\d+)$/
            distro_rev_ = match[0][1]
        }
    }
    private def install_pybot_( ) {
        create_dir_(distro_testdir_)
        Sys.execute( "svn co --trust-server-cert --non-interactive ${pybot_url_} robotframework", distro_testdir_ )
        def pybotsrcdir = new File( "${distro_testdir_}/robotframework" )
        assert pybotsrcdir.isDirectory( )
        Sys.execute( "${distro_config_} --exec python setup.py install", pybotsrcdir )
        assert pybotsrcdir.deleteDir( )
        // bash line limits for #! is 128 chars...
        // pybot/rebot use the path to casa's python interpreter in the #! line...
        // ...whose path I've found can exceed 128 chars...
        fixShebangOverflow( )
    }

	private UnpackedDistro( UnpackedDistro other, Map env_vars ) {
		distro_path_ = other.distro_path_							// java.lang.CloneNotSupportedException: java.io.File
		distro_type_ = other.distro_type_
		distro_config_ = other.distro_config_
		distro_testdir_ = other.distro_testdir_
		distro_ver_ = other.distro_ver_
		distro_rev_ = other.distro_rev_
		distro_os_ = other.distro_os_
		regression_os_root_ = other.regression_os_root_
		regression_result_ = other.regression_result_
		execute_env_ = other.execute_env_ + env_vars
		println ">>>>----> UnpackedDistro(${other.regression_result_.dump()}): ${regression_result_.dump()}}"
	}

    private def distro_path_
    private def distro_type_
    private def distro_config_
    private def distro_testdir_
    private def distro_ver_
    private def distro_rev_
    private def distro_os_
    private def regression_os_root_
	//    private def regression_result_ = new HashMap( )      //=====>>> groovy's [:] is a null singleton
    //    private def regression_result_ = [__id__: (Math.abs(new Random().nextInt()) % 600 + 1).toString() ]      //=====>>> groovy's [:] is a null singleton
	private def regression_result_ = [:]
	private def execute_env_ = [:]
    private static final def pybot_url_   = "https://svn.cv.nrao.edu/svn/casa/development_tools/testing/pybot"
	// ---------------- URL used for nightly regression runs ----------------
    private static regression_url_ = "https://svn.cv.nrao.edu/svn/casa/development_tools/testing/pybot-regression"
	
	public void setRegressionUrl (String  regUrl) {
		regression_url_=regUrl
	}
	
}
