# python3 skipdata.py  --cubedata {data_file}.txt
# convert the 32bit memory trace file to 16 bit traces by removing the two MSB bytes
import re
import collections
import subprocess
import sys
import argparse


parser = argparse.ArgumentParser()


parser.add_argument("--cubedata", required=True,help="Filename of the addresses for data --cubedata <loop_LN121_0.txt>")
args = parser.parse_args()


data_inp =  str(args.cubedata)
dm_data_file = open("data_modi.txt","w+")
dm_src_read_file = open(data_inp,"r")

dataToWrite = {}
dataVal = {}
dataVal_Ordered = {}
dataToWrite_backup = {}
lines_n = []
lines = dm_src_read_file.readlines()
########################################################################
    #Since the software is still 32 bit need to skip the MSB of each data word, Hence skipping the MSB lines
for idx,line in enumerate(lines):
    allField = line.split(',')
    if(allField[1].isdecimal()):
        if (((int(int(allField[1])) % 4) ==0) or ((int(int(allField[1])) % 4) ==1)):
            new_addr = (((int((int(allField[1]))/4))*2) + ((int(allField[1])) % 4));
            #allField_modi = [str(new_addr),allField[1],allField[2]]
            allField_modi = [allField[0],str(new_addr),allField[2],allField[3]]
            allField_modi_str =','.join(allField_modi)
            lines_n.append(allField_modi_str.strip())
    else:
        lines_n.append(line.strip());

for line in lines_n:
   # print(line)
   dm_data_file.write(line + "\n")
##########################################################################
dm_data_file.close()

