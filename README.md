# MetOffice_CTD_send
Automated CTD send from a research vessel

Description of the automated CTD send to the Met Office
=======================================================

One of the major concerns of the National Centre for Ocean Forecasting was the
lack of CTD data from the UK Research Vessel Fleet.  These data would prove
useful for ingest into operational models, especially in regions which are data
sparse.  This code has been written to address this need.

Code description
================
Version 1: Developed on D325 (11/2007) and D338 (05/2009) by TJS
Version 2: Developed on JR303 (10/2014) by TJS and interfaced with BAS automated systems with JR, ST and JW.
 
src/ctd2met.c
-------------
This C code takes as input a CTD data '.cnv' file.  There are a few things
in the header that are required:

* NMEA Latitude = dd mm.mm N(S)
* NMEA Longitude = ddd mm.mm W(E)
* NMEA UTC (Time) = MMM dd yyyy  hh:MM:ss
* Station:
*END*

The required fields for the correct operation of the code are (in order)
#name 0 = timeS: Time, Elapsed [seconds]
#name 1 = depSM: Depth [salt water, m]
#name 2 = prDM: Pressure, Digiquartz [db] 
#name 3 = t090C: Temperature [ITS-90, deg C]
#name 4 = t190C: Temperature, 2 [ITS-90, deg C]
#name 5 = c0S/m: Conductivity [S/m]
#name 6 = c1S/m: Conductivity, 2 [S/m]
#name 7 = sal00: Salinity, Practical [PSU] 
#name 8 = sal11: Salinity, Practical, 2 [PSU] 

together with a flag (field 10).

This format is produced using the JR_met_office_cnv.psa template file in conjuction with the DOS command file SEASAVEV7_SVP.CMD run on the CTD logging machine.  As part of the automatic processing, the DOS script dumps the CTD cnv file (with suffix _met) onto the unix system.

This is automatically produced every time the CTD operator issues the DOS command SEASAVEV7_SVP.CMD.

The crontab, running as user soc on jrlc, has the shell script ctd2met.sh called within it.  The crontab commands are found in crontab/crontab.ncof.  This is run every hour and the script looks for files that are newer than 60 minutes in the /data/cruise/jcr/current/ctd directory. If the script finds one it executes bin/ctd2met which formats the data and performs the automated email send.

The code extracts the salinity and temperature data at the following levels:
 
* 2 dbar intervals to 10 dbar 
* 5 dbar intervals to 50 dbar 
* 10 dbar intervals to 100 dbar 
* 20 dbar intervals to 200 dbar 
* 50 dbar intervals to 1000 dbar 
* 100 dbar intervals to max depth (1500 dbar) 

and prints them out together with a time stamp (decimal hours), day, month,
year, station, latitude, longitude, station number (this is returned as 001 if
no station ID is found).

The full header is:

Time(GMT), Day, Month, Year, Latitude, Longitude, Station Number, Depth(db), Temperature (degC), Salinity

To compile the code type (in the NCOF directory):
gcc -o bin/ctd2met src/ctd2met.c -lm

and put the executable into the bin directory.

For more details about the formatting and other issues see the source code. 

ctd2met.sh
----------
This is a UNIX bash shell wrapper which picks up the necessary ascii files and
puts the data into a file ready for send off.  The data are emailed to:

ocean.data@metoffice.gov.uk

The script looks in the Data directory for data files for files newer than 60 minutes old and if there are any (accepts multiple) then processes them.  The output is redirected to the output directory.

crontab directory
-----------------
The crontab.ncof file can be easily edited using vi or other and then installed in the crontab using the command:

crontab crontab.ncof

The crontab can be disabled using the command 

crontab -r

example_input/JR303_061met.cnv
------------------------------
This is an example file showing the format of data required for input.

--------
Tim Smyth 27/10/2014 (tjsm@pml.ac.uk) - JR303 (AMT24)



