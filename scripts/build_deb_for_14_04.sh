#!/bin/bash

if [ "$1" = "" ] || [ "$2" = "" ] ; then
    echo "Must provide version then distribution like ./build... \"1.0\" \"14.04\""
    exit 1
fi

VERSION=$1
ORIG_TAR="razer-${VERSION}.tar.gz"

PACKAGE_VERSION=$2
TEMP_DIR=$(mktemp --suffix="_deb_build_tmp" -d)
ROOT=$(git rev-parse --show-toplevel)
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)

if [ "$PACKAGE_VERSION" = "14.04" ] || [ "$PACKAGE_VERSION" = "14.10" ] ; then
    $PACKAGE_BRANCH="ubuntu14_04_packaging"
else
    $PACKAGE_BRANCH="ubuntu15_04_packaging"
fi

# Go to root of directory
pushd ${ROOT}

# Archive directory
echo "git archive ${CURRENT_BRANCH} | gzip > ${TEMP_DIR}/${ORIG_TAR}"


exit 1
# Get debian files
git checkout ${PACKAGE_BRANCH}
git archive ${PACKAGE_BRANCH} | gzip > ${TEMP_DIR}/debian_files.tar.gz
git checkout ${CURRENT_BRANCH}

pushd ${TEMP_DIR}


#tar -zchf "razer-1.0.tar.gz" razer_drivers_orig/

#echo -ne "m\n" | bzr dh-make razer 1.0 "razer-1.0.tar.gz"

#cd razer
#rm debian/README*
#dpkg-buildpackage -us -uc
