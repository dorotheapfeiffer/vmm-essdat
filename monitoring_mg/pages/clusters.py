import dash
from dash import html, dcc, Input, Output, State, callback_context
import pandas as pd
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import plotly.io as pio
import plotly.express as px
import plotly.colors as pc
import numpy as np
import time
from config import *

last_seen_timestamp = 0.0

dash.register_page(__name__, path='/clusters', name="Clusters")

layout = html.Div([
	html.Div([
		html.Div([
			# Log y-axis
			dcc.Checklist(
				id='logy_toggle',
				options=[{'label': '1D plots: y-axis log', 'value': 'logy'}],
				value=[],
				labelStyle={'display': 'inline-block', 'margin-right': '10px'}
			),
			
			# Log z-axis
			dcc.Checklist(
				id='logz_toggle',
				options=[{'label': '2D plots: z-axis log', 'value': 'logz'}],
				value=[],
				labelStyle={'display': 'inline-block', 'margin-right': '10px'}
			),
			html.Label("Color palette:", style={"margin-left": "20px", "margin-right": "2px"}),
			# Color palette dropdown
			dcc.Dropdown(
				id="color_palette",
				options=color_options,
				value="Config",
				clearable=False,
				style={"width": "120px", "margin-left": "5px", "margin-right": "20px", "verticalAlign": "middle"}
			),
			html.Div(id="color_preview", style={"display": "inline-block", "margin-left": "5px", "margin-right": "20px"}),
			html.Label("Color map 2D:"),
			dcc.Dropdown(
				id="heatmap_color",
				options=get_colorscale_options(),
				value="Hot",
				style={"width": "200px", "margin-left": "5px", "margin-right": "50px", "verticalAlign": "middle"}
			),
			html.Button("Clear Plots", id="clear_button", n_clicks=0)
			], style={
			"display": "flex",
			"alignItems": "center",  # vertically align
			"justifyContent": "center",
			"margin-bottom": "20px"
		})
	]),
	dcc.Graph(id='cluster_graph'),
	dcc.Store(id='h_totals_clusters')
])



@dash.callback(
	Output('cluster_graph', 'figure'),
	Output('h_totals_clusters', 'data'),
	Input('cluster_data', 'data'),
	Input('logy_toggle', 'value'),
	Input('logz_toggle', 'value'),
	Input("color_palette", "value"),
	Input("heatmap_color", "value"),
	Input("clear_button", "n_clicks"),
	State('h_totals_clusters', 'data')
)
def plot_data(cluster_data,logy_toggle,logz_toggle, color_palette, heatmap_color, clear_button, h_totals_clusters):
	print("clusters " + str(time.time()))
	global last_seen_timestamp
	h_totals_clusters = h_totals_clusters or {}
	if callback_context.triggered_id == "clear_button":
		empty_fig = make_subplots(rows = 2, cols = 5, horizontal_spacing=0.06, vertical_spacing=0.1, subplot_titles = ("clusters wires", "clusters grids", "clusters charge", "ToF","clusters wires:grids", "total clusters wires", "total clusters grids", "total clusters charge", "total ToF", "total clusters wires:grids"))
		empty_fig.update_layout(
			height=the_height,
			width=the_width,
			showlegend=False,
			barmode='group',
			uirevision='constant',
			plot_bgcolor='white',
			paper_bgcolor='white'
		)
		return empty_fig, {
			"h1": [0]*channels_x,
			"h2": [0]*channels_y,
			"h3_0": [0]*int(max_charge*charge_scale),
			"h3_1": [0]*int(max_charge*charge_scale),
			"h3_2": [0]*int(max_charge*charge_scale),
			"h3_3": [0]*int(max_charge*charge_scale),
			"h4": np.zeros((image_x, image_y), dtype=np.float64).tolist(),
			"h5_0": [0]*int(max_tof*tof_scale),
			"h5_1": [0]*int(max_tof*tof_scale),
		}
	updated_at = shared_data.get("cluster_updated_at", 0.0)	
	if not cluster_data or updated_at == last_seen_timestamp:
		return dash.no_update, h_totals_clusters
	#print("clusters " + str(updated_at) + "         " + str(updated_at-last_seen_timestamp))
	last_seen_timestamp = updated_at
		
	df_clusters = pd.DataFrame(cluster_data) 
	c0 = df_clusters.query("det >=0 and det <= 0")
	c1 = df_clusters.query("det >=1 and det <= 1")
	
	if color_palette == "Config":
		colors = color_config
	else:
		colors = palette_map.get(color_palette, px.colors.qualitative.Plotly)
	yaxis_type = 'log' if 'logy' in logy_toggle else 'linear'
	zaxis_type = 'log' if 'logz' in logz_toggle else 'linear'

	
	if cluster_algorithm == "charge2":
		pos_wires0 = c0["pos0_charge2"]
		pos_grids0 = c0["pos1_charge2"]
		pos_wires1 = c1["pos0_charge2"]+ 120
		pos_grids1 = c1["pos1_charge2"]+ 12	

	
	else:
		pos_wires0 = c0["pos0"]
		pos_grids0 = c0["pos1"]
		pos_wires1 = c1["pos0"]+ 120
		pos_grids1 = c1["pos1"]+ 12

	fig = make_subplots(rows = 2, cols = 5, horizontal_spacing=0.06, vertical_spacing=0.1, subplot_titles = ("clusters wires", "clusters grids", "clusters charge", "ToF","clusters wires:grids", "total clusters wires", "total clusters grids", "total clusters charge", "total ToF", "total clusters wires:grids"))
	pos_wires = np.concatenate([pos_wires0, pos_wires1])
	pos_grids = np.concatenate([pos_grids0, pos_grids1])
	
	h4 = np.histogram2d(pos_wires, pos_grids, bins = [image_x, image_y], range = np.array([(0, image_x), (0, image_y)]))
	
	
	h1 = np.histogram(pos_wires, bins = channels_x, range = [0.0, channels_x])
	h1_total = np.array(h_totals_clusters.get("h1", [0]*channels_x))
	h1_total += h1[0]
	
	h2 = np.histogram(pos_grids, bins = channels_y, range = [0.0, channels_y])
	h2_total = np.array(h_totals_clusters.get("h2", [0]*channels_y))
	h2_total += h2[0]
	
	h3_0 = np.histogram(c0["adc0"], bins = int(max_charge*charge_scale), range = [0.0, max_charge])
	h3_0_total = np.array(h_totals_clusters.get("h3_0", [0]*int(max_charge*charge_scale)))
	h3_0_total += h3_0[0]
	h3_1 = np.histogram(c0["adc1"], bins = int(max_charge*charge_scale), range = [0.0, max_charge])
	h3_1_total = np.array(h_totals_clusters.get("h3_1", [0]*int(max_charge*charge_scale)))
	h3_1_total += h3_1[0]
	h3_2 = np.histogram(c1["adc0"], bins = int(max_charge*charge_scale), range = [0.0, max_charge])
	h3_2_total = np.array(h_totals_clusters.get("h3_2", [0]*int(max_charge*charge_scale)))
	h3_2_total += h3_2[0]
	h3_3 = np.histogram(c1["adc1"], bins = int(max_charge*charge_scale), range = [0.0, max_charge])
	h3_3_total = np.array(h_totals_clusters.get("h3_3", [0]*int(max_charge*charge_scale)))
	h3_3_total += h3_3[0]
	
	h4_total = np.array(
	h_totals_clusters.get("h4", np.zeros((image_x, image_y), dtype=np.float64).tolist()),
	dtype=np.float64
	)
	h4_total += h4[0]
	
	h5_0 = np.histogram(0.001*(c0["time1"]-c0["pulse_time"]), bins = int(max_tof*tof_scale), range = [0.0, max_tof])
	h5_0_total = np.array(h_totals_clusters.get("h5_0", [0]*int(max_tof*tof_scale)))
	h5_0_total += h5_0[0]
	h5_1 = np.histogram(0.001*(c1["time1"]-c1["pulse_time"]), bins = int(max_tof*tof_scale), range = [0.0, max_tof])
	h5_1_total = np.array(h_totals_clusters.get("h5_1", [0]*int(max_tof*tof_scale)))
	h5_1_total += h5_1[0]

		
	h1_patched = np.where(h1[0] == 0, 0.1, h1[0])
	h2_patched = np.where(h2[0] == 0, 0.1, h2[0])
	h4_patched = np.where(h4[0] == 0, 0.1, h4[0])
	h1_total_patched = np.where(h1_total == 0, 0.1, h1_total)
	h2_total_patched = np.where(h2_total == 0, 0.1, h2_total)
	h4_total_patched = np.where(h4_total == 0, 0.1, h4_total)
	
	if zaxis_type == 'log':
		h4_patched = np.log10(h4_patched)
		h4_total_patched = np.log10(h4_total_patched)	
	
		min_val_41 = np.floor(h4_patched.min())
		max_val_41 = np.ceil(h4_patched.max())
		min_val_42 = np.floor(h4_total_patched.min())
		max_val_42 = np.ceil(h4_total_patched.max())

		tickvals_41 = np.arange(min_val_41, max_val_41 + 1)
		ticktext_41 = [f"10^{int(v)}" for v in tickvals_41]
		tickvals_42 = np.arange(min_val_42, max_val_42 + 1)
		ticktext_42 = [f"10^{int(v)}" for v in tickvals_42]		
		colorbar_41 = dict(title='counts',tickvals=tickvals_41,ticktext=ticktext_41,x=1.0,y=0.79,len=0.49,thickness=20)
		colorbar_42 = dict(title='counts',tickvals=tickvals_42,ticktext=ticktext_42,x=1.0,y=0.24,len=0.49,thickness=20)

		fig.add_trace(go.Heatmap(z = np.transpose(h4_patched), colorbar=colorbar_41,colorscale=heatmap_color), row = 1, col = 5)
		fig.add_trace(go.Heatmap(z = np.transpose(h4_total_patched), colorbar=colorbar_42,colorscale=heatmap_color), row = 2, col = 5)	
	else:
		fig.add_trace(go.Heatmap(z = np.transpose(h4_patched), colorbar=dict(title="Counts",x=1.0,y=0.79,len=0.49,thickness=20),colorscale=heatmap_color), row = 1, col = 5)
		fig.add_trace(go.Heatmap(z = np.transpose(h4_total_patched), colorbar=dict(title="Counts",x=1.0,y=0.24,len=0.49,thickness=20),colorscale=heatmap_color), row = 2, col = 5)	


		
	fig.add_trace(go.Bar(x = h1[1][0:120], y = h1_patched[0:120], name="det 1",marker_color = colors[0]), row = 1, col = 1)
	fig.add_trace(go.Bar(x = h1[1][120:240], y = h1_patched[120:240],name="det 2", marker_color = colors[1]), row = 1, col = 1)
	
	fig.add_trace(go.Bar(x = h2[1][0:12], y = h2_patched[0:12], name="det 1",marker_color = colors[2]), row = 1, col = 2)
	fig.add_trace(go.Bar(x = h2[1][12:24], y = h2_patched[12:24], name="det 2",marker_color = colors[3]), row = 1, col = 2)
	
	fig.add_trace(go.Bar(x = h1[1][0:120], y = h1_total_patched[0:120], name="det 1",marker_color = colors[0]), row = 2, col = 1)
	fig.add_trace(go.Bar(x = h1[1][120:240], y = h1_total_patched[120:240], name="det 2",marker_color = colors[1]), row = 2, col = 1)
	
	fig.add_trace(go.Bar(x = h2[1][0:12], y = h2_total_patched[0:12], name="det 1",marker_color = colors[2]), row = 2, col = 2)
	fig.add_trace(go.Bar(x = h2[1][12:24], y = h2_total_patched[12:24],name="det 2", marker_color = colors[3]), row = 2, col = 2)

	

	fig.add_trace(go.Scatter( x = h3_0[1][:-1], y = h3_0[0], name='wires det0', mode='lines', line=dict(color=colors[0])), row = 1, col = 3)
	fig.add_trace(go.Scatter( x = h3_1[1][:-1], y = h3_1[0], name='grids det0', mode='lines', line=dict(color=colors[2])), row = 1, col = 3)
	fig.add_trace(go.Scatter( x = h3_2[1][:-1], y = h3_2[0], name='wires det1', mode='lines', line=dict(color=colors[1])), row = 1, col = 3)
	fig.add_trace(go.Scatter( x = h3_3[1][:-1], y = h3_3[0], name='grids det1', mode='lines', line=dict(color=colors[3])), row = 1, col = 3)
	
	fig.add_trace(go.Scatter( x = h5_0[1][:-1], y = h5_0[0], name='det0', mode='lines', line=dict(color=colors[4])), row = 1, col = 4)
	fig.add_trace(go.Scatter( x = h5_1[1][:-1], y = h5_1[0], name='det1', mode='lines', line=dict(color=colors[5])), row = 1, col = 4)



	fig.add_trace(go.Scatter( x = h3_0[1][:-1], y = h3_0_total, name='wires det0', mode='lines', line=dict(color=colors[0])), row = 2, col = 3)
	fig.add_trace(go.Scatter( x = h3_1[1][:-1], y = h3_1_total, name='grids det0', mode='lines', line=dict(color=colors[2])), row = 2, col = 3)
	fig.add_trace(go.Scatter( x = h3_2[1][:-1], y = h3_2_total, name='wires det1', mode='lines', line=dict(color=colors[1])), row = 2, col = 3)
	fig.add_trace(go.Scatter( x = h3_3[1][:-1], y = h3_3_total, name='grids det1', mode='lines', line=dict(color=colors[3])), row = 2, col = 3)
		
	fig.add_trace(go.Scatter( x = h5_0[1][:-1], y = h5_0_total, name='det0', mode='lines', line=dict(color=colors[4])), row = 2, col = 4)
	fig.add_trace(go.Scatter( x = h5_1[1][:-1], y = h5_1_total, name='det1', mode='lines', line=dict(color=colors[5])), row = 2, col = 4)
	
	
	fig.update_xaxes(title_text=("wires [pitch x=22 mm, z=10 mm]"), row = 1, col = 1)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 1, col = 1)
	
	fig.update_xaxes(title_text=("grids [pitch y=25 mm]"), row = 1, col = 2)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 1, col = 2)
	
	fig.update_xaxes(title_text=("wires [pitch x=22 mm, z=10 mm]"), row = 2, col = 1)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 2, col = 1)
	
	fig.update_xaxes(title_text=("grids [pitch y=25 mm]"), row = 2, col = 2)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 2, col = 2)
	
	fig.update_xaxes(title_text=("charge"), row = 1, col = 3)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 1, col = 3)
	
	fig.update_xaxes(title_text=("charge"), row = 2, col = 3)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 2, col = 3)
	
	fig.update_xaxes(title_text=("wires [pitch x=22 mm, z=10 mm]"), row = 1, col = 5)
	fig.update_yaxes(title_text=("grids [pitch y=25 mm]"), row = 1, col = 5)
	
	fig.update_xaxes(title_text=("wires [pitch x=22 mm, z=10 mm]"), row = 2, col = 5)
	fig.update_yaxes(title_text=("grids [pitch y=25 mm]"), row = 2, col = 5)
	
	fig.update_xaxes(title_text=("ToF [ms]"), row = 1, col = 4)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 1, col = 4)
	
	fig.update_xaxes(title_text=("ToF [ms]"), row = 2, col = 4)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 2, col = 4)
	
	
	fig.update_layout(height = the_height, width = the_width, showlegend = False, barmode='overlay',uirevision='constant',bargap=0,plot_bgcolor='white',paper_bgcolor='white')

	return fig, {
		"h1": h1_total.tolist(),
		"h2": h2_total.tolist(),
		"h3_0": h3_0_total.tolist(),
		"h3_1": h3_1_total.tolist(),
		"h3_2": h3_2_total.tolist(),
		"h3_3": h3_3_total.tolist(),
		"h4": h4_total.tolist(),
		"h5_0": h5_0_total.tolist(),
		"h5_1": h5_1_total.tolist()
	}
