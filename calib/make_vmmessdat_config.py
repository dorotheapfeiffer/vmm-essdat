import json
import argparse
import os
text_description =  "Create vmm-essdat configuration file from EFU configuration fila"
parser = argparse.ArgumentParser(description=text_description,
								 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("config", help="EFU config file")
parser.add_argument("-f", "--file", default="vmmessdat-config.json", help="new config file")



args = parser.parse_args()
params = vars(args)
efu_config = params["config"]
new_config = params["file"]

cnt =0
data = {}
with open(new_config, 'w') as new_file:
	data["vmm_geometry"] = []  
	with open(efu_config, "r") as old_file:
		the_config  = json.load(old_file)
		for c in the_config["Config"]:
			
			ring = c["Ring"]
			fen = c["FEN"]
			hybrid = c["Hybrid"]
			offset = c["Offset"]
			isReversed = c["ReversedChannels"]
			panel = c["Panel"]
			plane = c["Plane"]
			config_vmm0 ={}
			config_vmm0["detector"]=panel
			config_vmm0["plane"]=plane
			config_vmm0["ring"]=ring
			config_vmm0["fen"]=fen
			config_vmm0["vmm"]=hybrid*2
			config_vmm0["id"] = []
			config_vmm1 ={}
			config_vmm1["detector"]=panel
			config_vmm1["plane"]=plane
			config_vmm1["ring"]=ring
			config_vmm1["fen"]=fen
			config_vmm1["vmm"]=hybrid*2+1
			config_vmm1["id"] = []
			if isReversed:
				config_vmm1["id0"] = []
				for x in range(64):
					config_vmm1["id"].append(offset+63-x)
					config_vmm0["id"].append(offset+127-x)
				data["vmm_geometry"].append(config_vmm1)
				data["vmm_geometry"].append(config_vmm0)
			else:
				for x in range(64):
					config_vmm0["id"].append(offset+x)
					config_vmm1["id"].append(offset+64+x)
				data["vmm_geometry"].append(config_vmm0)
				data["vmm_geometry"].append(config_vmm1)			
			
			
			
	json.dump(data,new_file,sort_keys=False)