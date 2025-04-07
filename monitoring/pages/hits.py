import dash
from dash import html, dcc, Input, Output, State
import pandas as pd
from plotly.subplots import make_subplots
import plotly.graph_objects as go
import plotly.io as pio
import numpy as np
from config import *

h1_total = [0] * channels_x
h2_total = [0] * channels_y
h3_total = np.zeros((channels_x, 128))
h4_total = np.zeros((channels_y, 128))

dash.register_page(__name__, path='/hits', name="Hits")

layout = html.Div([
	dcc.Checklist(id='log-y-toggle',options=[{'label': 'y-axis log', 'value': 'logy'}],value=[], labelStyle={'display': 'inline-block', 'margin-right': '10px'}),
	dcc.Graph(id='hit_graph'),
	dcc.Store(id='h_totals_hits')
])



@dash.callback(
	Output('hit_graph', 'figure'),
	Output('h_totals_hits', 'data'),
	Input('hit_data', 'data'),
	State('h_totals_hits', 'data'),
	Input('log-y-toggle', 'value')
)
def plot_data(hit_data, h_totals_hits, log_axis_options):
	h_totals_hits = h_totals_hits or {}
	if not hit_data:
		return dash.no_update, h_totals_hits
	df_hits = pd.DataFrame(hit_data)
	hits0 = df_hits.query("plane == 0 and (det >= 0 and det <=3)")
	hits1 = df_hits.query("plane == 1 and (det >= 0 and det <=3)")
	fig = make_subplots(rows = 2, cols = 4, horizontal_spacing=0.06, vertical_spacing=0.1, subplot_titles = ("hits pos0", "hits pos1", "hits adc0", "hits adc1", "total hits pos0", "total hits pos1", "total hits adc0", "total hits adc1"))

	yaxis_type = 'log' if 'logy' in log_axis_options else 'linear'
	
	h1 = np.histogram(hits0['ch']+64*hits0['vmm']+hits0['det']*640, bins = channels_x, range = [0.0, channels_x])
	h1_total = np.array(h_totals_hits.get("h1", [0]*channels_x))
	h1_total += h1[0]
	
	h2 = np.histogram(hits1['ch']+64*hits1['vmm']+hits1['det']*640, bins = channels_y, range = [0.0, channels_y])
	h2_total = np.array(h_totals_hits.get("h2", [0]*channels_y))
	h2_total += h2[0]

	h3 = np.histogram2d(hits0['ch']+64*hits0['vmm']+hits0['det']*640, hits0['adc'], bins = [channels_x, 128], range = np.array([(0, channels_x), (0,1024)]))
	h3_total = np.array(
	h_totals_hits.get("h3", np.zeros((channels_x, 128), dtype=np.float64).tolist()),
	dtype=np.float64
	)
	h3_total += h3[0]

	h4 = np.histogram2d(hits1['ch']+64*hits1['vmm']+hits1['det']*640, hits1['adc'], bins = [channels_y, 128], range = np.array([(0, channels_y), (0,1024)]))
	h4_total = np.array(
	h_totals_hits.get("h4", np.zeros((channels_y, 128), dtype=np.float64).tolist()),
	dtype=np.float64
	)
	h4_total += h4[0]
		
	h1_patched = np.where(h1[0] == 0, 0.1, h1[0])
	h2_patched = np.where(h2[0] == 0, 0.1, h2[0])
	h1_total_patched = np.where(h1_total == 0, 0.1, h1[0])
	h2_total_patched = np.where(h2_total == 0, 0.1, h2[0])

	
	fig.add_trace(go.Bar(x = h1[1][:-1], y = h1_patched, marker_color = color_x), row = 1, col = 1)
	fig.add_trace(go.Bar(x = h2[1][:-1], y = h2_patched, marker_color = color_y), row = 1, col = 2)
	fig.add_trace(go.Heatmap(z = np.transpose(h3[0]), colorbar=dict(title="Counts",x=0.736,y=0.79,len=0.49,thickness=20),colorscale='jet'), row = 1, col = 3)
	fig.add_trace(go.Heatmap(z = np.transpose(h4[0]), colorbar=dict(title="Counts",x=1.0,y=0.79,len=0.49,thickness=20),colorscale='jet'), row = 1, col = 4)
	fig.add_trace(go.Bar(x = h1[1][:-1], y = h1_total_patched, marker_color = color_x), row = 2, col = 1)
	fig.add_trace(go.Bar(x = h2[1][:-1], y = h2_total_patched, marker_color = color_y), row = 2, col = 2)
	fig.add_trace(go.Heatmap(z = np.transpose(h3_total),colorbar=dict(title="Counts",x=0.736,y=0.24,len=0.49,thickness=20),colorscale='jet'), row = 2, col = 3)
	fig.add_trace(go.Heatmap(z = np.transpose(h4_total), colorbar=dict(title="Counts",x=1.0,y=0.24,len=0.49,thickness=20),colorscale='jet'), row = 2, col = 4)


	fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 1, col = 1)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 1, col = 1)
	
	fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 1, col = 2)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 1, col = 2)
	
	fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 1, col = 3)
	fig.update_yaxes(title_text=("adc"), row = 1, col = 3)
	
	fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 1, col = 4)
	fig.update_yaxes(title_text=("adc"), row = 1, col = 4)

	fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 2, col = 1)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 2, col = 1)
	
	fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 2, col = 2)
	fig.update_yaxes(type=yaxis_type,title_text=("counts"), row = 2, col = 2)
	
	fig.update_xaxes(title_text=("x [pitch 0.4 mm]"), row = 2, col = 3)
	fig.update_yaxes(title_text=("adc"), row = 2, col = 3)
	
	fig.update_xaxes(title_text=("y [pitch 0.4 mm]"), row = 2, col = 4)
	fig.update_yaxes(title_text=("adc"), row = 2, col = 4)
	fig.update_layout(height = the_height, width = the_width, showlegend = False, barmode='group',uirevision='constant',bargap=0,plot_bgcolor='white',paper_bgcolor='white')
	
	return fig, {
		"h1": h1_total.tolist(),
		"h2": h2_total.tolist(),
		"h3": h3_total.tolist(),
		"h4": h4_total.tolist(),
	}
