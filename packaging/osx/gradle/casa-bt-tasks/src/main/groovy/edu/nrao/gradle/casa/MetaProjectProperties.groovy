package edu.nrao.gradle.casa
import edu.nrao.gradle.casa.Sys
import org.gradle.api.Project

@Singleton
class MetaProjectProperties {
    private static def properties = new java.util.HashMap<Object,java.util.Properties>( )
    private static def defaultProperties( ) {
        def result = new java.util.Properties( )
        result.setProperty("user.home",Sys.home( ))
        result.setProperty("user.name",Sys.username( ))
        result.setProperty("os.arch",Sys.arch( ))
        result.setProperty("os.name",Sys.os( ))
        result.setProperty("os.label",Sys.osLabel( ))
        result.setProperty("host.name",Sys.hostname( ))

        def os = Sys.os( )
        def version = os == "osx" ? Sys.getOsVersionNumber( ) : Sys.osLabel( )
        for ( String type: ["test", "stable", "prerelease", "release", "monthly"] ) {
            result.setProperty("url.distro.${type}", "https://svn.cv.nrao.edu/casa/distro/${os}/${type}/${version}")
        }

        return result
    }
    static {
        // search, for example:
        //
        //       gradle.properties.el6.rpmbuild
        //       gradle.properties.linux.rpmbuild
        //       gradle.properties.rpmbuild
        //       gradle.properties.el6
        //       gradle.properties.linux
        //       gradle.properties
        //
        Project.metaClass.properties {
        KEY=null ->
            if ( ! properties.containsKey(delegate) ) {
                def root = delegate.file("..").getCanonicalPath( )
                def username = Sys.username( )
                def hostname = Sys.hostname( )
                def osver = Sys.osLabel( )
                def os = Sys.os( )
                def file = [ "gradle.properties.${hostname}.${username}",
                             "gradle.properties.${osver}.${username}",
                             "gradle.properties.${os}.${username}",
                             "gradle.properties.${username}",
                             "gradle.properties.${hostname}",
                             "gradle.properties.${osver}",
                             "gradle.properties.${os}",
                             "gradle.properties"
                           ].findResult { def f = new File("${root}/${it}"); f.isFile( ) ? f : null }
                def props = defaultProperties( )
                println "loading properties from ${file}"
                if ( file != null ) props.load(new FileInputStream(file))
                // make properties set to the empty string the same thing as an unset property...
                // this should perhaps include properties that are entirely whitespace...
                props = props.inject(new java.util.Properties()){acc,k,v-> if ( v.length( ) > 0 ) acc.setProperty(k,v); acc}
                // expand all "[:KEY:]" substrings with KEY's VALUE
                for ( int i: 1..10 ) {
                    def orig = props.inject(new java.util.Properties()){acc,k,v-> acc.setProperty(k,v);acc}
                    boolean changed = false
                    props.each{ key,value->
                        props.setProperty(key,orig.inject(value){
                                              acc,k,v ->
                                                  // I'm going to go out on a limb here  // and say that there is no way in
                                                  // java/groovy to have "[:${key}:]" interpreted as a simple string of
                                                  // characters and not a [...] regular expression...
                                                  return acc.replaceAll(/\Q[:${k}:]\E/,v) })
                        changed = changed || value != props[key]
                    }
                    if ( ! changed ) break
                }
                
                properties.put(delegate,props)
            }
            return KEY == null ? properties.get(delegate) : properties.get(delegate)[KEY]
        }
    }
}
