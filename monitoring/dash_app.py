import os
import pandas as pd
import uproot
from dash import Dash, html, dcc, page_container, Input, Output
from config import count_files, get_oldest_file, file_filter, delete_root, cluster_algorithm, cleanup_old_root_files

def create_dash_app():
	app = Dash(__name__, use_pages=True)
	app.title = "NMX monitoring"

	app.layout = html.Div([
		dcc.Location(id="url"),  # Required for use_pages
		dcc.Interval(id='refresh', interval=5000, n_intervals=0),
		dcc.Store(id='hit_data'),
		dcc.Store(id='plane_data'),
		dcc.Store(id='cluster_data'),
		page_container
	])
	return app


def register_callbacks(app):
	print("DASH register_callbacks")		
	@app.callback(
		Output('hit_data', 'data'),
		Output('plane_data', 'data'),
		Output('cluster_data', 'data'),
		Input('refresh', 'n_intervals')
	)
	def send_data(_):
		try:
			cleanup_old_root_files(".", max_files=10)
			root_files = count_files("*.root", file_filter)
			if not root_files:
				raise ValueError("No root files found")
			oldest_file = get_oldest_file(root_files)
			if not oldest_file:
				raise ValueError("No oldest file")
			
			print(f"Plotting {oldest_file}")
			tree_hits = uproot.open(oldest_file)['hits']
			tree_plane = uproot.open(oldest_file)['clusters_plane']
			tree_clusters = uproot.open(oldest_file)['clusters_detector']
			if delete_root > 0:
				os.remove(oldest_file)
			print(f"Removed {oldest_file}")
			df_hits = tree_hits.arrays(['adc', 'ch', 'vmm','plane', 'det'], library='pd')
			df_plane = tree_plane.arrays(['plane', 'det'], library='pd')
			if cluster_algorithm == "utpc":
				df_clusters = tree_clusters.arrays([
					'adc0', 'pos0_utpc', 'time0', 'size0', 'span_cluster0', 'max_delta_time0', 'max_missing_strip0',
					'adc1', 'pos1_utpc', 'time1','size1', 'span_cluster1', 'max_delta_time1', 'max_missing_strip1', 'det'
				], library='pd')
			elif cluster_algorithm == "charge2":
				df_clusters = tree_clusters.arrays([
					'adc0', 'pos0_charge2', 'time0', 'size0', 'span_cluster0', 'max_delta_time0', 'max_missing_strip0',
					'adc1', 'pos1_charge2', 'time1','size1', 'span_cluster1', 'max_delta_time1', 'max_missing_strip1', 'det'
				], library='pd')
			else:
				df_clusters = tree_clusters.arrays([
					'adc0', 'pos0', 'time0', 'size0', 'span_cluster0', 'max_delta_time0', 'max_missing_strip0',
					'adc1', 'pos1', 'time1','size1', 'span_cluster1', 'max_delta_time1', 'max_missing_strip1', 'det'
				], library='pd')			
			return (
				df_hits.to_dict('records'),
				df_plane.to_dict('records'),
				df_clusters.to_dict('records')
			)
		except Exception as e:
			print(f"Error: {e}")
			return (
				pd.DataFrame(columns=['adc', 'pos', 'plane', 'det']).to_dict('records'),
				pd.DataFrame(columns=['plane', 'det']).to_dict('records'),
				pd.DataFrame(columns=[
					'adc0', 'pos0', 'time0', 'size0', 'span_cluster0', 'max_delta_time0', 'max_missing_strip0',
					'adc1', 'pos1', 'time1', 'size1', 'span_cluster1', 'max_delta_time1', 'max_missing_strip1', 'det'
				]).to_dict('records')
			)