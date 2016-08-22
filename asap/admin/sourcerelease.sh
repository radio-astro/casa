#!/bin/sh

rev=Release12
version=1.2
exclude="admin bugs"
cvsroot=/nfs/atapplic/cvsroot

outdir=asap${version}_src
echo $outdir

cd /tmp
cvs -d $cvsroot  export -r $rev -d $outdir asap

for i in ${exclude};
do
  rm -rf ${outdir}/$i
done
tar jcvf $outdir.tar.bz2 $outdir
\rm -rf $outdir
echo "Created source release archive /tmp/${outdir}.tar.bz2"
