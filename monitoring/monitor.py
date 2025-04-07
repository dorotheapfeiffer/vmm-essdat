import multiprocessing
import subprocess
import glob
import os
import config as cfg
from timeit import default_timer as timer
from dash_app import create_dash_app, register_callbacks

def run_dash_app():
	app = create_dash_app()
	register_callbacks(app)
	print("\n🔍 Dash registered callbacks:")
	for output in app.callback_map:
		print(" -", output)
	app.run(debug=False, port=8050)


def store_data():
	cnt = 0
	if cfg.acquire_pcapng>0:
		while True:
			fileId = cnt%cfg.number_of_files
			name = cfg.file_name + "_" + f"{fileId:05}" + ".pcapng"
			args = ["dumpcap", "-w", name, "-a", "filesize:" + str(filesize), "-i", interface]
			subprocess.call(args)
			print("Wrote (" + str(cnt) + "): " + name +  "\n")
			cnt = cnt + 1

def analyse_data():
	while True:
		pcap_files = cfg.count_files("*.pcapng",cfg.file_filter)
		if not pcap_files:
			t.sleep(2)
			continue	
		oldest_file = cfg.get_oldest_file(pcap_files)
		if not oldest_file:
			t.sleep(2)
			continue	
		start_time = timer()
		args_vmmsdat = [cfg.path+"/convertFile", '-f', oldest_file, '-geo', cfg.geo_file, '-calib', cfg.calib_file,'-bc', cfg.bc, '-tac', cfg.tac, '-th',cfg.th, '-cs',cfg.cs, '-ccs', cfg.ccs, '-dt', cfg.dt, '-mst',cfg.mst, '-spc', cfg.spc, '-dp', cfg.dp, '-coin', 'center-of-masss', '-crl', cfg.crl, '-cru', cfg.cru, '-save', cfg.save, '-algo', '4', '-info', 'monitoring', '-df',cfg.df,'-stats','0']
		subprocess.call(args_vmmsdat)
		now_time = timer()
		print("Analysis (" + oldest_file + "): " + str(now_time - start_time) + " s\n")
		try:
			if cfg.acquire_pcapng > 0:
				os.remove(oldest_file)
		except OSError:
			print("Error deleting pcapng file " + oldest_file)
		
			
if __name__ == "__main__":
	try:
		p1 = multiprocessing.Process(target=store_data)
		p2 = multiprocessing.Process(target=analyse_data)
		p3 = multiprocessing.Process(target=run_dash_app)
	
		p1.start()
		p2.start()
		p3.start()
	
		p1.join()
		p2.join()
		p3.join()
	except KeyboardInterrupt:
		print("\nCaught Ctrl+C! Cleaning up...")
	finally:
		fileList = glob.glob("./" + cfg.file_filter + "*.root", recursive=True)
		for file in fileList:
			try:
				if cfg.delete_root > 0:
					os.remove(file)
			except OSError:
				print("Error while deleting root files..")
		fileList = glob.glob("./" + cfg.file_filter + "*.pcapng", recursive=True)
		for file in fileList:
			try:
				if cfg.acquire_pcapng > 0:
					os.remove(file)
			except OSError:
				print("Error while deleting pcapng files..")			
		print("End of program!")
