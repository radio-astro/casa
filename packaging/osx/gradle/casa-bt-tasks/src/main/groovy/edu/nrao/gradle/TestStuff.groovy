package edu.nrao.gradle

class TestStuff {
	class InOne {
		private def proc
		@Override
		void finalize( ) { 
			throw new RuntimeException("InOne::finalize( ) was called")
		}
		def get( ) { return proc }
	}
	// static def makeit( ) {
	def makeit( int i ) {
		addShutdownHook {
			def foo = i
			println "teardown ${foo}"
		}
		return new InOne(proc: i)
	}
	@Override
	void finalize( ) { 
		throw new RuntimeException("TestStuff::finalize( ) was called")
	}
}