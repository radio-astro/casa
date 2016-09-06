package edu.nrao.gradle.casa
import java.nio.file.*;

class Rpm {

	Rpm( String s ) { this(new File(s)) }
	Rpm( Path p ) { this(p.toFile( )) }
	Rpm( File f ) {
		assert f.isFile( ) : "${f} does not exist"
		path = f.toPath( )
	}

	boolean exists( ) { return path.toFile( ).isFile( ) }
	Rpm copy( String s ) { return copy(new File(s)) }
	Rpm copy( Path p ) { return copy(p.toFile( )) }
	Rpm copy( File to ) {
		assert to.isDirectory( ) : "target directory, ${to}, does not exist"
		Path to_path = to.toPath( ).resolve(path.getFileName( ))
		Files.copy( path, to_path )
		return new Rpm(to_path)
	}

	Rpm toStable( String to=null, boolean verbose=false ) {
		return toStable(to==null ? path.getParent( ) : Paths.get(to),verbose)
	}
	Rpm toStable( Path to, boolean verbose=false ) { return toStable(to.toFile( ),verbose) }
	Rpm toStable( File to, boolean verbose=false ) { return transform(to,verbose,test2stable( )) }

	//	Rpm toMonthly( String to=null )
	//	Rpm toRelease( String to=null )
	//	Rpm toVersionedRelease( String to=null )

	String toString( ) { return path.toString( ) }

	private Path path;

	private Rpm transform( File to, boolean verbose, String code ) {
		assert to.isDirectory( ) : "target directory, ${to}, does not exist"
		def outdir = File.createTempDir( )
		def Path outpath = outdir.toPath( )
		def File script = outpath.resolve('rpmmorph').toFile( )
		script << """#!/bin/env groovy
                    |System.in.eachLine {
                    |${code}
                    |}""".stripMargin( )
		script.setExecutable(true)
		def exeout = Sys.execute("rpmrebuild --verbose --notest-install --directory=${outpath} --filter=${script} -p ${path}")
		Rpm rpmout = null
		exeout.out.eachLine {
			if ( it ==~ ~/Wrote:\s\S+/ ) {
                it.replaceAll(~/Wrote:\s+(\S+)/) {
                    full, m1 ->
                        sign(m1)
                        rpmout = new Rpm(m1)
                }
            }
		}
		assert rpmout != null && rpmout.exists( ) : "transform failed, expected location for output RPM: ${outpath}"
		def result = rpmout.copy(to)
		Sys.rmdir(outpath)
		return result
	}

	private String test2stable( ) {
		return '''
              |    def cflt = it.replaceAll(~/(Conflicts:\\s+casa)-stable/) { f, m -> f = "${m}-test" }
              |    if ( cflt != it ) println cflt
              |    else println it.replaceAll("casa-test","casa-stable")
              |'''.stripMargin( )
	}
	private String stable2monthly( ) {
		return '''
              |    def cflt = it.replaceAll(~/(Conflicts:\\s+casa)-monthly/) { f, m -> f = "${m}-stable" }
              |    if ( cflt != it ) println cflt
              |    else println it.replaceAll("casa-stable","casa-monthly")
              |'''.stripMargin( )
	}
	private String pre2release( ) {
		return '''
              |    def cflt = it.replaceAll(~/(Conflicts:\\s+casa)-release/) { f, m -> f = "${m}-prerelease" }
              |    if ( cflt != it ) println cflt
              |    else println it.replaceAll("casa-prerelease","casa-release")
              |'''.stripMargin( )
	}
	private String pre2version( String version ) {
		return '''
              |    def cflt = it.replaceAll(~/(Conflicts:\\s+casa)${version}/) { f, m -> f = "${m}-prerelease" }
              |    if ( cflt != it ) println cflt
              |    else println it.replaceAll("casa-prerelease","casa${version}")
              |'''.stripMargin( ).replaceAll('\\$\\{version\\}',version)
	}

    static boolean sign_initalized_ = false
    static String sign_script_ = null
    public void sign( String path ) {
        if ( ! sign_initalized_ ) {
            sign_initalized_ = true
            def gpgdir = [ Sys.env('GNUPGHOME'),
                           Sys.home('.gnupg') ].findResult { it != null && new File(it).isDirectory( ) ? it : null }
            if ( gpgdir == null ) {
                println "warning: rpms will NOT be signed (missing GPG setup)"
                return
            }

            def rpmmacros = new File( Sys.home('.rpmmacros') )
            if ( ! rpmmacros.isFile( ) ) {
                println "warning: rpms will NOT be signed (missing ~/.rpmmacros)"
                return
            }

            def missing = ['%_signature','%_gpg_name','%_gpg_path'] as Set
            rpmmacros.eachLine {
                if ( it ==~ /.*%_signature.*/ ) missing.remove('%_signature')
                if ( it ==~ /.*%_gpg_name.*/ ) missing.remove('%_gpg_name')
                if ( it ==~ /.*%_gpg_path.*/ ) missing.remove('%_gpg_path')
            }
            if ( missing.size( ) > 0 ) {
                println "warning: rpms will NOT be signed (~/.rpmmacros is missing " + missing.join(', ') + ")"
                return
            }

            // it is regrettable that an external (expect) script must be used to sign
            // the RPMs, but the "rpm --resign ..." command uses getpass(3) which reopens
            // /dev/tty to read the password, so typical input tricks do not work, a
            // pseudo tty must be created and used to interact with it... an expect
            // library written in groovy would be nice...
            def rpmsign = Sys.home('gradle-testing','gradle','scripts','rpmsign')
            if ( ! new File(rpmsign).canExecute( ) ) {
                println "warning: rpms will Not be signed (~/scripts/rpmsign is missing)"
                return
            }
            sign_script_ = rpmsign
            println "rpms can be signed"
        }
        if ( sign_script_ != null ) {
            println "          running: ${sign_script_} ${path}"
            def sign = Sys.execute( "${sign_script_} ${path}" )
            (sign.out + sign.err).eachLine { if ( it ==~ /^gpg:.*/ ) println "                   ${it}" }
        }
    }
}
