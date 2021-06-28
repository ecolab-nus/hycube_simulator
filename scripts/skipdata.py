# python3 automate.py --cubeins {file_from_mapper}.bin --cubedata {data_file}.txt --cubetime {II+1} --cubeclus {no_clusters}

import re
import collections
import subprocess
import sys
import argparse

## PYTHON FILE with constants
#import hycube_config as HY

parser = argparse.ArgumentParser()

#parser.add_argument("--cubeins", required=True,help="Filename of the addresses for instructions --cubeins <test4.trc>")
parser.add_argument("--cubedata", required=True,help="Filename of the addresses for data --cubedata <loop_LN121_0.txt>")
#parser.add_argument("--cubetime", required=True,help="Filename of the addresses for instructions --cubetime <6>")
#parser.add_argument("--cubeclus", required=True,help="How many clusters needs to be on --cubeclus <2>")
args = parser.parse_args()

#ins_inp  =  str(args.cubeins)
data_inp =  str(args.cubedata)
#TIMEEXEC = int(args.cubetime)
#no_clusters_on = int(args.cubeclus)

#cm_addr_file = open("./mem_files/addr_ins.trc","w+")
#dm_addr_file = open("./mem_files/addr_data_skip.trc","w+")
dm_data_file = open("data_modi.txt","w+")
#results_expected_file = open("results_expected.trc", "w+")
dm_src_read_file = open(data_inp,"r")

#if(HY.WRITE_DM):
#addr_mem_type = HY.ADDR_MEM_TYPE_ENCODING["DM"]

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

dm_data_file.write("loopstart,0,1,0" + "\n")
dm_data_file.write("loopend,0,0,1" + "\n")
##########################################################################
dm_data_file.close()

