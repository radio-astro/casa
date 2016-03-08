package edu.nrao.gradle.casa

import org.gradle.api.DefaultTask
import org.gradle.api.tasks.TaskAction
import org.htmlcleaner.*

class SampleTask extends DefaultTask { 
	String mode = 'develop'

	def version
	private def goofy = 'ggg'

	static def doit( ) {  println 'in doit...' }

	@TaskAction
	def fillInfo( ) { 
		//def url = "https://svn.cv.nrao.edu/cgi-bin/casa-version"

		/////////////////tagsoup////////////////////////////////////////////////////
		//def tagsoupParser = new org.ccil.cowan.tagsoup.Parser()
		//def slurper = new XmlSlurper(tagsoupParser)
		//def htmlParser = slurper.parse(url)
 
		/////////////////HtmlCleaner////////////////////////////////////////////////////
		//// Clean any messy HTML
		//def cleaner = new HtmlCleaner()
		//def node = cleaner.clean(url.toURL())
 
		//// Convert from HTML to XML
		//def props = cleaner.getProperties()
		//def serializer = new SimpleXmlSerializer(props)
		//def xml = serializer.getXmlAsString(node)
 
		//// Parse the XML into a document we can work with
		//def htmlParser = new XmlSlurper(false,false).parseText(xml)

		//htmlParser.each { item -> println item }

		def url = "https://svn.cv.nrao.edu/cgi-bin/casa-version".toURL( )
		url.eachLine { 
			if (it.trim()!="") {
				println "Casa version: " + it
			} 		
		}

		version = 'hello ' + goofy + ' ' + mode
	}
}