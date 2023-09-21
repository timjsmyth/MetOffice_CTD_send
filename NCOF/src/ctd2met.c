/* 
* Module: ctd2met.c 
* Author: Tim Smyth (tjsm@pml.ac.uk) 
* Version: 1.2 
* Date: 22/10/2014 Version: 1.1 - Updated version for the JCR to run on the generic version of cnv files which are created on all cruises
* Renamed function getline as getctdline to avoid stdlib.h conflicts on compile
* Date: 13/09/2015 Version: 1.2 - Updated for depths down to 6000 m (100 dbar intervals between 1000 - 6000 dbar)
*/

/* 
* Description: C-code to read in the ctd data from the various research
* cruises and put into a format suitable for ingestion into the UKMO models 
*/
   
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<math.h>

#define USAGE "ctd2met <ctdfilename>"

/* 
* 2 dbar intervals to 10 dbar 
* 5 dbar intervals to 50 dbar 
* 10 dbar intervals to 100 dbar 
* 20 dbar intervals to 200 dbar 
* 50 dbar intervals to 1000 dbar 
* 100 dbar intervals to 6000 dbar 
*/
static float press_level[91] = {2.,4.,6.,8.,10.,15.,20.,25.,30.,35.,40.,45.,50.,60.,70.,80.,90.,100.,120.,140.,160.,180.,200.,250.,300.,350.,400.,450.,500.,550.,600.,650.,700.,750.,700.,750.,800.,850.,900.,950.,1000.,1100.,1200.,1300.,1400.,1500.,1600.,1700.,1800.,1900.,2000.,2100.,2200.,2300.,2400.,2500.,2600.,2700.,2800.,2900.,3000.,3100.,3200.,3300.,3400.,3500.,3600.,3700.,3800.,3900.,4000.,4100.,4200.,4300.,4400.,4500.,4600.,4700.,4800.,4900.,5000.,5100.,5200.,5300.,5400.,5500.,5600.,5700.,5800.,5900.,6000.};

static int *press_flag;

void *safe_malloc(size_t bytes)
/* 
* Safe version of memory allocation
*/
{
   void *ptr = malloc(bytes);
   if (ptr == NULL) {
      fprintf(stderr, "malloc failed (asked for %d bytes)\n", (unsigned int) bytes);
      exit(1);
   }
   return ptr;
}

char *getctdline(FILE *f)
/*
 * Read a line from the CTD file.
 *
 *  - strip trailing whitespace
 *  - throw blank lines away
 *
 * Returns: NULL on end-of-file or error, otherwise returns pointer to text.
 *
 */
{
	char *p;
	static char buf[256];

	for (;;) {
		p = fgets(buf,sizeof(buf),f);
		if ( p == NULL ) return p;
		p = buf+strlen(buf)-1;
		while ( p >= buf ) {
			if ( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' )
				*p = '\0';
			else
				break;
			p--;
			}
		if ( strlen(buf) > 0 ) return buf;
		}
}

int smonth2int(char *smonth)
/* 
* Convert the 3 character string version of month
* and return the numerical equivalent.  
* e.g. 'Apr' returns 4
*/
{
   int intmonth;

   if (strncmp(smonth,"Jan",3) == 0)
      intmonth = 1;
   if (strncmp(smonth,"Feb",3) == 0)
      intmonth = 2;
   if (strncmp(smonth,"Mar",3) == 0)
      intmonth = 3;
   if (strncmp(smonth,"Apr",3) == 0)
      intmonth = 4;
   if (strncmp(smonth,"May",3) == 0)
      intmonth = 5;
   if (strncmp(smonth,"Jun",3) == 0)
      intmonth = 6;
   if (strncmp(smonth,"Jul",3) == 0)
      intmonth = 7;
   if (strncmp(smonth,"Aug",3) == 0)
      intmonth = 8;
   if (strncmp(smonth,"Sep",3) == 0)
      intmonth = 9;
   if (strncmp(smonth,"Oct",3) == 0)
      intmonth = 10;
   if (strncmp(smonth,"Nov",3) == 0)
      intmonth = 11;
   if (strncmp(smonth,"Dec",3) == 0)
      intmonth = 12;
   
   return intmonth;
}

int main(int argc, char *argv[])
/* 
* Main routine.  Needs an ascii file passing to it
*/
{
   char filename[50];
   char string[50], smonth[50];
   char N, E;
   char *line;
   int nlines, retid, nlevel, n, day, year, hour, min, sec;
   int lat_deg, lon_deg, end_of_header=0, mm, month, station_id;
   float lat_min, lon_min, lat, lon, dectime;
   /* float temperature, salinity, pressure, fluor, tol, fdisc; */
   float timeS, depSM, prDM, temperature1, temperature2, cond1, cond2, salinity1, salinity2, oxy1, oxy2;
   float trans, par, flC, turb, altM, flag;
   float tol;
   FILE *ifp;
   
   if (argc != 2){ 
      printf("USAGE: %s\n", USAGE);
      return 1;
   }
   sscanf(argv[1],"%s",filename);
   
   /* Read in the binary file */
   if ((ifp = fopen(filename, "r")) == NULL){
      printf("CTD file not found.\n");
      return -1;
   }

   nlines = 0;
   while(!feof(ifp) && end_of_header == 0){ 
      line = (char *) safe_malloc(sizeof(char) * 150);
      line = getctdline(ifp);
      
      /* extract the latitude */
      if (strncmp(line,"* NMEA Latitude",15) == 0){
         sscanf(line,"%s %s %s %s %d %f %s", &string, &string, &string, &string, &lat_deg, &lat_min, &N);
         lat = (float)lat_deg+lat_min/60.;
	 /* switch the sign of latitude if in southern hemisphere */
         switch(N){
	    case 'S':
	       lat =- lat;
	       break;
	 }
      }
      
      /* extract the longitude */
      if (strncmp(line,"* NMEA Longitude",16) == 0){
         sscanf(line,"%s %s %s %s %d %f %s", &string, &string, &string, &string, &lon_deg, &lon_min, &E);
	 lon = (float)lon_deg+lon_min/60.;
	 /* switch the sign of longitude if in western hemisphere */
	 switch(E){
	    case 'W':
	       lon =- lon;
	       break;
	 }
      }

      /* extract the date and time from NMEA string */
      if (strncmp(line,"* NMEA UTC",10) == 0){
         sscanf(line,"%s %s %s %s %s %s %d %d %02d:%02d:%02d", &string, &string, &string, &string, &string, &smonth, &day, &year, &hour, &min, &sec);

	 /* Return a numeric for the month */
	 month = smonth2int(smonth);
	 
	 /* Return a decimal time */
	 dectime = hour + (min * 60. + sec)/3600.;
      }
      
      if (strncmp(line,"** Station Number:",18) == 0){
         retid = sscanf(line,"%s %s %s %d", &string, &string, &string, &station_id);
	 if (retid == 2 || station_id > 500){
/* 	    printf("Station ID not entered, using 999 instead\n");*/
	    station_id = 999;
	 }  
      }
      
      if (strncmp(line,"*END*",5) == 0)
         end_of_header = 1;  
      
      nlines++;
   }
   
   /* Print the header */
   printf("Time(GMT), Day, Month, Year, Latitude, Longitude, Station Number, Depth(db), Temperature (degC), Salinity\n"); 
   /* Now to read in the CTD data */    
   n = 0;
   tol = 1.0; /* Depth tolerance within 1.0 m */

   /* Required format - requires the .psa file run by the SeaBird processor: this is called JR_met_office_cnv.psa
   # name 0 = timeS: Time, Elapsed [seconds]
   # name 1 = depSM: Depth [salt water, m]
   # name 2 = prDM: Pressure, Digiquartz [db]  
   # name 3 = t090C: Temperature [ITS-90, deg C] 
   # name 4 = t190C: Temperature, 2 [ITS-90, deg C]
   # name 5 = c0S/m: Conductivity [S/m]
   # name 6 = c1S/m: Conductivity, 2 [S/m]
   # name 7 = sal00: Salinity, Practical [PSU]   
   # name 8 = sal11: Salinity, Practical, 2 [PSU] 
   */
   
   press_flag = calloc(91,sizeof(int));

   while(!feof(ifp)){
      fscanf(ifp,"%f %f %f %f %f %f %f %f %f %f\n", &timeS, &depSM, &prDM, &temperature1, &temperature2, &cond1, &cond2, &salinity1, &salinity2, &flag);
      for (n=0;n<91;n++){
         if ((prDM >= (press_level[n] - tol)) && (prDM <= (press_level[n] + tol)) && (fabs(salinity1 - salinity2) <= 0.01) && (fabs(temperature1 - temperature2) <= 0.01) && press_flag[n] == 0 && timeS >= 300.0){
            printf("%8.4f, %02d, %02d, %04d, %8.4f, %8.4f, %03d, %6.1f, %6.3f, %6.3f\n", dectime, day, month, year, lat, lon, station_id, press_level[n], temperature1, salinity1);
	    press_flag[n] = 1;
         }
      } 
	       
      nlines++;
   } 

   fclose(ifp);    
   return 0;
}  
   
