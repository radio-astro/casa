# Building the pipeline
## Check target installation directory
First, check which CASA installation is first on your path by running
`casa-config`.
```
$ casa-config --prefix
```
The pipeline will be installed into the output path given by this command, so
ensure that it points to the correct CASA installation. If not, adjust your
PATH environment variable accordingly. Naturally, you will also need write
permissions for the target installation directory.

## Standard install
The pipeline can be built and installed like any standard Python module, with
```
$ casa-config --exec python setup.py install
```
If a pipeline egg is already installed, this command will upgrade the 
pipeline with the new installation. 

To build a pipeline .egg file without installing the egg, execute 
```
$ casa-config --exec python setup.py build
```
The resulting .egg can be found in the dist directory.

### Removing legacy pipeline installation from CASA
To prevent any possible conflict between legacy pipeline installation and new
pipeline code, the legacy pipeline installation should be removed from CASA. 
Execute:
```
casa-config --sh-exec rm '$PYTHONHOME/pipeline'
``` 

## Developer install
As a developer, you will quickly grow weary of creating an egg every time you
wish to exercise new code. The pipeline supports developer installations. In
this mode, a pseudo installation is made which adds your source directory to
the CASA site-packages. Hence the working version you are editing will become
the pipeline version available to CASA.
```
$ casa-config --exec python setup.py develop
```
To uninstall the developer installation, execute
```
$ casa-config --exec python setup.py develop -u
```

###Optional: CASA CLI bindings
The CASA CLI bindings are always generated and included in a standard install.
To make the CASA CLI bindings available for a developer install, the CLI 
bindings need to be written to the src directory. This can be done using the
`buildmytasks` command, using the _-i_ option to generate the bindings 
in-place, i.e., 
```
$ casa-config --exec python setup.py buildmytasks -i
```
Take care not to commit the code-generated files to SVN!
