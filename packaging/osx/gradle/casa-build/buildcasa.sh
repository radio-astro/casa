export PATH=/opt/casa/01/libexec/qt4/bin:/Users/rpmbuild/gradle-2.3/bin/:/Users/rpmbuild/bin:/opt/casa/01/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin:/opt/X11/bin:/usr/texbin:/opt/local/bin
gradle -b /Users/rpmbuild/gradle/casa-build/build.gradle buildAndPublish > /Users/rpmbuild/gradle/workdir/buildlogs/build.log 2>&1
#gradle -b /Users/rpmbuild/gradle/casa-build/build.gradle -PpackagingRevision=35541 buildAndPublish > /Users/rpmbuild/gradle/workdir/buildlogs/build.log 2>&1
#mv /Users/rpmbuild/gradle/workdir/buildlogs/build.log /Users/rpmbuild/gradle/workdir/buildlogs/casa-test.`cat /Users/rpmbuild/gradle/workdir/trunkrevision.txt`.log
mv /Users/rpmbuild/gradle/workdir/buildlogs/build.log /Users/rpmbuild/gradle/workdir/buildlogs/casa-test.`cat /Users/rpmbuild/gradle/casa-build/trunk_svn_version.txt`.log
