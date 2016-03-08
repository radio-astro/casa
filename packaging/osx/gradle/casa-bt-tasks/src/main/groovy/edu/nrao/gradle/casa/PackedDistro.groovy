package edu.nrao.gradle.casa
import groovy.io.FileType
import edu.nrao.gradle.casa.Sys
import edu.nrao.gradle.casa.Convert
import edu.nrao.gradle.casa.UnpackedDistro
import edu.nrao.gradle.casa.Version
import edu.nrao.gradle.casa.Rpm
import java.nio.file.*;

class PackedDistro { 
	
	// Convenient way of getting file location for the downloaded & unpacked package
	
	File myPath;
	public File getMyPath() {
		return myPath;
	}

    // pattern to recognize a casa distro
    static def pattern( type, osver = Sys.os( ), osarch = Sys.arch( ), boolean end_user=true ) {
		def oslabel = Sys.getLabel(osver)
        switch ( "${osver}-${type}" ) { 
        //switch ( "linux-${type}" ) { 
			case "linux-prerelease": return end_user ? \
			                                ~/^casa-${type}.*(\d+\.\d+\.\d+)-(\d+).*\.tar.gz$/ : \
			                                ~/^casa-${type}(?:-(\w+))?-(\d+\.\d+\.\d+)-(\d+)\.${oslabel}\.${osarch}\.rpm$/
            case "linux-test": return end_user ? \
			                                ~/^casa-${type}.*(\d+\.\d+\.\d+).*\.tar.gz$/ : \
			                                ~/^casa-${type}(?:-(\w+))?-(\d+\.\d+\.\d+)-(\d+)\.${oslabel}\.${osarch}\.rpm$/
            case "linux-stable": return end_user ? \
			                                ~/^casa-${type}.*(\d+\.\d+\.\d+).*\.tar.gz$/ : \
			                                ~/^casa-${type}(?:-(\w+))?-(\d+\.\d+\.\d+)-(\d+)\.${oslabel}\.${osarch}\.rpm$/
            case "osx-prerelease": return ~/^CASA-${type}-(\d+\.\d+\.\d+).*-(\d+)\.dmg$/
            case "osx-test": return ~/^CASA-${type}-(\d+\.\d+\.\d+)(?:-(\d+))?.*\.dmg$/
            case "osx-stable": return ~/^CASA-${type}-(\d+\.\d+\.\d+)(?:-(\d+))?.*\.dmg$/
        }

        throw Exception( "OS/repo-type error" )
    }

    // convert a casa distro version number to a numeric value, e.g. 4.4.19 to 4004019
    static def numeric( n ) {
        def m = n =~ ~/(\d+)\.(\d+)\.(\d+)/
        return m[0][1].toInteger() * 1000 ** 2 + m[0][2].toInteger() * 1000 + m[0][3].toInteger()
    }

    PackedDistro( File p, os_label = Sys.osLabel( ), os = Sys.os( ), os_arch = Sys.arch( ) ) { 
        distro_path_ = p
        regression_os_label_ = os_label
		regression_os_arch_ = os_arch
        distro_os_ = os
        assert distro_path_.isFile( )

        ctor_set_version_and_type_( distro_path_.getName( ), os )

//        def elem = distro_path_.getName( )
//      def distro_type = elem =~ ~/(?i)casa\-(\w+)/
//      assert distro_type[0].size( ) == 2
//      distro_type_ = distro_type[0][1]
//      def distro_version = elem =~ pattern(distro_type_,distro_os_)
//      assert distro_version[0].size( ) >= 2
//      distro_ver_ = distro_version[0][1]
    }
    PackedDistro( URL p, os_label = Sys.osLabel( ), os = Sys.os( ), os_arch = Sys.arch( ) ) {
        distro_path_ = p
        distro_os_ = os
        regression_os_label_ = os_label
		regression_os_arch_ = os_arch
        def elem_match = distro_path_ =~ ~/[^\/]+$/
        assert elem_match.size( ) == 1

        ctor_set_version_and_type_( elem_match[0], os )

//        def elem = elem_match[0]
//      def distro_type = elem =~ ~/(?i)casa\-(\w+)/
//      assert distro_type[0].size( ) == 2
//      distro_type_ = distro_type[0][1]
//      def distro_version = elem =~ pattern(distro_type_,distro_os_)
//      assert distro_version[0].size( ) >= 2
//      distro_ver_ = distro_version[0][1]
    }

    PackedDistro( String s, os_label = Sys.osLabel( ), os = Sys.os( ), os_arch = Sys.arch( ) ) {
        this ( new File(s).isFile( ) ? new File(s) : new URL(s), os_label, os, os_arch )
    }

    UnpackedDistro unpack( Object outdir = null ) {
        return unpack_( distro_path_, Convert.asDir(outdir,true,true) )
    }

	List<Rpm> fetchRpms( String p ) {
		if ( ! output_directory_.isDirectory( ) )
			throw Exception( "output directory (${output_directory_}) does not exist [unpack(...) prior to fetchRpms(...)]" )
		return new File(p).isDirectory( ) ? fetchRpms( new File(p) ) : fetchRpms( new URL(p) )
	}

	List<Rpm> fetchRpms( URL path ) {
		List<Rpm> result = [ ]
		if ( ! output_directory_.isDirectory( ) )
			throw Exception( "output directory (${output_directory_}) does not exist [unpack(...) prior to fetchRpms(...)]" )
		println "fetching RPMs from a URL: ${path}"
		path.eachLine {
			l ->
				((l =~ ~/href="(\S+?)"/).collect{ it[1] }).grep(pattern(distro_type_,distro_os_,regression_os_arch_,false)).each{
					x ->
						def match = x =~ pattern(distro_type_,distro_os_,regression_os_arch_,false)
						if ( distro_ver_[0] == match[0][2] ) {
							def outfile = new File( "${output_directory_}/${x}" )
							def sink = new FileOutputStream( outfile )
							def buffer = new BufferedOutputStream( sink )
							URL url = new URL(x.startsWith("http") ? x : path.toString( ) + "/${x}")
							buffer << url.openStream( )
							buffer.close( )
							result << new Rpm(outfile)
						}
				}
		}
		return result
	}

	List<Rpm> fetchRpms( File path ) {
		List<Rpm> result = [ ]
		if ( ! output_directory_.isDirectory( ) )
			throw Exception( "output directory (${output_directory_}) does not exist [unpack(...) prior to fetchRpms(...)]" )
		println "fetching RPMs from a file: ${path}"
		path.eachFileMatch(FileType.FILES,pattern(distro_type_,distro_os_,regression_os_arch_,false)) {
			x -> def match = x.getName( ) =~ pattern(distro_type_,distro_os_,regression_os_arch_,false)
				 if ( distro_ver_[0] == match[0][2] ) {
					def outfile = new File( "${output_directory_}/${x.getName( )}" )
                    if ( outfile.exists( ) ) outfile.delete( )
					Files.copy(x.toPath( ),outfile.toPath( ))
					result << new Rpm(outfile)
				}
		}
		return result
	}

    def repackage( String output_dir, Closure cvt ) {
	repackage( new File(output_dir), cvt )
    }
    def repackage( File output_dir, Closure cvt ) {
	File tmpdir = File.createTempDir( )
	repackage_( distro_path_, output_dir, cvt, tmpdir )
	Sys.rmdir(tmpdir)
    }
    private String commonprefix(String a, String b) {
	int len = Math.min(a.length(), b.length());
	for (int i = 0; i < len; i++) {
	    if (a.charAt(i) != b.charAt(i)) {
	        return a.substring(0, i);
	    }
	}
	return a.substring(0, len);
    }
    private def repackage_( File src_distro, File output_dir, Closure cvt, File tmpdir ) {
	assert tmpdir.isDirectory( )
	assert src_distro.isFile( )
	String path = null
	Sys.untar(src_distro,tmpdir,true) {
	    if ( path == null ) path = it
	    else path = commonprefix(path,it)
        }
	assert path.length( ) > 3
	File oldd = new File("${tmpdir}/${path}")
	assert oldd.isDirectory( )
	String news = cvt("${path}")
	File newd = new File("${tmpdir}/${news}")
	Sys.move(oldd,newd)
	def oldfn = new File("${distro_path_}").getName( )
	def newfn = cvt(oldfn)
	Sys.tar(newd,newfn,output_dir) { println "                        [${it}]" }
	def result_file = new File("${output_dir}/${newfn}")
	assert result_file.isFile( )
	def md5_file = new File("${output_dir}/${newfn}.md5")
	Sys.md5(result_file,md5_file)
	assert md5_file.isFile( )
	println "output distribution: ${result_file}"
	println "           checksum: ${md5_file}"

    }
    private def repackage_( URL src_distro, File output_dir, Closure cvt, File tmpdir ) {
	// still to be implemented...
	// by the time we get to repackaging the binary... we should have already
	//     downloaded the original binary distribution (which will be repackaged)...
	assert tmpdir.isDirectory( )
	println "--------------------------------->>>>>>>>> ${output_dir}"
	println "URL                    ---------->>>>>>>>> ${src_distro}"
	println "                       ---------->>>>>>>>> ${tmpdir}"
    }

    PackedDistro md5( ) {
        def md5_file = new File("${distro_path_}.md5")
        Sys.md5(distro_path_,md5_file)
        println "output distribution: ${distro_path_}"
        println "           checksum: ${distro_path_}.md5"
        return this        
    }

	PackedDistro copy( String s ) { return copy(new File(s)) }
	PackedDistro copy( Path p ) { return copy(p.toFile( )) }
	PackedDistro copy( File to ) {
		assert to.isDirectory( ) : "target directory, ${to}, does not exist"
        // need to add check for distro_path_ URL 
        assert distro_path_ instanceof File
		Path to_path = to.toPath( ).resolve(distro_path_.getName( ))
        Files.deleteIfExists(to_path)
		Files.copy( distro_path_.toPath( ), to_path )
		return new PackedDistro(to_path.toString(),regression_os_label_,distro_os_,regression_os_arch_)
	}

    def cleanup( ) {  }

    String toString( ) { return distro_path_.toString( ) }
    Integer toInteger( ) { return numeric(distro_ver_) }
	Version version( ) { return new Version(distro_ver_) }


    private def unpack_( File path, File outdir ) {
		myPath = path;
		output_directory_ = outdir
        if ( distro_os_ == 'osx' ) {
            def top = new File("${outdir}/casa-pkg")
            def mnt = new File("${top}/image")
            mnt.mkdirs( )
            Sys.execute("hdiutil attach ${path.getName( )} -mountpoint ${mnt}",path.getParentFile())
            def casa_app = new File("${mnt}/CASA.app")
            if ( ! casa_app.isDirectory( ) ) { throw Exception("casa application does not exist: ${casa_app}") }
            
			def target = new File("${top}/CASA.app")
			
			println "Removing casa-data link if one exists"
			def oldData = new File(distro_os_ == 'osx' ? "${target}/Contents/Resources/casa-data" : "${target}/data")
			println "Checking for link at: " + oldData
			if (oldData.exists()) {
				Path p = oldData.toPath()
				println "Checking if oldData path is a symbolic link."
				Boolean isLink =Files.isSymbolicLink(p)
				if (isLink) {
					println "Deleting symbolic link: " + oldData
					oldData.delete()
				}
			}
	   
			
			println "Here is the target to delete " +target			
            if ( target.exists( ) ) { 
				try {
				 println "Trying to delete target"	
				 Boolean isDeleted = target.deleteDir( )
				 if (isDeleted) {
					 println "Target deleted: " + target.toString()
				 }
				 else {
					 println "Couldn't delete target " + target.toString()
				 }
				} 
				catch (Exception e) {
					println "Target Deletion failed: "
					println e
					throw e;
				} 
			}
			Sys.execute("ditto ${casa_app} ${target}")
            Sys.execute("hdiutil detach ${mnt}")

            def ver
            assert target.isDirectory( )
            // does internal version match external version...
            "./casa-config --version".execute(null,new File("${target}/Contents/MacOS/")).text.eachLine { 
                x -> def match = x =~ ~/^(\d+\.\d+\.\d+)/
                     if ( match.size( ) > 0 ) { ver = match[0][1] }
            }
            assert ver == distro_ver_[0]
            return new UnpackedDistro(target,distro_type_,regression_os_label_,distro_os_)
        } else {
			def output_dir
			Deque log = new LinkedList( )
            // if the directory we're unpacking into already exists, we run into problems due
            // (at least) to the exting 'data' symlink... it might be better to determine
            // the directory the tar file will unpack into by doing a "tar tf ...", but this
            // requires forking another process, and the PackedDistro for OSX is a dmg (for
            // which there is no way to get a table of contents (as with "tar tf ..."). So
            // for now we guess the path by looking at the tarfile name...
            def name = path.getName( ).split("\\.(?=[^\\.]+\\.[^\\.]+\$)")
            def unpacked_distro_path = new File( "${outdir}" + File.separator + name[0] )
            if ( unpacked_distro_path.isDirectory( ) ) {
                println "removing existing distribution: ${unpacked_distro_path}"
                Sys.rmdir(unpacked_distro_path)
            }
			Sys.untar(path,outdir,true) {
		                log.push(it)
				if ( log.size( ) > 7 ) log.remove( )
				def match = it =~ ~/^(?:x\s+)?([^\/]+)\/.*/
		                if ( match.size( ) > 0 && match[0].size( ) > 1 ) { output_dir = match[0][1] }
		        }
			if ( log.size( ) > 0 ) {
			    println "                        [ ... ]"
			    log.collect { println "                        [${it}]" }
			}

            assert output_dir != null
	    assert new File("${outdir}/${output_dir}/.revstate").exists( )

            def target = new File("${outdir}/${output_dir}")
            assert target.isDirectory( )

            def ver
            // does internal version match external version...
            def config = ["bin/casa-config","casa-config"].inject(null) {
                result,i -> result != null ? result : new File("${outdir}/${output_dir}/${i}").exists( ) ? "${outdir}/${output_dir}/${i}" : null
            }
            assert config != null
            "${config} --version".execute(null,target).text.eachLine { 
                x -> def match = x =~ ~/^(\d+\.\d+\.\d+)/
                     if ( match.size( ) > 0 ) { ver = match[0][1] }
            }
            assert ver == distro_ver_[0]
            return new UnpackedDistro(target,distro_type_,regression_os_label_,distro_os_)
        }
    }
    private def unpack_( URL path, File outdir ) {
		output_directory_ = outdir
		def name = new File(path.getFile( )).getName( )
		def outfile = new File( "${outdir}/${name}" )
		def sink = new FileOutputStream( outfile )
		def buffer = new BufferedOutputStream( sink )
		buffer << path.openStream( )
		buffer.close( )
		unpack_( outfile, outdir )
    }
    private def distro_path_
	private def output_directory_ = new File('')
    private def distro_os_
    private def regression_os_label_
	private def regression_os_arch_
    private def distro_ver_
    private def distro_type_

    private def ctor_set_version_and_type_( elem, os ) {
        def distro_type = elem =~ ~/(?i)casa\-(\w+)/
        assert distro_type[0].size( ) == 2
        distro_type_ = distro_type[0][1]
        def distro_version = elem =~ pattern(distro_type_,distro_os_)
        assert distro_version[0].size( ) >= 2
        distro_ver_ = distro_version[0].size( ) > 2 ? ["${distro_version[0][1]}","${distro_version[0][2]}"] : ["${distro_version[0][1]}",""]
    }
}
