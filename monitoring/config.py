import os
import glob


#########################
acquire_pcapng=0
delete_root=1
path="../build"
file_filter="monitoring"
file_size=20000
number_of_files = 10
#interface="enp1s0f0"
interface="en0"
geo_file="nmx_vmmessdat_config.json"
calib_file="nmx_vmmessdat_calib.json"
cluster_algorithm="utpc"
cs="1"
ccs="3"
dt="200"
mst="1"
spc="500"
dp="2000000"
crl="0.1"
cru="10"
save='[[0,1,2,3],[0,1,2,3],[0,1,2,3]]'
df="0x44"
th="0"
bc="44.02625"
tac="60"
channels_x = 2560
channels_y = 2560
image_x = 1280
image_y = 1280
max_charge = 10000
charge_scale = 0.05
bins_size=20
bins_missing_strip=4
bins_span=20
bins_max_delta_hits=20
bins_delta_plane=50
color_x = 'blue'
color_x0 = '#1f77b4'
color_x1 = '#17becf'
color_x2 = '#9467bd'
color_x3 = '#2ca02c'
color_y ='red'
color_y0 ='#d62728'
color_y1 ='#ff7f0e'
color_y2 ='#6D071A'
color_y3 ='#e377c2'
color_xy = 'purple'
color_xy0 = '#000000'
color_xy1 = '#800000'
color_xy2 = '#808080'
color_xy3 = '#c0c0c0'
#color_xy0 = '#000000'
#color_xy1 = '#8f8f8f'
#color_xy2 = '#555500'
#color_xy3 = '#8c564b'
color_rate = 'red'
the_height=1200
the_width=3000
time_points=50
##########################



def count_files(extension, name_filter):
    # Find all .txt files
	file_list = glob.glob(os.path.join(".", extension))
	file_list = [f for f in file_list if name_filter in os.path.basename(f)]
	if not file_list:
		return None
	if len(file_list) < 2:
		return None
	return file_list


def get_oldest_file(file_list):
	if not file_list:
		return None
	if len(file_list) < 2:
		return None
	return min(file_list, key=os.path.getmtime)
