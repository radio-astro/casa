package edu.nrao.gradle.casa
import edu.nrao.gradle.casa.Sys

class TempCleaner {
    private static LinkedList cleanup_ =  new LinkedList( )
    private static boolean initialized_ = false

    protected TemporaryCleaner( ) { }

    static File cleanup( File f ) {
        if ( initialized_ == false ) {
            initialized_ = true
            addShutdownHook {
                cleanup_.each {
                    println "removing ${it}"
                    Sys.rm(it)
                }
            }
        }
        cleanup_.push(f)
        return f
    }

}
