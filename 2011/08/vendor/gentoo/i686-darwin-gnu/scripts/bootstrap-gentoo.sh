#!/bin/bash
# encoding: utf-8
#
# see http://www.gentoo.org/proj/en/gentoo-alt/prefix/bootstrap-macos.xml



cd "$(dirname $0)/.."

export EPREFIX=$(pwd)/root
mkdir -v "${EPREFIX}"

eval $(./scripts/path-helper)


./scripts/bootstrap-prefix.sh "${EPREFIX}/tree"
for i in make wget sed python coreutils findutils tar15 patch9 grep gawk bash
do
./scripts/bootstrap-prefix.sh "${EPREFIX}/tmp" $i
done
./scripts/bootstrap-prefix.sh "${EPREFIX}" portage


export LDFLAGS="-Wl,-search_paths_first -L${EPREFIX}/usr/lib -L${EPREFIX}/lib"
export CPPFLAGS="-I${EPREFIX}/usr/include"
hash -r

emerge --oneshot sed
emerge --oneshot --nodeps wget
emerge --oneshot bash

emerge --oneshot --nodeps baselayout-prefix
emerge --oneshot --nodeps lzma-utils
emerge --oneshot --nodeps m4
emerge --oneshot --nodeps flex
emerge --oneshot --nodeps bison
emerge --oneshot --nodeps binutils-config
emerge --oneshot --nodeps binutils-apple
emerge --oneshot --nodeps gcc-config
emerge --oneshot --nodeps gcc-apple

unset LDFLAGS CPPFLAGS CHOST CC CXX HOSTCC

emerge --oneshot coreutils
emerge --oneshot findutils
emerge --oneshot tar
emerge --oneshot grep
emerge --oneshot patch
emerge --oneshot gawk
emerge --oneshot make

rm $EPREFIX/usr/share/man/man1/{env-update,quickpkg,dispatch-conf,repoman,emerge,emaint,ebuild,etc-update}.1
env FEATURES="-collision-protect" emerge --oneshot portage

rm -Rf ${EPREFIX}/tmp/*
hash -r
emerge --sync

emerge -u system

cp ./config/make.conf ./root/etc/make.conf

emerge -e system
