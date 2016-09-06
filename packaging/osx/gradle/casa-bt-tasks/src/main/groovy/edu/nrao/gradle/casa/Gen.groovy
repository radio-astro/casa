package edu.nrao.gradle.casa


class Gen {
	// for example, println Gen.randomHash('A'..'Z','0'..'9','a'..'z')(60)
	static def randomHash(Range... rges) {
		def alphabet = rges.inject(new EmptyRange()) { acc, r -> acc + r }.join( )
		return { int n -> new Random().with { (1..n).collect { alphabet[ nextInt( alphabet.length() ) ] }.join() } }
	}
}
