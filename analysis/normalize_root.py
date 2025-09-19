#!/usr/bin/python
import os
import subprocess
import re
import sys

directory_binary="../build/"
directory_data="./data/"

name_data = "tim_4p5mVfC_130mV_q0_105mV_4700V.root"
name_norm = "tim_4p5mVfC_130mV_4700V.root"
 
print(name_data)
print(name_norm)

try:	
	args = [directory_binary+'normalizeHisto', '-fin', directory_data+name_data, '-fnorm', directory_data+name_norm, '-fout', 'norm.root', '-ns','2','-nb', '3']
	subprocess.call(args)
except OSError:
	pass
