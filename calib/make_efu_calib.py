import json
import argparse
import os
text_description =  "Tool creates a JSON EFU format calibration file from a slow control calibration file"
parser = argparse.ArgumentParser(description=text_description,
								 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("calib", help="slow control calibration file")
parser.add_argument("-f", "--efu_file", default="efu.json", help="new calibration file in efu format")



args = parser.parse_args()
config = vars(args)
efu = config["efu_file"]
calib = config["calib"]
cnt =0
data = {}
with open(efu, 'w') as efu_file:
	
	data["Detector"]="NMX"
	data["Version"] = 1
	data["Comment"]= "v1: TDC, no ADC and timewalk"
	data["Hybrids"]= 40
	data["Calibrations"] = []  
	with open(calib, "r") as calib_file:
		the_calib  = json.load(calib_file)
		for c in the_calib["vmm_calibration"]:
			if c["vmmID"]%2== 0:
				calibration ={}
				print(int(c["fecID"]/16))
				print(int(c["fecID"]%16))
				print(int(c["vmmID"]/2))
				print(c["hybridID"])
				print("\n")
				calibration["VMMHybridCalibration"] = {}
				calibration["VMMHybridCalibration"]["HybridIndex"] = cnt
				cnt = cnt+1
				#print(cnt)
				calibration["VMMHybridCalibration"]["HybridId"] = c["hybridID"]
				calibration["VMMHybridCalibration"]["CalibrationDate"] = "20250328-185141"
				calibration["VMMHybridCalibration"]["vmm0"] = {}
				calibration["VMMHybridCalibration"]["vmm0"]["tdc_offset"] = c["time_offsets"]
				calibration["VMMHybridCalibration"]["vmm0"]["tdc_slope"] = c["time_slopes"]
			else:
				calibration["VMMHybridCalibration"]["vmm1"] = {}
				data["Calibrations"].append(calibration)
				calibration["VMMHybridCalibration"]["vmm1"]["tdc_offset"] = c["time_offsets"]
				calibration["VMMHybridCalibration"]["vmm1"]["tdc_slope"] = c["time_slopes"]
				
			#print(c["hybridID"])

	json.dump(data,efu_file,sort_keys=False)
	print("Generated EFU calibration file {0} from time calibrations of {1} hybrids".format(efu,cnt))
