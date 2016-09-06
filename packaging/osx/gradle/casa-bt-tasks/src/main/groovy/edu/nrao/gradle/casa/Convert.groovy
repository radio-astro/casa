package edu.nrao.gradle.casa
import edu.nrao.gradle.casa.TempCleaner

class Convert {
    static File asDir( Object dir, boolean create=true, boolean throwexcep=false, boolean cleanup=true ) {
        if ( dir == null ) {
            if ( ! create ) {
                if ( throwexcep == false ) return null
                throw new Exception("received null instead of directory")
            }
            File ret = File.createTempDir( )
            if ( cleanup ) TempCleaner.cleanup(ret)
            return ret
        } else if ( dir instanceof File ) {
            File ret = (File) dir
            if ( ret.isDirectory( ) ) return ret
            else if ( new File(ret.getParent( )).isDirectory( ) ) {
                if ( ret.exists( ) ) {
                    if ( throwexcep == false ) return null
                    throw new Exception("${ret} exists but is not a directory")
                }
                if ( ret.mkdir( ) == false ) {
                    if ( throwexcep == false ) return null
                    throw new Exception("cannot create ${ret}")
                }
                return ret
            } else {
                if ( throwexcep == false ) return null
                throw new Exception("will not create whole directory tree")
            }
        } else if ( dir instanceof String ) return asDir( new File((String)dir), create )
        else if ( dir instanceof GString ) return asDir( new File((String)dir), create )
        else throw new Exception("cannot create directory from unkown type (${dir.getClass()})")
    }

}
