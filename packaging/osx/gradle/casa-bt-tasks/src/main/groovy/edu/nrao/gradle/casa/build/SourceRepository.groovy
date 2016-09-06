package edu.nrao.gradle.casa.build

import java.util.ArrayList;

public interface SourceRepository {

	 public RepositoryType getType();
	 public void getSource(String localDir, String repositoryLocation, String branch, ArrayList env); // Get new (replace) source
	 public void updateSource(); // Refresh existing source 
	 
}
