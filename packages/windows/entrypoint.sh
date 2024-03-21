#! /bin/bash

set -ex

BRANCH=$1
JOBS=$2
DEBUG=$3
ZIP_NAME=$4
TRUST_VERIFICATION=$5
CA_NAME=$6

# Compile the wazuh agent for Windows
FLAGS="-j ${JOBS} IMAGE_TRUST_CHECKS=${TRUST_VERIFICATION} CA_NAME=\"${CA_NAME}\" "

if [[ "${DEBUG}" = "yes" ]]; then
    FLAGS+="-d "
fi

if [[ ${BRANCH} != "" ]]; then
    URL_REPO=https://github.com/wazuh/wazuh/archive/${BRANCH}.zip
    SRC_PATH=/wazuh-*

    # Download the wazuh repository
    wget -O wazuh.zip ${URL_REPO} && unzip wazuh.zip
else
    SRC_PATH=/local-src
fi

bash -c "make -C ${SRC_PATH}/src deps TARGET=winagent ${FLAGS}"
bash -c "make -C ${SRC_PATH}/src TARGET=winagent ${FLAGS}"

rm -rf ${SRC_PATH}/src/external

zip -r ${ZIP_NAME} ${SRC_PATH}
cp ${ZIP_NAME} /shared
