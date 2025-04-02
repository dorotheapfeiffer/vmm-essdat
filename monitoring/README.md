# README file for monitoring and pcapng data storage 

## How to monitor data
The script **_monitor.py_** is a multi-process script. The first process acquires *pcapng* files from the network interface and stores them on disk. The second process analyses the oldest pcapng file in *vmm-sdat*, and produces thus a root tree. After analysing the pcapng file it is being deleted. The third process loads the data from the oldest root file with uproot and then plots the data with plotly. Three html files are created, nmx_hits.html, nmx_clusters.html and nmx_stats.html. After plotting the data the root file is deleted.

The user has to edit the parameters for the analysis, in the same way as for their normal analysis. The configuration file for the complete NMX detector is provided in the monitoring directory, as well as a calibration file.

To test the script with the provided example *pcapng* file **monitoring_00000.pcapng_**, acquire_pcapng=0 has to be set in the script. Then start it from the command line with 

> python3 monitor.py 

and open **nmx_hits.html**, **nmx_clusters.html**  and **nmx_stats.html**  in a browser.

To run the monitoring with real data, acquire_pcapng=1 has to be set, and the correct network interface has to be defined.


## How to take physics data and store it
The python script **_storePcapngData.py_** illustrates how to take data using *dumpcap* either using a fixed duration per file or a fixed file size. The user has to edit the number of desired files, the file name and the internet interface.

