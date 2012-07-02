#!/usr/bin/perl -Ws

# This script
#   - searches for new releases
#   If release for this platform found
#      - stops test executor
#      - removes install and working directories
#      - restarts test executor

# Distribution: END_USER_BINARY

$safe = 0;

system("date -u");

if (! $prefix) {
    $prefix = "/casa/regression/01";
}
if (! $install_dir) {
    $install_dir = "$prefix/casa_install/";
}
if (! $work_dir) {
    $work_dir    = "$prefix/work/";
}
if (! $result_dir) {
    $result_dir  = "$prefix/result";
}
if (! $data_dir) {
    $data_dir = "/casa/data/regression";
}
if (! $first_test) {
    $first_test = 0;
}
$scheduler_state = "$result_dir/tests_next.txt";

if (`uname` eq "Darwin\n") {
    $release_dir = "https://svn.cv.nrao.edu/casa/osx_distro/test/";
    $filename_regexp = "CASA-intel-[0-9]*\\.dmg"
}
elsif (`uname` eq "Linux\n") {
    #$release_dir = "http://svn.cv.nrao.edu/casa/linux_distro/test/";
    $release_dir = "/casa/www/linux_distro/test/"; # works in CV

    # different filenames for 32 and 64 bit tar balls
    $arch = `uname -i`;
    $? == 0 or die $!;
    chomp($arch);
    if ($arch eq "unknown") {
	# It happened on Ubuntu...
	$arch = `uname -m`;
	$? == 0 or die $!;
	chomp($arch);
	if ($arch eq "i686" or
	    $arch eq "i586") {
	    $arch = "i386";
	}
    }
    if ($arch eq "x86_64") {
	$filename_regexp = "casapy-(test-)?[0-9.-]*-64b\\.tar\\.gz";
    }
    elsif ($arch eq "i386") {
	$filename_regexp = "casapy-(test-)?[0-9.-]*\\.tar\\.gz";
    }
    else {
	die "Unknown architechture '$arch'\n";
    }
}
else {
    die "Unknown architechture " . `uname`;
}

if ($release_dir =~ /^http/) {
# identify latest release by parsing the HTML
    $cmd = "curl $release_dir 2>/dev/null | egrep -o \"$filename_regexp\" | sort | tail -1";
}
else {
    $cmd = "/bin/ls -1t $release_dir 2>/dev/null | egrep -o \"$filename_regexp\" | head -1";
}
$latest_release = `$cmd`;
$? == 0 or die "$cmd: $!";
chomp($latest_release);
print "Getting $latest_release from $release_dir...\n";

if ($release_dir =~ /^http/) {
    sys_exe("curl $release_dir/$latest_release > $prefix/$latest_release");
}
else {
    sys_exe("cp $release_dir/$latest_release $prefix/$latest_release");
}

if (! -d $result_dir) {
    mkdir($result_dir) or die $result_dir;
}
sys_exe("cksum $prefix/$latest_release > $prefix/cksum_new");
if (! -e "$prefix/cksum" or
    system("diff $prefix/cksum_new $prefix/cksum") != 0) {
    print "$prefix/$latest_release is a new release\n";
}
else {
    print "$prefix/$latest_release: same checksum as previous install\n";
    sys_exe("rm -f $prefix/$latest_release");
    exit 0;
}

# fixme: find and kill scheduler
if (!$safe){
    sys_exe("rm -rf $work_dir");
    sys_exe("rm -rf $result_dir");
    mkdir($result_dir) or die;
}
sys_exe("echo 999999 0 > $scheduler_state");

print "Going to install $latest_release in $install_dir ...\n";

sys_exe("rm -rf $install_dir");

if (!-d $install_dir) {
    mkdir($install_dir) or die;
}
chdir($install_dir) or die;

if (`uname` eq "Darwin\n") {
    # Attach disk image, copy contents, unattach
    sys_exe("mkdir $install_dir/tmp");
    sys_exe("hdiutil attach $prefix/$latest_release -mountroot $install_dir/tmp");
    system("cp -R $install_dir/tmp/* $install_dir/");  # might partially fail
    sys_exe("ls -1 $install_dir/tmp | grep CASA | sed 's/ /\\\\ /g' | xargs -Ixxx -n 1 hdiutil eject -force $install_dir/tmp/xxx");
}
elsif (`uname` eq "Linux\n") {
    sys_exe("tar zxf $prefix/$latest_release");
}
else {
    die "Unknown architechture " . `uname`;
}
rename("$prefix/cksum_new", "$prefix/cksum");

sys_exe("rm -f $prefix/$latest_release");

$unpacked_dir = `/bin/ls -1 | grep -E "(CASA|casapy-)"`;  # newly unpacked directory
$? == 0 or die $!;
chomp($unpacked_dir);
chdir($unpacked_dir) or die $unpacked_dir;

# Linux: ./data contains some data but not ./data/regression
# Mac  : ./data is a link to /opt/casa/data
if (`uname` eq "Linux\n") {
    sys_exe("echo PATH=$install_dir/$unpacked_dir:\\\$PATH > $prefix/test_env");
    sys_exe("ln -s $data_dir/regression ./data/regression");

} else {
    $unpacked_dir =~ s/ /\\ /g;    # escape spaces
    sys_exe("ln -s $install_dir/$unpacked_dir/CASA.app/Contents/ $install_dir/Contents");
    sys_exe("echo PATH=$install_dir/:\\\$PATH > $prefix/test_env");

    # need to run casapy 1st time in order to create /opt/casa/data
    chdir("$install_dir") or die;
    open FILE, ">casapy" or die;
    print FILE "#!/bin/bash\n";
    print FILE "cd $install_dir\n";
    print FILE "exec $install_dir/Contents/MacOS/casapy \$\{\@:1\}\n";
    close(FILE);
    sys_exe("chmod +x ./casapy");
    sys_exe("cat ./casapy");

    system("./casapy < /dev/null"); # don't stop on error

    sys_exe("ln -s $data_dir/regression ./Contents/data/regression");
}

sys_exe("echo 0 $first_test > $scheduler_state");

# Finally, update this script and other files related to this testing infrastructure
# with latest version from the release
#
# It might have strange side-effects to update this file while perl
# is executing it, so do that at the very latest thing before exit.
if (`uname` eq "Linux\n") {
    if ($arch eq "x86_64") {
	$lib = "lib64";
    }
    elsif ($arch eq "i386") {
	$lib = "lib";
    }
    else {
	die "Unknown architechture '$arch'\n";
    }
    sys_exe("cp $install_dir/$unpacked_dir/$lib/python2.5/regressions/admin/*.py \$HOME/admin/");
    sys_exe("cp $install_dir/$unpacked_dir/$lib/casapy/bin/*.sh \$HOME/admin/");
    sys_exe("cp $install_dir/$unpacked_dir/$lib/casapy/bin/*.txt \$HOME/admin/");
    sys_exe("cp $install_dir/$unpacked_dir/$lib/casapy/bin/*.pl \$HOME/admin/");
}
else {
    sys_exe("cp $install_dir/$unpacked_dir/CASA.app/Contents/Resources/python/regressions/admin/*.py \$HOME/admin/");
    sys_exe("cp $install_dir/$unpacked_dir/CASA.app/Contents/MacOS/*.sh \$HOME/admin/");
    sys_exe("cp $install_dir/$unpacked_dir/CASA.app/Contents/MacOS/*.txt \$HOME/admin/");
    sys_exe("cp $install_dir/$unpacked_dir/CASA.app/Contents/MacOS/*.pl \$HOME/admin/");
}

exit 0;

sub sys_exe
{
    my $cmd = $_[0];
    print $cmd . "\n";
    system($cmd) == 0 or die $!;
    return;
}






