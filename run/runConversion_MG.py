#!/usr/bin/python
import os
import subprocess
import re
import sys

#filename1="20241211_"
filename1="20241211_192719_duration_s_10_HV900V-rate-JX1Y10_00000"
directory = os.fsencode(".")

dtvec = [25, 50,75,100,125,150,175,200,500,750, 1000, 1500, 2000,5000]
infovec = ['dt25','dt50','dt75','dt100','dt125','dt150','dt175','dt200','dt500','dt750','dt1000','dt1500', 'dt2000','dt5000']

for x in range(0,14,1):
	for file in os.listdir(directory):
		filename = os.fsdecode(file)
		if filename.startswith(filename1) and filename.endswith(".pcapng"): 
			print(filename)
			try:	
				args = ['../build/convertFile', '-f', filename, '-geo', 'MG_Dec2024_geometry.json',
			'-bc', '44.44', '-tac', '60', '-th','0', '-cs','1', '-ccs', '2', '-dt', str(dtvec[x]), '-mst', '0', '-spc', '1000000', '-dp', '200', '-coin', 'center-of-masss', '-crl', '0.1', '-cru', '10', '-save', '[[0],[0],[0]]', '-json','0', '-algo', '0', '-df', '0x48', '-log', 'INFO', '-info', infovec[x]]	
				subprocess.call(args)
		
		
			except OSError:
				pass


