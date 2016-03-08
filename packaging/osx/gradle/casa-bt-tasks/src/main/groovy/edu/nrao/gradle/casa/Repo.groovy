package edu.nrao.gradle.casa
import groovy.io.FileType
import edu.nrao.gradle.casa.PackedDistro
import edu.nrao.gradle.casa.Sys
import edu.nrao.gradle.casa.Convert

class Repo {
	Repo( String r, t = 'test', os_label = Sys.osLabel( ), os = Sys.os( ) ) {
		this( new File(r).exists( ) ? new File(r) : new URL(r), t, os_label, os )
	}

	Repo( File r, t = 'test', os_label = Sys.osLabel( ), os = Sys.os( ) ) {
		assert r.isDirectory( )
		repo_path_ = r
		repo_type_ = t
		repo_os_ = os
		regression_os_label_ = os_label
	}

	Repo( URL r, t = 'test', os_label = Sys.osLabel( ), os = Sys.os( ) ) {
		repo_path_ = r
		repo_type_ = t
		repo_os_ = os
		regression_os_label_ = os_label
	}

	def current( os = repo_os_ ) {
		def result = current_( repo_path_, os )
		if ( ! result[1] ) {  throw new Exception("No current CASA distribution found...") }
		return new PackedDistro( result[1], regression_os_label_, os )
	}

	void pruneRpms( Object backup_dir=null, os = repo_os_ ) {
		assert Sys.os( ) == 'linux' : "rpm management is only supported on linux hosts"
        File back = Convert.asDir(backup_dir,false)
        if ( backup_dir != null && back == null )
            throw new Exception("Cannot save backups to directory ${backup_dir}")
		// create the Repo obj with the desired type instead of
		// specifying the type as a parameter to this function...
		String type = repo_type_
		if ( ! repo_path_ instanceof File )
			throw new Exception("RPMs can only be pruned from a directory (not ${repo_path_})")
		println "cleaning ${type} RPMs in ${repo_path_}"

		if ( back != null && back.isDirectory( ) ) {
			println "    saving backups to ${backup_dir}"
			repo_path_.eachFileMatch(FileType.FILES,PackedDistro.pattern(type,os,Sys.arch( ),false)) {
				println "        > moving ${it} to ${backup_dir}"
				Sys.move(it,back,true)
			}
		} else {
			repo_path_.eachFileMatch(FileType.FILES,PackedDistro.pattern(type,os,Sys.arch( ),false)) {
				println "        > deleting ${it}"
				if ( it.delete( ) == false ) throw new Exception("deletion of ${it} failed")
			}
		}
	}

	void updateMeta( os = repo_os_ ) {
		if ( os == 'linux' ) assert Sys.os( ) == 'linux' : "linux meta data can only be updated from a linux host"

		assert Sys.os( ) == 'linux' : "rpm management is only supported on linux hosts"
		// create the Repo obj with the desired type instead of
		// specifying the type as a parameter to this function...
		String type = repo_type_
		if ( ! repo_path_ instanceof File )
			throw new Exception("can only update meta information in a directory (not ${repo_path_})")
		def repo_update = Sys.execute("createrepo ${repo_path_}")
		repo_update.err.eachLine { println "      repo error>\t${it}" }
	}

	private def current_( File path, os ) {
		def result = [ 0, '' ]
		def aux = 0
		println "${os}:\t${path}"
		path.eachFileMatch(FileType.FILES,PackedDistro.pattern(repo_type_,os)) {
			file ->
				def match = file.getName( ) =~ PackedDistro.pattern(repo_type_,os)
				def num = PackedDistro.numeric(match[0][1])
				if ( num > result[0] ) {
					if ( match[0].size( ) > 2 ) { aux = (match[0][2]).toInteger( ) }
					result = [ num, file ]
				} else if ( num == result[0] && match[0].size( ) > 2 && (match[0][2]).toInteger( ) > aux ) {
					aux = (match[0][2]).toInteger( )
					result = [ num, file ]
				}
		}
		return result
	}

	private def current_( URL path, os ) {
		def result = [ 0, '' ]
		def aux = 0                  // prerelease/release distros have an extra build number
		def terminated_path = path.toString( )
		if ( ! terminated_path.endsWith('/') ) {  terminated_path = terminated_path + '/' }
		path.eachLine {
			l ->
				((l =~ ~/href="(\S+?)"/).collect{ it[1] }).grep(PackedDistro.pattern(repo_type_,os)).each{
					x ->
						def match = x =~ PackedDistro.pattern(repo_type_,os)
						def num = PackedDistro.numeric(match[0][1])
						if ( num > result[0] ) {
							if ( match[0].size( ) > 2 ) { aux = (match[0][2]).toInteger( ) }
							result = [ num, terminated_path + x ]
						} else if ( num == result[0] && match[0].size( ) > 2 && (match[0][2]).toInteger( ) > aux ) {
							aux = (match[0][2]).toInteger( )
							result = [ num, terminated_path + x ]
						}
				}
		}
		return result
	}

    String toString( ) { return repo_path_.toString( ) }

	// repo_path_ is a file or a url
	private def repo_path_
	// repo_type_ is one of 'test', 'prerelease', etc.
	private def repo_type_
	// repo_os_ is 'osx' or 'linux'
	private def repo_os_
	private def regression_os_label_
}
