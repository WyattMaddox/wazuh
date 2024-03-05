#!/bin/bash

# Wazuh package builder
# Copyright (C) 2015, Wazuh Inc.
#
# This program is a free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.

build_directories() {
  local build_folder=$1
  local wazuh_dir="$2"
  local future="$3"

  mkdir -p "${build_folder}"
  wazuh_version="$(cat wazuh*/src/VERSION| cut -d 'v' -f 2)"

  if [[ "$future" == "yes" ]]; then
    wazuh_version="$(future_version "$build_folder" "$wazuh_dir" $wazuh_version)"
    source_dir="${build_folder}/wazuh-${BUILD_TARGET}-${wazuh_version}"
  else
    package_name="wazuh-${BUILD_TARGET}-${wazuh_version}"
    source_dir="${build_folder}/${package_name}"
    cp -R $wazuh_dir "$source_dir"
  fi
  echo "$source_dir"
}

# Function to handle future version
future_version() {
  local build_folder="$1"
  local wazuh_dir="$2"
  local base_version="$3"

  specs_path="$(find $wazuh_dir -name SPECS|grep $PACKAGE_FORMAT)"

  local major=$(echo "$base_version" | cut -dv -f2 | cut -d. -f1)
  local minor=$(echo "$base_version" | cut -d. -f2)
  local version="${major}.30.0"
  local old_name="wazuh-${BUILD_TARGET}-${base_version}"
  local new_name=wazuh-${BUILD_TARGET}-${version}

  local new_wazuh_dir="${build_folder}/${new_name}"
  cp -R ${wazuh_dir} "$new_wazuh_dir"
  # TODO: Check using RPM/deb what change over the -o -name "*.spec"
  find "$new_wazuh_dir" "${specs_path}" \( -name "*VERSION*" -o -name "*changelog*" \
        -o -name "*.spec" \) -exec sed -i "s/${base_version}/${version}/g" {} \;
  sed -i "s/\$(VERSION)/${major}.${minor}/g" "$new_wazuh_dir/src/Makefile"
  sed -i "s/${base_version}/${version}/g" $new_wazuh_dir/src/init/wazuh-{server,client,local}.sh
  echo "$version"
}

# Function to generate checksum and move files
post_process() {
  local file_path="$1"
  local checksum_flag="$2"
  local source_flag="$3"

  if [[ "$checksum_flag" == "yes" ]]; then
    sha512sum "$file_path" > /var/local/checksum/$(basename "$file_path").sha512
  fi

  if [[ "$source_flag" == "yes" ]]; then
    mv "$file_path" /var/local/wazuh
  fi
}

# Main script body

# Script parameters
wazuh_branch="$1"
export PACKAGE_RELEASE="${2}"
export JOBS="$3"
debug="$4"
checksum="$5"
future="$6"
legacy="$7"
src="$8"

build_dir="/build_wazuh"

source helper_function.sh

set -x

# Download source code if it is not shared from the local host
if [ ! -d "/wazuh-local-src" ] ; then
    curl -sL https://github.com/wazuh/wazuh/tarball/${wazuh_branch} | tar zx
fi

# Build directories
source_dir=$(build_directories "$build_dir/${BUILD_TARGET}" "wazuh*" $future)

wazuh_version="$(cat $source_dir/src/VERSION| cut -d 'v' -f 2)"
package_name="wazuh-${BUILD_TARGET}-${wazuh_version}"
specs_path="$(find $source_dir -name SPECS|grep $PACKAGE_FORMAT)"

setup_build "$source_dir" "$specs_path" "$build_dir" "$package_name" "$debug"

set_debug $debug $sources_dir

# Installing build dependencies
cd $sources_dir
build_deps $legacy
build_package $package_name $debug

# Post-processing
get_checksum $wazuh_version
