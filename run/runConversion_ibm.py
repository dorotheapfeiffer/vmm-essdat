#!/usr/bin/python
import os
import subprocess
import re
import sys

fileName = "./example_ibm.pcapng"

try:	
	args = ['../build/convertFile', '-f', fileName, '-df', '0x10', '-log', 'TRACE']	
	subprocess.call(args)


except OSError:
	pass




