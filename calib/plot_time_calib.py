#!/usr/bin/python
import os,glob
import subprocess
import re
import sys
import math
import uproot3 as uproot
import pandas as pd
import matplotlib.pyplot as plt
from timeit import default_timer as timer
import time as t
import numpy as np
from matplotlib.colors import LogNorm
import json
import argparse
import os


text_description =  "JSON calibration file plotter. The tool plot_calib_file.py plots one type of calibration at a time (adc, time, time walk)."
parser = argparse.ArgumentParser(description=text_description,
								 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("calib", help="calibration file")


args = parser.parse_args()
config = vars(args)
calib_file = config["calib"]


h_channels = np.zeros(64)
color_1 = 'darkblue'
color_2 ='darkgreen'
color_3 = 'rebeccapurple'
color_4 = 'red'
line_width = 3

for i in range(64):
	h_channels[i] = i

hybrids = []
vmms = []
fecs = []
n=0
cols = 2
with open(calib_file, 'r') as file:
	the_calib  = json.load(file)
	for calib in the_calib["vmm_calibration"]:
		n=n+1



rows = int(n/cols)
if n%cols > 0:
	rows = rows + 1

	
fig, ax = plt.subplots(nrows=rows, ncols=cols, figsize=(30, 15))

n=0			
with open(calib_file, 'r') as file:
	the_calib  = json.load(file)
	for calib in the_calib["vmm_calibration"]:
		vmmID = calib["vmmID"]
		vmms.append(vmmID)
		hybridID = calib["hybridID"]
		hybrids.append(hybridID)
		


		slopes = calib["time_slopes"]
		offsets = calib["time_offsets"]
		for v in range(64):
			if offsets[v] > 20 or offsets[v] < -15:
				print(str(hybridID) + " - " + str(config["calib"]) + " vmm" + str(vmmID) + " ch " + str(v) + ": offset " + str(offsets[v]))

		ax[int(n%cols)].plot(h_channels,offsets, color=color_1, linewidth=line_width, linestyle='solid')
		ax[int(n%cols)].plot(h_channels,slopes, color=color_4, linewidth=line_width, linestyle='solid')
		ax[int(n%cols)].title.set_text(str(config["calib"]) + "\n" + hybridID[0:16] + " " + hybridID[16:32]+"\nvmm"+str(vmmID)+"\ntime offset/slope")
		ax[int(n%cols)].set_xlabel('channel')
		#ax[int(n%cols)].set_ylabel('slope/offset')
		ax[int(n%cols)].set_ylim(-15, 20)			
	
		n=n+1

plt.show()