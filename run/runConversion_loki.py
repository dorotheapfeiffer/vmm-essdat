#!/usr/bin/python
import os
import subprocess
import re
import sys
#	'-cal', 'calib_amor.json', '-cahi','1'
fileName = "/Users/dpfeiffe/data/loki_debug/12rings_rmm114_test3.pcapng"

try:	
	args = ['../build/convertFile', '-f', fileName, '-df', '0x30']	
	subprocess.call(args)


except OSError:
	pass




