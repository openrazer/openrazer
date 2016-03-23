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
    PACKAGE_BRANCH="ubuntu_14_04_packaging"
else
    PACKAGE_BRANCH="ubuntu_15_04_packaging"
fi

# Go to root of directory
cd ${ROOT}

# Archive directory
git archive ${CURRENT_BRANCH} | gzip > ${TEMP_DIR}/${ORIG_TAR}

# Get debian files
git checkout ${PACKAGE_BRANCH} &>/dev/null
git archive ${PACKAGE_BRANCH} | gzip > ${TEMP_DIR}/debian_files.tar.gz
git checkout ${CURRENT_BRANCH} &>/dev/null

# Merge the two tar (not tar.gz) files
cd ${TEMP_DIR}
mkdir razer
cp ${ORIG_TAR} razer_${VERSION}.orig.tar.gz
tar xf ${ORIG_TAR} -C razer
tar xf debian_files.tar.gz -C razer
cd razer
dpkg-buildpackage -us -uc

cd ${CWD}

echo "Temp DIR: ${TEMP_DIR}"
