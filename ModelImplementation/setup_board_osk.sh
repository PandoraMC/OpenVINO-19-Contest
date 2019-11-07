# openvino environment
	source ../../bin/setupvars.sh
	export IE_INSTALL="$INTEL_CVSDK_DIR/deployment_tools"
	export PATH="$PATH:$IE_INSTALL/inference_engine/samples/build/intel64/Release"
	export INTELFPGAOCLSDKROOT="/opt/altera/aocl-pro-rte/aclrte-linux64"
        export QUARTUS_ROOTDIR="$HOME/intelFPGA/17.1/qprogrammer"

# copy arch descriptions
	Terasic_arch="$INTEL_CVSDK_DIR/deployment_tools/terasic_demo/bitstreams/arch_descriptions"
	Intel_arch_dir="$INTEL_CVSDK_DIR/deployment_tools/inference_engine/lib/intel64/arch_descriptions"
	if [ -d $Terasic_arch ];then
		\cp $Terasic_arch/* $Intel_arch_dir/ -rf
	fi

#lsmod
	result=$(lsmod | grep aclpci_drv)
	if [ "$result" != "" ]; then
		rmmod aclpci_drv
	fi

#Install osk Board
# install opencl bsp
	if [ ! -d "$INTELFPGAOCLSDKROOT/board/osk/" ];then
		cp -r ./bitstreams/osk/osk $INTELFPGAOCLSDKROOT/board/osk
	fi

# set osk opencl 
	export AOCL_BOARD_PACKAGE_ROOT="$INTELFPGAOCLSDKROOT/board/osk"

# osk bitstream
	export DLA_AOCX_PATH="$INTEL_CVSDK_DIR/deployment_tools/terasic_demo/bitstreams/osk"
	export DLA_AOCX=$DLA_AOCX_PATH/dla_8x16_fp11_sb10800_i1_actk2_poolk2_normk2_owk2_image224x224x4096_prelu_osk.aocx

#INTEL FPGA OPENCL
	export CL_CONTEXT_COMPILER_MODE_INTELFPGA=3
	rm     $INTELFPGAOCLSDKROOT/installed_packages -rf
        rm     /opt/Intel -rf
	source $INTELFPGAOCLSDKROOT/init_opencl.sh

#AOCL INSTALL
	result=$(lsmod | grep aclpci_drv)
	if [ "$result" = "" ]; then
		aocl install
	fi
	

