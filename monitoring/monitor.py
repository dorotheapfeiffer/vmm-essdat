import multiprocessing
import time
import math
import random
import os,glob
import subprocess
import re
import sys
import time as t
from timeit import default_timer as timer
import numpy as np
from matplotlib.colors import LogNorm
import gc
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import plotly.io as pio
import uproot3 as uproot
import pandas as pd


#########################
acquire_pcapng=0
delete_root=1
path="../build"
file_filter="monitoring"
file_size=20000
number_of_files = 10
#interface="enp1s0f0"
interface="en0"
channels_x = 1280
channels_y = 1280
max_charge = 10000
charge_scale = 0.02
bins_size=32
bins_missing_strip=4
bins_span=50
bins_max_delta_hits=25
bins_delta_plane=200
color_x = 'blue'
color_y ='green'
color_xy = 'purple'
color_rate = 'red'
the_height=1500
the_width=2200
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


def store_data():
	cnt = 0
	if acquire_pcapng>0:
		while True:
			fileId = cnt%number_of_files
			name = file_name + "_" + f"{fileId:05}" + ".pcapng"
			args = ["dumpcap", "-w", name, "-a", "filesize:" + str(filesize), "-i", interface]
			subprocess.call(args)
			print("Wrote (" + str(cnt) + "): " + name +  "\n")
			cnt = cnt + 1


def analyse_data():
	while True:
		pcap_files = count_files("*.pcapng",file_filter)
		if not pcap_files:
			t.sleep(2)
			continue	
		oldest_file = get_oldest_file(pcap_files)
		if not oldest_file:
			t.sleep(2)
			continue	
		start_time = timer()
		args_vmmsdat = [path+"/convertFile", '-f', "./" + oldest_file, '-geo', 'nmx_vmmessdat_config.json', '-calib', 'nmx_vmmessdat_calib.json','-bc', '44.02625', '-tac', '60', '-th','0', '-cs','1', '-ccs', '3', '-dt', '200', '-mst', '1', '-spc', '500', '-dp', '2000000', '-coin', 'center-of-masss', '-crl', '0.2', '-cru', '10', '-save', '[[0,1,2,3],[0,1,2,3],[0,1,2,3]]', '-algo', '4', '-info', '', '-df','0x44','-stats','0']
		subprocess.call(args_vmmsdat)
		now_time = timer()
		print("Analysis (" + oldest_file + "): " + str(now_time - start_time) + " s\n")
		try:
			if acquire_pcapng > 0:
				os.remove(oldest_file)
		except OSError:
			print("Error deleting pcapng file " + oldest_file)
		
		
def plot_data():
	start_run = timer()
	h1_total = [0] * channels_x
	h2_total = [0] * channels_y
	h3_total = np.zeros((channels_x, 128))
	h4_total = np.zeros((channels_y, 128))
	h5_total = [0] * channels_x
	h6_total = [0] * channels_y
	h7_total = [0] * int(max_charge*charge_scale)
	h8_total = np.zeros((channels_x, channels_y))
	h_percentage_x = [0] * 100 
	h_percentage_y = [0] * 100
	h_time = [0] * 100  
	h_size0 = [0] * bins_size
	h_size1 = [0] * bins_size 
	h_size01 = [0] * 2*bins_size
	h_deltaTime = [0] * bins_delta_plane 
	h_max_missing_strip0 = [0] * bins_missing_strip 
	h_max_missing_strip1 = [0] * bins_missing_strip 
	h_span_cluster0 = [0] * bins_span 
	h_span_cluster1 = [0] * bins_span 
	h_max_delta_time0 = [0] * bins_max_delta_hits
	h_max_delta_time1 = [0] * bins_max_delta_hits 


	while True:
		try:
			root_files = count_files("*.root",file_filter)
			if not root_files:
				t.sleep(2)
				continue	
			oldest_file = get_oldest_file(root_files)
			if not oldest_file:
				t.sleep(2)
				continue	
			start_time = timer()
			print("Plotting (" + oldest_file + ")")
			####################################################################    
			# Hits
			####################################################################
			tree_hits = uproot.open(oldest_file)['hits']
			adc = tree_hits.array('adc')
			pos = tree_hits.array('pos')
			time = tree_hits.array('time')
			det = tree_hits.array('det')
			plane = tree_hits.array('plane')
			data_hits = {'adc': adc,'pos': pos,'time': time, 'det': det,'plane': plane}
			df_hits = pd.DataFrame(data_hits)
			hits0 = df_hits.query("plane == 0 and (det >= 0 and det <=3)")
			hits1 = df_hits.query("plane == 1 and (det >= 0 and det <=3)")
			fig = make_subplots(rows = 2, cols = 4, horizontal_spacing=0.06, vertical_spacing=0.1, subplot_titles = ("hits pos0", "hits pos1", "hits adc0", "hits adc1", "total hits pos0", "total hits pos1", "total hits adc0", "total hits adc1"))
			fig.update_layout(
				plot_bgcolor='white',
				paper_bgcolor='white'
			)
			h1 = np.histogram(hits0['pos'], bins = channels_x, range = [0.0, channels_x])
			h1_total = h1_total + h1[0]
			h2 = np.histogram(hits1['pos'], bins = channels_y, range = [0.0, channels_y])
			h2_total = h2_total + h2[0]             
			h3 = np.histogram2d(hits0['pos'], hits0['adc'], bins = [channels_x, 128], range = np.array([(0, channels_x), (0,1024)]))
			h3_total = h3_total + h3[0]
			h4 = np.histogram2d(hits1['pos'], hits1['adc'], bins = [channels_y, 128], range = np.array([(0, channels_y), (0,1024)]))
			h4_total = h4_total + h4[0]
			
			
			fig.add_trace(go.Bar(x = h1[1][:-1], y = h1[0], marker_color = color_x), row = 1, col = 1)
			fig.add_trace(go.Bar(x = h2[1][:-1], y = h2[0], marker_color = color_y), row = 1, col = 2)
			fig.add_trace(go.Heatmap(z = np.transpose(h3[0]), colorbar=dict(title="Counts",x=0.736,y=0.79,len=0.49,thickness=20),colorscale='jet'), row = 1, col = 3)
			fig.add_trace(go.Heatmap(z = np.transpose(h4[0]), colorbar=dict(title="Counts",x=1.0,y=0.79,len=0.49,thickness=20),colorscale='jet'), row = 1, col = 4)
			fig.add_trace(go.Bar(x = h1[1][:-1], y = h1_total, marker_color = color_x), row = 2, col = 1)
			fig.add_trace(go.Bar(x = h2[1][:-1], y = h2_total, marker_color = color_y), row = 2, col = 2)
			fig.add_trace(go.Heatmap(z = np.transpose(h3_total),colorbar=dict(title="Counts",x=0.736,y=0.24,len=0.49,thickness=20),colorscale='jet'), row = 2, col = 3)
			fig.add_trace(go.Heatmap(z = np.transpose(h4_total), colorbar=dict(title="Counts",x=1.0,y=0.24,len=0.49,thickness=20),colorscale='jet'), row = 2, col = 4)

			fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 1, col = 1)
			fig.update_yaxes(title_text=("counts"), row = 1, col = 1)
			fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 1, col = 2)
			fig.update_yaxes(title_text=("counts"), row = 1, col = 2)
			fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 1, col = 3)
			fig.update_yaxes(title_text=("adc"), row = 1, col = 3)
			fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 1, col = 4)
			fig.update_yaxes(title_text=("adc"), row = 1, col = 4)

			fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 2, col = 1)
			fig.update_yaxes(title_text=("counts"), row = 2, col = 1)
			fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 2, col = 2)
			fig.update_yaxes(title_text=("counts"), row = 2, col = 2)
			fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 2, col = 3)
			fig.update_yaxes(title_text=("adc"), row = 2, col = 3)
			fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 2, col = 4)
			fig.update_yaxes(title_text=("adc"), row = 2, col = 4)
			fig.update_traces(opacity=1.0)
			fig.update_layout(height = the_height, width = the_width, showlegend = False)
			fig.update_layout(barmode='group')
			
			html_str = pio.to_html(fig, full_html=True, include_plotlyjs='cdn')
			refresh_tag = f'<meta http-equiv="refresh" content="60">'
			html_str = html_str.replace("<head>", f"<head>\n    {refresh_tag}")
			with open("nmx_hits.html", "w") as f:
				f.write(html_str)
			fig.data = []
			
			####################################################################    
			# Clusters
			####################################################################			
			tree_detector = uproot.open(oldest_file)['clusters_detector']
			d_adc0 = tree_detector.array('adc0')
			d_pos0 = tree_detector.array('pos0')
			d_adc1 = tree_detector.array('adc1')
			d_pos1 = tree_detector.array('pos1')
			d_time0 = tree_detector.array('time0')
			d_time1 = tree_detector.array('time1')
			d_size0 = tree_detector.array('size0')
			d_size1 = tree_detector.array('size1')
			d_det = tree_detector.array('det')

			d_span_cluster0 = tree_detector.array('span_cluster0')
			d_span_cluster1 = tree_detector.array('span_cluster1')
			d_max_delta_time0 = tree_detector.array('max_delta_time0')
			d_max_delta_time1 = tree_detector.array('max_delta_time1')
			d_max_missing_strip0 = tree_detector.array('max_missing_strip0')
			d_max_missing_strip1 = tree_detector.array('max_missing_strip1')
			
			data_clusters = {'size0': d_size0,'size1': d_size1,'time0': d_time0,'time1': d_time1,'adc0': d_adc0,'pos0': d_pos0,'adc1': d_adc1,'pos1': d_pos1,'det': d_det, 'span_cluster0': d_span_cluster0,  'span_cluster1': d_span_cluster1,  'max_delta_time0': d_max_delta_time0,  'max_delta_time1': d_max_delta_time1,  'max_missing_strip0': d_max_missing_strip0,  'max_missing_strip1': d_max_missing_strip1}
			df_clusters = pd.DataFrame(data_clusters)       
			fig = make_subplots(rows = 2, cols = 4, horizontal_spacing=0.06, vertical_spacing=0.1, subplot_titles = ("clusters pos0", "clusters pos1", "clusters adc0+adc1", "clusters pos1:pos0","total clusters pos0", "total clusters pos1", "total clusters adc0+adc1", "total clusters pos1:pos0"))
			fig.update_layout(
				plot_bgcolor='white',
				paper_bgcolor='white'
			)
			h5 = np.histogram(d_pos0, bins = channels_x, range = [0.0, channels_x])
			h5_total = h5_total + h5[0]
			h6 = np.histogram(d_pos0, bins = channels_y, range = [0.0, channels_y])
			h6_total = h6_total + h6[0]
			h7 = np.histogram(d_adc0+d_adc1, bins = int(max_charge*charge_scale), range = [0.0, max_charge])
			h7_total = h7_total + h7[0]
			h8 = np.histogram2d(d_pos0, d_pos1, bins = [channels_x, channels_y], range = np.array([(0, channels_x), (0, channels_y)]))
			h8_total = h8_total + h8[0]
			fig.add_trace(go.Bar(x = h5[1][:-1], y = h5[0], marker_color = color_x), row = 1, col = 1)
			fig.add_trace(go.Bar(x = h6[1][:-1], y = h6[0], marker_color = color_y), row = 1, col = 2)
			fig.add_trace(go.Bar(x = h7[1][:-1], y = h7[0], marker_color = color_xy), row = 1, col = 3)
			fig.add_trace(go.Heatmap(z = np.transpose(h8[0]), colorbar=dict(title="Counts",x=1.0,y=0.79,len=0.49,thickness=20),colorscale='hot'), row = 1, col = 4)
			fig.add_trace(go.Bar(x = h5[1][:-1], y = h5_total, marker_color = color_x), row = 2, col = 1)
			fig.add_trace(go.Bar(x = h6[1][:-1], y = h6_total, marker_color = color_y), row = 2, col = 2)
			fig.add_trace(go.Bar(x = h7[1][:-1], y = h7_total, marker_color = color_xy), row = 2, col = 3)
			fig.add_trace(go.Heatmap(z = np.transpose(h8_total), colorbar=dict(title="Counts",x=1.0,y=0.24,len=0.49,thickness=20),colorscale='hot'), row = 2, col = 4)	
			fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 1, col = 1)
			fig.update_yaxes(title_text=("counts"), row = 1, col = 2)
			fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 1, col = 2)
			fig.update_yaxes(title_text=("counts"), row = 1, col = 2)
			fig.update_xaxes(title_text=("charge"), row = 1, col = 3)
			fig.update_yaxes(title_text=("counts"), row = 1, col = 3)
			fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 1, col = 4)
			fig.update_yaxes(title_text=("y [pitch 0.4 mm]"), row = 1, col = 4)
			fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 2, col = 1)
			fig.update_yaxes(title_text=("counts"), row = 2, col = 2)
			fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 2, col = 2)
			fig.update_yaxes(title_text=("counts"), row = 2, col = 2)
			fig.update_xaxes(title_text=("charge"), row = 2, col = 3)
			fig.update_yaxes(title_text=("counts"), row = 2, col = 3)
			fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 2, col = 4)
			fig.update_yaxes(title_text=("y [pitch 0.4 mm]"), row = 2, col = 4)
			fig.update_traces(opacity=1.0)
			fig.update_layout(height = the_height, width = the_width, showlegend = False)
			fig.update_layout(barmode='group')
			html_str = pio.to_html(fig, full_html=True, include_plotlyjs='cdn')
			refresh_tag = f'<meta http-equiv="refresh" content="60">'
			html_str = html_str.replace("<head>", f"<head>\n    {refresh_tag}")
			with open("nmx_clusters.html", "w") as f:
				f.write(html_str)
			fig.data = []
			####################################################################    
			# Stats
			####################################################################
			num_clusters = d_det.size
			tree_plane= uproot.open(oldest_file)['clusters_plane']
			p_plane = tree_plane.array('plane')
			plane_clusters = {'plane': p_plane}
			df_plane = pd.DataFrame(plane_clusters)
			p0 = df_plane.query("plane == 0")
			p1 = df_plane.query("plane == 1")
			num_clusters_x = p0["plane"].size
			num_clusters_y = p1["plane"].size
			h_percentage_x.pop(0)
			h_percentage_y.pop(0)
			h_percentage_x.append(num_clusters*100/num_clusters_x)
			h_percentage_y.append(num_clusters*100/num_clusters_y)
			h_time.pop(0)
			now_time = timer()
			h_time.append(now_time-start_run)    
			h_temp = np.histogram(d_size0, bins = bins_size, range = [0.0, bins_size])
			h_size0 = h_size0 + h_temp[0]
			h_temp = np.histogram(d_size1, bins = bins_size, range = [0.0, bins_size])
			h_size1 = h_size1 + h_temp[0]
			
			h_temp = np.histogram(d_size0+d_size1, bins = 2*bins_size, range = [0.0, 2*bins_size])
			h_size01 = h_size01 + h_temp[0]
			
			h_temp = np.histogram(d_max_missing_strip0, bins = bins_missing_strip, range = [0.0, bins_missing_strip])
			h_max_missing_strip0 = h_max_missing_strip0 + h_temp[0]
			h_temp = np.histogram(d_max_missing_strip1, bins = bins_missing_strip, range = [0.0, bins_missing_strip])
			h_max_missing_strip1 = h_max_missing_strip1 + h_temp[0]

			h_temp = np.histogram(d_max_delta_time0, bins = bins_max_delta_hits, range = [0.0, 250])
			h_max_delta_time0 = h_max_delta_time0 + h_temp[0]
			h_temp = np.histogram(d_max_delta_time1, bins = bins_max_delta_hits, range = [0.0, 250])
			h_max_delta_time1 = h_max_delta_time1 + h_temp[0]
			
			h_temp = np.histogram(d_span_cluster0, bins = bins_span, range = [0.0, 500])
			h_span_cluster0 = h_span_cluster0 + h_temp[0]
			h_temp = np.histogram(d_span_cluster1, bins = bins_span, range = [0.0, 500])
			h_span_cluster1 = h_span_cluster1 + h_temp[0]
			
			h_temp = np.histogram(d_time1 - d_time0, bins = bins_delta_plane, range = [-100000, 100000])
			h_deltaTime = h_deltaTime + h_temp[0]
			
			fig = make_subplots(rows = 2, cols = 3, horizontal_spacing=0.06, vertical_spacing=0.1, subplot_titles = ("clusters size", "delta time planes", "max missing strip", "span cluster", "max delta time", "common clusters"))
			fig.update_layout(
				plot_bgcolor='white',
				paper_bgcolor='white'
			)
			fig.add_trace(go.Scatter(x = np.arange(0,32.5,1), y = h_size0, name='x clusters', mode='markers', marker=dict(symbol='square',size=16,color=color_x)), row = 1, col = 1)
			fig.add_trace(go.Scatter(x = np.arange(0,32.5,1), y = h_size1, name='y clusters', mode='markers', marker=dict(symbol='circle',size=16,color=color_y)), row = 1, col = 1)
			fig.add_trace(go.Scatter(x = np.arange(0,32.5,1), y = h_size01, name='common clusters', mode='markers', marker=dict(symbol='diamond',size=16,color=color_xy)), row = 1, col = 1)
			fig.add_trace(go.Scatter(x = np.arange(-1000,1000,10), y = h_deltaTime, name='common clusters', mode='markers', marker=dict(symbol='diamond',size=16,color=color_xy)), row = 1, col = 2)
			fig.add_trace(go.Scatter(x = np.arange(0,4,1), y = h_max_missing_strip0, name='x clusters', mode='markers', marker=dict(symbol='square',size=16,color=color_x)), row = 1, col = 3)
			fig.add_trace(go.Scatter(x = np.arange(0,4,1), y = h_max_missing_strip1, name='y clusters', mode='markers', marker=dict(symbol='circle',size=16,color=color_y)), row = 1, col = 3)

			fig.add_trace(go.Scatter(x = np.arange(0,500,10), y = h_span_cluster0, name='x clusters', mode='markers', marker=dict(symbol='square',size=16,color=color_x)), row = 2, col = 1)
			fig.add_trace(go.Scatter(x = np.arange(0,500,10), y = h_span_cluster1, name='y clusters', mode='markers', marker=dict(symbol='circle',size=16,color=color_y)), row = 2, col = 1)

			fig.add_trace(go.Scatter(x = np.arange(0,250,10), y = h_max_delta_time0, name='x clusters', mode='markers', marker=dict(symbol='square',size=16,color=color_x)), row = 2, col = 2)
			fig.add_trace(go.Scatter(x = np.arange(0,250,10), y = h_max_delta_time1, name='y clusters', mode='markers', marker=dict(symbol='circle',size=16,color=color_y)), row = 2, col = 2)
			
			fig.add_trace(go.Scatter( x = h_time, y = h_percentage_x, name='x clusters', mode='markers', marker=dict(symbol='square',size=16,color=color_x)), row = 2, col = 3)
			fig.add_trace(go.Scatter( x = h_time, y = h_percentage_y, name='y clusters', mode='markers', marker=dict(symbol='circle',size=16,color=color_y)), row = 2, col = 3)
			
			fig.update_xaxes(title_text="size [strips]", row = 1, col = 1)
			fig.update_yaxes(title_text="counts", row = 1, col = 1)
			fig.update_xaxes(title_text="time [ns]", row = 1, col = 2)
			fig.update_yaxes(title_text="counts", row = 1, col = 2)
			fig.update_xaxes(title_text="strips", row = 1, col = 3)
			fig.update_yaxes(title_text="counts", row = 1, col = 3)
			fig.update_xaxes(title_text="time [ns]", row = 2, col = 1)
			fig.update_yaxes(title_text="counts", row = 2, col = 1)
			fig.update_xaxes(title_text="time [ns]", row = 2, col = 2)
			fig.update_yaxes(title_text="counts", row = 2, col = 2)
			fig.update_xaxes(title_text="time since start of acq [s]", row = 2, col = 3)
			fig.update_yaxes(title_text="[%]", row = 2, col = 3)
			fig.update_traces(opacity=1.0)
			fig.update_layout(height = the_height, width = the_width, showlegend = False)
			fig.update_layout(barmode='group')
			html_str = pio.to_html(fig, full_html=True, include_plotlyjs='cdn')
			refresh_tag = f'<meta http-equiv="refresh" content="60">'
			html_str = html_str.replace("<head>", f"<head>\n    {refresh_tag}")
			with open("nmx_stats.html", "w") as f:
				f.write(html_str)
			fig.data = []
			
			gc.collect()
			now_time = timer()
			print("Plotting (" + oldest_file + "): " + str(now_time - start_time) + " s\n")
			if delete_root > 0:
				os.remove(oldest_file)
		except OSError:
			print("Error")
			continue
			
if __name__ == "__main__":
	try:
		p1 = multiprocessing.Process(target=store_data)
		p2 = multiprocessing.Process(target=analyse_data)
		p3 = multiprocessing.Process(target=plot_data)
	
		p1.start()
		p2.start()
		p3.start()
	
		p1.join()
		p2.join()
		p3.join()
	except KeyboardInterrupt:
		print("\nCaught Ctrl+C! Cleaning up...")
	finally:
		fileList = glob.glob("./" + file_filter + "*.root", recursive=True)
		for file in fileList:
			try:
				if delete_root > 0:
					os.remove(file)
			except OSError:
				print("Error while deleting root files..")
		fileList = glob.glob("./" + file_filter + "*.pcapng", recursive=True)
		for file in fileList:
			try:
				if acquire_pcapng > 0:
					os.remove(file)
			except OSError:
				print("Error while deleting pcapng files..")			
		print("End of program!")
