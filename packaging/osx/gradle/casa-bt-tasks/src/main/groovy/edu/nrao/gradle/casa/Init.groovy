package edu.nrao.gradle.casa

@Singleton
class Init {
    static { 
        Class.forName("edu.nrao.gradle.casa.MetaProjectProperties")
    }
}
