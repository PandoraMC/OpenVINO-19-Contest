#!/usr/bin/env bash

# Copyright (C) 2018-2019 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

target="CPU"

ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

run_again="Then run the script again\n\n"
dashes="\n\n###################################################\n\n"

system_ver=`cat /etc/lsb-release | grep -i "DISTRIB_RELEASE" | cut -d "=" -f2`

setupvars_path="/opt/intel/2019_r3/openvino/bin/setupvars.sh"
#setupvars_path="/home/anubis/Documents/terasic_demo/setup_board_osk.sh"
if ! . $setupvars_path ; then
    printf "Unable to run ./setupvars.sh. Please check its presence. ${run_again}"
    exit 1
fi

# Build samples
printf "${dashes}"
printf "Build Inference Engine demos\n\n"

demos_path="/home/anubis/Documents/OpenVINO-19-Contest/ModelImplementation"

if ! command -v cmake &>/dev/null; then
    printf "\n\nCMAKE is not installed. It is required to build Inference Engine demos. Please install it. ${run_again}"
    exit 1
fi

OS_PATH=$(uname -m)
NUM_THREADS="-j2"

if [ $OS_PATH == "x86_64" ]; then
  OS_PATH="intel64"
  NUM_THREADS="-j8"
fi

build_dir="/home/anubis/Documents/COMPILADOS"
printf "${build_dir}"
if [ -e $build_dir/CMakeCache.txt ]; then
	rm -rf $build_dir/CMakeCache.txt
fi
mkdir -p $build_dir
cd $build_dir
cmake -DCMAKE_BUILD_TYPE=Release $demos_path
make $NUM_THREADS Test

# Run samples
printf "${dashes}"
printf "Run Inference Engine Test\n\n"

binaries_dir="${build_dir}/${OS_PATH}/Release"
cd $binaries_dir

#./Test /media/anubis/Data/OpenVINO/Datasets/PH2/training/images/IMD002.bmp

./Test /media/anubis/Data/OpenVINO/Datasets/PH2/training/images /home/anubis/Documents/COMPILADOS/UNet UNetFull Class_100E_160x320D
#./Test /media/anubis/Data/OpenVINO/HAM1000/HAM10000_images_part_1 /home/anubis/Documents/COMPILADOS/UNet UNetFull Class_100E_160x320D
#./Test /media/anubis/Data/OpenVINO/Datasets/ISIC/images /media/anubis/Data/OpenVINO/UnetFull UNetFull

printf "${dashes}"
printf "Demo completed successfully.\n\n"
