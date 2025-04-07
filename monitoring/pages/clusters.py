import dash
from dash import html, dcc, Input, Output, State
import pandas as pd
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import plotly.io as pio
import numpy as np
from config import *



dash.register_page(__name__, path='/clusters', name="Clusters")

layout = html.Div([
	dcc.Checklist(id='log-y-toggle',options=[{'label': 'y-axis log', 'value': 'logy'}],value=[], labelStyle={'display': 'inline-block', 'margin-right': '10px'}),
	dcc.Graph(id='cluster_graph'),
	dcc.Store(id='h_totals_clusters')
])

@dash.callback(
	Output('cluster_graph', 'figure'),
	Output('h_totals_clusters', 'data'),
	Input('cluster_data', 'data'),
	State('h_totals_clusters', 'data'),
	Input('log-y-toggle', 'value')
)
def plot_data(cluster_data, h_totals_clusters,log_axis_options):
	h_totals_clusters = h_totals_clusters or {}
	if not cluster_data:
		return dash.no_update, h_totals_clusters
	df_clusters = pd.DataFrame(cluster_data) 
	d0 = df_clusters.query("det == 0")
	d1 = df_clusters.query("det == 1")
	d2 = df_clusters.query("det == 2")
	d3 = df_clusters.query("det == 3")

	yaxis_type = 'log' if 'logy' in log_axis_options else 'linear'

	if cluster_algorithm == "utpc":
		pos0x = d0["pos0_utpc"]
		pos1x = d1["pos0_utpc"]
		pos2x = 1280+d2["pos0_utpc"]
		pos3x = 1280+d3["pos0_utpc"]
		pos0y = d0["pos1_utpc"]
		pos1y = 1280+d1["pos1_utpc"]
		pos2y = d2["pos1_utpc"]
		pos3y = 1280+d3["pos1_utpc"]
		h4 = np.histogram2d(df_clusters["pos0_utpc"], df_clusters["pos1_utpc"], bins = [image_x, image_y], range = np.array([(0, image_x), (0, image_y)]))
		fig = make_subplots(rows = 2, cols = 4, horizontal_spacing=0.06, vertical_spacing=0.1, subplot_titles = ("clusters pos0_utpc", "clusters pos1_utpc", "clusters adc0+adc1", "clusters pos1_utpc:pos0_utpc","total clusters pos0_utpc", "total clusters pos1_utpc", "total clusters adc0+adc1", "total clusters pos1_utpc:pos0_utpc"))
		
	elif cluster_algorithm == "charge2":
		pos0x = d0["pos0_charge2"]
		pos1x = d1["pos0_charge2"]
		pos2x = 1280+d2["pos0_charge2"]
		pos3x = 1280+d3["pos0_charge2"]
		pos0y = d0["pos1_charge2"]
		pos1y = 1280+d1["pos1_charge2"]
		pos2y = d2["pos1_charge2"]
		pos3y = 1280+d3["pos1_charge2"]	
		h4 = np.histogram2d(df_clusters["pos0_charge2"], df_clusters["pos1_charge2"], bins = [image_x, image_y], range = np.array([(0, image_x), (0, image_y)]))
		fig = make_subplots(rows = 2, cols = 4, horizontal_spacing=0.06, vertical_spacing=0.1, subplot_titles = ("clusters pos0_charge2", "clusters pos1_charge2", "clusters adc0+adc1", "clusters pos1_charge2:pos0_charge2","total clusters pos0_charge2", "total clusters pos1_charge2", "total clusters adc0+adc1", "total clusters pos1_charge2:pos0_charge2"))
	
	else:
		pos0x = d0["pos0"]
		pos1x = d1["pos0"]
		pos2x = 1280+d2["pos0"]
		pos3x = 1280+d3["pos0"]
		pos0y = d0["pos1"]
		pos1y = 1280+d1["pos1"]
		pos2y = d2["pos1"]
		pos3y = 1280+d3["pos1"]	
		h4 = np.histogram2d(df_clusters["pos0"], df_clusters["pos1"], bins = [image_x, image_y], range = np.array([(0, image_x), (0, image_y)]))
		fig = make_subplots(rows = 2, cols = 4, horizontal_spacing=0.06, vertical_spacing=0.1, subplot_titles = ("clusters pos0", "clusters pos1", "clusters adc0+adc1", "clusters pos1:pos0","total clusters pos0", "total clusters pos1", "total clusters adc0+adc1", "total clusters pos1:pos0"))
	
	
	posx = np.concatenate([pos0x, pos1x, pos2x, pos3x])
	posy = np.concatenate([pos0y, pos1y, pos2y, pos3y])
	
	h1 = np.histogram(posx, bins = channels_x, range = [0.0, channels_x])
	h1_total = np.array(h_totals_clusters.get("h1", [0]*channels_x))
	h1_total += h1[0]
	
	h2 = np.histogram(posy, bins = channels_y, range = [0.0, channels_y])
	h2_total = np.array(h_totals_clusters.get("h2", [0]*channels_y))
	h2_total += h2[0]
	
	h1_patched = np.where(h1[0] == 0, 0.1, h1[0])
	h2_patched = np.where(h2[0] == 0, 0.1, h2[0])
	h1_total_patched = np.where(h1_total == 0, 0.1, h1[0])
	h2_total_patched = np.where(h2_total == 0, 0.1, h2[0])

	
	h3_0 = np.histogram(d0["adc0"]+d0["adc1"], bins = int(max_charge*charge_scale), range = [0.0, max_charge])
	h3_0_total = np.array(h_totals_clusters.get("h3_0", [0]*int(max_charge*charge_scale)))
	h3_0_total += h3_0[0]
	h3_1 = np.histogram(d1["adc0"]+d1["adc1"], bins = int(max_charge*charge_scale), range = [0.0, max_charge])
	h3_1_total = np.array(h_totals_clusters.get("h3_1", [0]*int(max_charge*charge_scale)))
	h3_1_total += h3_1[0]
	h3_2 = np.histogram(d2["adc0"]+d2["adc1"], bins = int(max_charge*charge_scale), range = [0.0, max_charge])
	h3_2_total = np.array(h_totals_clusters.get("h3_2", [0]*int(max_charge*charge_scale)))
	h3_2_total += h3_2[0]
	h3_3 = np.histogram(d3["adc0"]+d3["adc1"], bins = int(max_charge*charge_scale), range = [0.0, max_charge])
	h3_3_total = np.array(h_totals_clusters.get("h3_3", [0]*int(max_charge*charge_scale)))
	h3_3_total += h3_3[0]
	
	h4_total = np.array(
	h_totals_clusters.get("h4", np.zeros((image_x, image_y), dtype=np.float64).tolist()),
	dtype=np.float64
	)
	h4_total += h4[0]
		
	fig.add_trace(go.Bar(x = h1[1][:-1], y = h1_patched,  name="x",marker_color = color_x), row = 1, col = 1)
	fig.add_trace(go.Bar(x = h2[1][:-1], y = h2_patched,  name="y",marker_color = color_y), row = 1, col = 2)
	fig.add_trace(go.Bar(x = h3_0[1][:-1], y = h3_0[0], name="q0", marker_color = color_xy0), row = 1, col = 3)
	fig.add_trace(go.Bar(x = h3_1[1][:-1], y = h3_1[0], name="q1",marker_color = color_xy1), row = 1, col = 3)
	fig.add_trace(go.Bar(x = h3_2[1][:-1], y = h3_2[0], name="q2",marker_color = color_xy2), row = 1, col = 3)
	fig.add_trace(go.Bar(x = h3_3[1][:-1], y = h3_3[0], name="q3",marker_color = color_xy3), row = 1, col = 3)
	fig.add_trace(go.Heatmap(z = np.transpose(h4[0]), colorbar=dict(title="Counts",x=1.0,y=0.79,len=0.49,thickness=20),colorscale='hot'), row = 1, col = 4)

	fig.add_trace(go.Bar(x = h1[1][:-1], y = h1_total_patched,  name="x",marker_color = color_x), row = 2, col = 1)
	fig.add_trace(go.Bar(x = h2[1][:-1], y = h2_total_patched,  name="y",marker_color = color_y), row = 2, col = 2)
	fig.add_trace(go.Bar(x = h3_0[1][:-1], y = h3_0_total, name="q0", marker_color = color_xy0), row = 2, col = 3)
	fig.add_trace(go.Bar(x = h3_1[1][:-1], y = h3_1_total, name="q1",marker_color = color_xy1), row = 2, col = 3)
	fig.add_trace(go.Bar(x = h3_2[1][:-1], y = h3_2_total, name="q2",marker_color = color_xy2), row = 2, col = 3)
	fig.add_trace(go.Bar(x = h3_3[1][:-1], y = h3_3_total, name="q3",marker_color = color_xy3), row = 2, col = 3)
	fig.add_trace(go.Heatmap(z = np.transpose(h4_total), colorbar=dict(title="Counts",x=1.0,y=0.24,len=0.49,thickness=20),colorscale='hot'), row = 2, col = 4)	
	
	fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 1, col = 1)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 1, col = 1)
	
	fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 1, col = 2)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 1, col = 2)
	
	fig.update_xaxes(title_text=("charge"), row = 1, col = 3)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 1, col = 3)
	
	fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 1, col = 4)
	fig.update_yaxes(title_text=("y [pitch 0.4 mm]"), row = 1, col = 4)
	
	fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 2, col = 1)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 2, col = 1)
	
	fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 2, col = 2)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 2, col = 2)

	fig.update_xaxes(title_text=("charge"), row = 2, col = 3)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 2, col = 3)
	
	fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 2, col = 4)
	fig.update_yaxes(title_text=("y [pitch 0.4 mm]"), row = 2, col = 4)
	
	fig.update_layout(height = the_height, width = the_width, showlegend = False, barmode='overlay',uirevision='constant',bargap=0,plot_bgcolor='white',paper_bgcolor='white')

	return fig, {
		"h1": h1_total.tolist(),
		"h2": h2_total.tolist(),
		"h3_0": h3_0_total.tolist(),
		"h3_1": h3_1_total.tolist(),
		"h3_2": h3_2_total.tolist(),
		"h3_3": h3_3_total.tolist(),
		"h4": h4_total.tolist()
	}
