package edu.nrao.gradle.casa;

import java.util.ArrayList;

class ResultXml {
	
	//public static void main (String [] args) {
	//	ResultXml rx =  new ResultXml()
	//	def result = rx.testOK("/Users/vsuorant/testlogs/prerelease-tests/osx10_10/4.4.0-33119.smoke/output.xml");
	//	result;
	//	println "Has errors:" + result
	//}

	Boolean testOK (String robotXmlFile) {
		//println robotXmlFile
		def records = new XmlSlurper().parse(robotXmlFile);
		//println records.size();
		def allNodes = records.depthFirst().collect{ it }
		Boolean result = true;
		allNodes.each() {
			if (it.name()=="total") {
				//println it.name()+" record found"
				def totalStats = it.depthFirst().collect{ it }
				//println totalStats.size();
				totalStats.each() {
					//println it.name() +" " + it.text() +" " + it.@fail.text() +" " + it.@pass.text();
					if (it.text()=="All Tests" ) {
						//println "Setting results"
						//println "Pass " + it.@pass.text();
						//println "Fail " + it.@fail.text();
						if (it.@fail.text().trim()!="0") {
							result = false;
						} 
					}
				}
			}
		}
		return result;
	}
}
