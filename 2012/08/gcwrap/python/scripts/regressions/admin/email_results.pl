#!/usr/bin/perl -Ws
use File::Find;

# This script emails the messages notices by get_results.pl

#
# First some global definitions
#

$sendmail = "/usr/sbin/sendmail";

die "no sendmail" unless -x $sendmail;

if (! $prefix ) {
    $prefix = "/export/data/casa-regressions";
}
if (! $mail_dir ) {
    $mail_dir = "$prefix/mail";
    die "no mail directory" unless -d $mail_dir;
}


sub find_msg {
    my $path = "$File::Find::dir/$_";
    my $file = $_;

    if ( -f $file && $file =~ m|^email\d+.txt$| ) {
	print "Sending $path...\n";

	unless ( open( FILE, "< $file" ) ) {
	    die "could not open $path";
	}
	unless ( open( SENDMAIL, "| $sendmail -t" ) ) {
	    die "could not run $sendmail...";
	}

	while ( <FILE> ) { print SENDMAIL; }

	unless ( close(FILE) ) {
	    die "could not close $path";
	}
	unless ( close(SENDMAIL) ) {
	    die "execution of $sendmail with \"$path\" failed\n";
	}
	unlink( $file );
    }
}


find( {wanted => \&find_msg}, $mail_dir );
