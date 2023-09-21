#!/bin/sh
# Module: ctd2met.sh
# Author: Tim Smyth
# Date: 23/10/2014
# Version: 1.1
# Updated version to operate on the JCR
#
# Description: bash script to operate the ctd2met C executable
# and then email the result to the prescribed email address(es).
#

USAGE="ctd2met.sh"

#change this for individual ships
research_ship=JamesClarkRoss

# top dir
top_dir=/users/soc/NCOF

# Where the C executable resides
bin_dir=${top_dir}/bin

# input directory - ctd data
in_dir=/data/cruise/jcr/current/ctd

# specific path to the output file
out_dir=${top_dir}/output

# specify the processed directory
proc_dir=${top_dir}/processed

# email address(es) to send the data to
email="ocean.data@metoffice.gov.uk, timjsmyth@googlemail.com, jred@bas.ac.uk"

echo "======================================"
echo "Executing ctd2met.sh"
date -u

# return only the files that were produced in the last 60 mins
files=`find ${in_dir} -cmin -60 -name \*met.cnv`

# testing purposes only
#files=`find ${in_dir} -name \*met.cnv`

for filename in $files ; do
   # prevent the overwriting of files by putting slight pause in here
   sleep 2 
   echo $filename

   # Timestamp for the output filename (which is stored as well as sent)
   timestamp=`date -u +%y%m%d_%H%M%S.tem`
   outfile=${out_dir}/${research_ship}_${timestamp}

   # C executable call with redirection
   ${bin_dir}/ctd2met $filename  | sort -k 8n -t "," > $outfile

   echo "============================"
   echo "Created file: $outfile" 
   echo "now preparing for automated email"

   # send the automated email
   cat $outfile | mail -S smtp=ams3.jcr.nerc-bas.ac.uk -v -s \"${research_ship}\ CTD\ data\ ${timestamp}\" $email

done

echo "================================="

exit 0

