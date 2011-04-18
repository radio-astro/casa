#!/usr/bin/perl -w

use strict;

use Getopt::Long;

my $purge = 0;

GetOptions('purge'=>\$purge);

my @log = <>;
my $log = join '', @log;

while ($log =~ /REQUIREMENT:\s*(R.*?):.?E.?N.?D.?R.?E.?Q/sg) {
  $_ = $1;
  s/\n//;

  if ($purge) {
    next if (/& Deferred &/);
    next if (/& Obsolete &/);
    next if (/& Done.? &/);
    next if (/& Removed &/);
  }

  s/& Done1 & /& Completed &/;
  s/& Done & /& Completed &/;

  s/(&[^&]*){2}$//;

  print "$_ \\\\\n";
}
