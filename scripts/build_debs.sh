#!/bin/bash

RED='\033[0;31m'
GREEN='\033[1;32m'
BLUE='\033[0;37m'
NC='\033[0m'

TEMP_DIR=$(mktemp --suffix="_deb_build_tmp" -d)
ROOT="$(git rev-parse --show-toplevel)"

echo -e "${BLUE}Temp DIR: ${TEMP_DIR}${NC}"

echo -e "${BLUE}Copying source to temporary build directory${NC}"

# Go to root of directory
cd "${ROOT}"

# Extract version from changelog
VERSION=$(cat "${ROOT}/debian/changelog" | grep -m 1 -oP '(?<=openrazer \()[^\-\)]+')
ORIG_TAR="openrazer_${VERSION}.orig.tar.gz"

# Add a -0 revision to changelog for building
sed -i 's|'$VERSION')|'$VERSION'-0)|' "${ROOT}/debian/changelog"

#git archive ${CURRENT_BRANCH} | gzip > ${TEMP_DIR}/${ORIG_TAR}
tar --exclude-vcs --exclude-vcs-ignores -zcf ${TEMP_DIR}/${ORIG_TAR} -C "${ROOT}" .


cd ${TEMP_DIR}
mkdir razer
tar xf ${ORIG_TAR} -C razer
cd razer
echo -e "${BLUE}Building deb packages${NC}\n"
dpkg-buildpackage -us -uc > ${TEMP_DIR}/build.log 2>&1
result_code=$?

cd "${CWD}"

if [ $result_code = 0 ]; then
    echo -e "${GREEN}Deb packages build successfully. Copied to ./dist/${NC}"
    mkdir -p "${ROOT}/dist/"
    cp ${TEMP_DIR}/*.deb "${ROOT}/dist/"

else
    echo -e "${RED}Failed to generate deb files. Check ${TEMP_DIR}/build.log for more details${NC}"
fi

# Add a -0 revision to changelog for building
sed -i 's|'$VERSION'-0)|'$VERSION')|' "${ROOT}/debian/changelog"

