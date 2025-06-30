#!/usr/bin/python
import os
import subprocess
import re
import sys

#directory_data="."
directory_data = "/Volumes/NMX_DATA_HD/Prevessin_20250624"
directory_convert="../build"
directory_config="."
#name = "Three_efu_mask_q3_4p5mVfC_130mV_no_q2_masked_4600V_00001"

    
for file in os.listdir(directory_data):
    filename = os.fsdecode(file)

    #if filename.startswith(name) and filename.endswith(".pcapng"):
    if filename.endswith(".pcapng"): 
        print(filename)
        try:	
            args = [directory_convert+'/convertFile', '-f', directory_data+'/'+filename, '-geo', directory_config+'/nmx_vmmessdat_config.json', '-calib', directory_config+'/nmx_vmmessdat_calib.json','-bc', '44.02625', '-tac', '60', '-th','0', '-cs', '1', '-ccs', '2', '-dt', '200', '-mst','1', '-spc', '500', '-dp', '500', '-coin', 'center-of-masss', '-crl', '0.1', '-cru', '10', '-save', '[[],[],[0,1,2,3]]', '-algo', '4', '-df','0x44','-stats','1', '-buf', '1000000']
            subprocess.call(args)
        except OSError:
            pass
