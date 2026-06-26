#!/usr/bin/python
import os
import subprocess
import re
import sys

#filename1="20241211_"
filename1="JX1Y10_3_frames"
directory = os.fsencode(".")

for file in os.listdir(directory):
	filename = os.fsdecode(file)
	if filename.startswith(filename1) and filename.endswith(".pcapng"): 
		print(filename)
		try:	
			args = ['../build/convertFile', '-f', filename, '-geo', 'MG_Dec2024_geometry.json',
		'-bc', '44.44', '-tac', '60', '-th','150', '-cs','1', '-ccs', '2', '-dt', '100', '-mst', '0', '-spc', '500', '-dp', '500', '-coin', 'center-of-masss', '-crl', '0.1', '-cru', '10', '-save', '[[0],[0],[0]]', '-json','0', '-algo', '0', '-df', '0x48', '-log', 'TRACE']	
			subprocess.call(args)
	
	
		except OSError:
			pass


