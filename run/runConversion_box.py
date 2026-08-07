#!/usr/bin/python
import os
import subprocess
import re
import sys

directory_data="."
#directory_data = "/Volumes/NMX_DATA_HD/Prevessin_20250624"
directory_convert="../build"
directory_config="."
#name = "Sample_4700V_130mV_4p5mVfC_00020_20250828161647"

#name="tim_4p5mVfC_130mV_4700V_00031_20250829130938"
name="nmx_test_1frame"


for file in os.listdir(directory_data):
	filename = os.fsdecode(file)

#if filename.startswith(name) and filename.endswith(".pcapng"):
	if filename.endswith(".pcapng") and filename.startswith(name): 
		print(filename)
		try:
			args = ['../build/convertFile', '-f', filename, '-geo', 'nmx_vmmessdat_config.json',
			'-bc', '44.44', '-tac', '60', '-th','0', '-cs','1', '-ccs', '2', '-dt', '100', '-mst', '1', '-spc', '500', '-dp', '250', '-coin', 'center-of-masss', '-crl', '0.1', '-cru', '10', '-save', '[[0,1,2,3],[0,1,2,3],[0,1,2,3]]', '-json','0', '-algo', '0', '-df', '0x44', '-log', 'TRACE', '-n', '1000']	

			subprocess.call(args)
		except OSError:
			pass
