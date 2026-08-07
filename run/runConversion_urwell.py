#!/usr/bin/python
import os
import subprocess
import re
import sys

base_dir = '.'  # root directory

for root, dirs, files in os.walk(base_dir):
	print(root)
	print(dirs)
	for file in files:
		if file.startswith("STF") and file.endswith('.pcapng'):
			full_path = os.path.join(root, file)
			print(f"Processing: {full_path}")
			
			try:	
				args = [
					'../build/convertFile',
					'-f', full_path,
					'-geo', './urwell.json',
					'-bc', '44.44',
					'-tac', '60',
					'-th', '0',
					'-cs', '1',
					'-ccs', '1',
					'-dt', '100',
					'-mst', '1',
					'-spc', '500',
					'-dp', '200',
					'-coin', 'center-of-masss',
					'-crl', '0.2',
					'-cru', '10',
					'-save', '[[0],[0],[]]',
					'-algo', '5',
					'-info', 'urwell',
					'-df', '0x4C',
					'-log', 'INFO'
				]
				subprocess.call(args)
			except OSError:
				pass
