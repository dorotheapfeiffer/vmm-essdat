#!/usr/bin/python
import os
import subprocess
import re
import sys

directory = os.fsencode("./efficiency")

for file in os.listdir(directory):
	filename = os.fsdecode(file)
	if filename.startswith("20250527_230437_duration_s_600_efficiency_mg1_mg3_950V_MG3_wth240_gth240_MG1_wth215_gth215") and filename.endswith(".pcapng"): 
		print(filename)
		try:	
			args = ['../build/convertFile', '-f', "./efficiency/"+filename, '-geo', 'MGEMMA_June.json',
		'-bc', '44.44', '-tac', '60', '-th','150', '-cs','1', '-ccs', '2', '-dt', '100', '-mst', '0', '-spc', '500', '-dp', '200', '-coin', 'center-of-masss', '-crl', '0.5', '-cru', '2', '-save', '[[2],[],[1]]', '-json','0', '-algo', '0', '-info', '', '-df', '0x48']	
			subprocess.call(args)
	
	
		except OSError:
			pass
