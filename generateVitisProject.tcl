################################################################################
# Vitis tcl script for creating Vitis project from C sources
#
# Usage:
# - Launch Vitis
# - choose "project_vitis/workspace" as workspace
# - close "Welcome" page
# - open console with "Xilinx -> XSCT console"
# - cd [path to git repository]
# - source generateVitisProject.tcl
################################################################################

# define constants
set proc ps7_cortexa9_0
set ptf_name DragonCrackPlatform
set app_name DragonCrackMainApp
set ws_path project_vitis/workspace
set src_path project_vitis/src
set mss_path project_vitis/system.mss
set sw_repo_path vitis_sw_repo
set xsa_file project_FPGA/top.xsa

# set workspace
setws $ws_path

# check if XSA file exists
if { ![file isfile $xsa_file] } {
  puts " Could not find $xsa_file file"
  exit
}

# create platform and application
platform create -name $ptf_name -hw $xsa_file -os standalone -proc $proc
app create -name $app_name -platform $ptf_name -template "Empty Application"

# import sources
importsources -name $app_name -path [file normalize $src_path] -soft-link

# import software repository
repo -set [file normalize $sw_repo_path]
repo -scan

# configure BSP from MSS file
domain config -mss [file normalize $mss_path]