#!/usr/bin/python
import os
import subprocess
import re
import sys

try:	
	args = ['../build/convertFile', '-f', 'test900V-noSlit-4DORO-241209-194037_00000.txt', '-df', '0x34']	
	subprocess.call(args)


except OSError:
	pass




