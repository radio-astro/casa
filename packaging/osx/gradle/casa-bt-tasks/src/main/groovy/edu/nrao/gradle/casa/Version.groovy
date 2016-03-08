package edu.nrao.gradle.casa

import org.gradle.api.DefaultTask
import org.gradle.api.tasks.TaskAction
import org.htmlcleaner.*


class Version implements Comparable {
    // when full_version_str_ == true, toString( ) returns something like
    // "4.5.67-2" otherwise it returns "4.5.67"... `stable' testing
    // uses the simple version string while `prerelease' testing uses the
    // full version string... 
	Version( boolean full_version_str=false ) { }
	Version( ArrayList v, boolean full_version_str=false ) {
		assert v.size( ) == 2
		v.each { assert it instanceof String || it instanceof GString }
		init( "${v[0]} ${v[1]}" )
        full_ver_str_ = full_version_str
	}
	Version( String s, boolean full_version_str=false ) {
		def f = new File(s)
        // file "f" may not exist yet... but if it's path exists assume that we want
        // to initialize based upon a file...
        if ( f.getParentFile( ) != null && f.getParentFile( ).isDirectory( ) ) { init(f) }
		else { init(s) }
        full_ver_str_ = full_version_str
	}

	Version( File f, boolean full_version_str=false ) {
        init(f)
        full_ver_str_ = full_version_str
    }
	//    BigInteger toBigInteger( ) { return numbers( ).inject(0.toBigInteger( )){ acc, val -> acc * 100000 + val } }
	BigInteger value( ) { return value_ }
    BigInteger toBigInteger( ) { return value_ }
    String  toString( ) { return full_ver_str_ ? version_.replaceAll(' ','-') : version_.takeWhile{it != ' '} }
	
	int compareTo( other ) { value( ) <=> other.value( ) }
    Object asType(Class clazz) { 
        if ( clazz == BigInteger ) { toBigInteger( ) }
        else if ( clazz == String ) { toString( ) }
        else { super.asType(clazz) }
    }

    def rightShift( Version other ) {
        return other.leftShift(this)
    }
    def leftShift( Version other) {
        version_ = other.version_
        value_ = other.value_
        count_ = other.count_
        if ( store != null ) save( )
        return this
    }

	void save( File f=null ) {
        if ( f == null && store == null ) throw new Exception("no file available for version storage")
        (f != null ? f : store).withWriter{ out -> out.writeLine(version_) }
    }
    

    private boolean full_ver_str_ = false
    private def File store = null
	private def version_ = "0.0.0 -1"
	private BigInteger value_ = -1
	private def count_ = 0
	private def pattern = ~/^(\d+)(?:\.(\d+)\.(\d+)(?:\s+(\d+))?)?/
	private def init( File f ) {
		if ( f.isFile( ) ) {
			assert f.isFile( )
			def line = f.withReader{ it.readLine( ) }
			init( line )
		}
        full_ver_str_ = false
        // remember, 'f' may not exist at this point...
        store = f
	}
	private def init( String line ) { 
		def match = line =~ pattern
		assert match.size( ) == 1
		def v = match[0][1..4]
		count_ = v.inject(0){ acc, val -> val == null ? acc : acc+1 }
		def numbers = v.collect{ it == null ? 0 : it as Integer }
		version_ = count_ > 1 ? numbers[0..2].join('.') + " ${numbers[3]}" : "${numbers[0]}"
		value_ = numbers.inject(0.toBigInteger( )){ acc, val -> acc * 100000 + val }
        full_ver_str_ = false
	}
}


class DevVersion { 


    DevVersion( p, m='develop' ) {
        path_ = p
        mode_ = m
    }

    private def mode_
    private def path_
    private static def production_version_
    private static def production_revision_

    private static def fill_production_version_revision_info_( ) throws Exception { 
        def url = "https://svn.cv.nrao.edu/cgi-bin/casa-version".toURL( )
        def matches = url.readLines( ) =~ ~/(\d+\.\d+\.\d+)\s+(\d+)/
        if ( matches.size( ) < 1 ) {  throw new Exception("${url} did not contain version information") }
        production_version_ = matches[0][1];
        production_revision_ = matches[0][2];
    }

    def version( ) throws Exception { 
        return mode_
    }

    static def prodVersion( ) throws Exception { 
        if ( production_version_ == null ) { fill_production_version_revision_info_( ) }
        return production_version_;
    }

    static def prodRevision( ) throws Exception { 
        if ( production_revision_ == null ) { fill_production_version_revision_info_( ) }
        return production_revision_;
    }

}
