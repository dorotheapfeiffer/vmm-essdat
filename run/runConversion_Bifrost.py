#!/usr/bin/python
import os
import subprocess
import re
import sys
#	'-cal', 'calib_amor.json', '-cahi','1'
old_firmware = "old_test8_1650V_fullbeam.pcapng"
#new_firmware = "new_test3_highrate.pcapng"
try:	
	args = ['../build/convertFile', '-f', old_firmware, '-df', '0x34']	
	subprocess.call(args)


except OSError:
	pass




