# python3 automate.py --cubeins {file_from_mapper}.bin --cubedata {data_file}.txt --cubetime {II+1} --cubeclus {no_clusters}

import re
import collections
import subprocess
import sys
import argparse

## PYTHON FILE with constants
import hycube_config as HY

parser = argparse.ArgumentParser()

parser.add_argument("--cubeins", required=True,help="Filename of the addresses for instructions --cubeins <test4.trc>")
parser.add_argument("--cubedata", required=True,help="Filename of the addresses for data --cubedata <loop_LN121_0.txt>")
parser.add_argument("--cubetime", required=True,help="Filename of the addresses for instructions --cubetime <6>")
parser.add_argument("--cubeclus", required=True,help="How many clusters needs to be on --cubeclus <2>")
args = parser.parse_args()

ins_inp  =  str(args.cubeins)
data_inp =  str(args.cubedata)
TIMEEXEC = int(args.cubetime)
no_clusters_on = int(args.cubeclus)

cm_addr_file = open("./mem_files/addr_ins.trc","w+")
dm_addr_file = open("./mem_files/addr_data.trc","w+")
dm_data_file = open("./mem_files/data.trc","w+")
results_expected_file = open("results_expected.trc", "w+")
dm_src_read_file = open(data_inp,"r")

######################################
# CM BINARY
######################################

###### step 1: split CM instructions by tiles
num_inst_per_tile = 0
for cluster in HY.clusters_cm_to_write:
    for i in HY.CLUSTER_ROW_INDEXES[cluster]:
        for j in HY.CLUSTER_COL_INDEXES[cluster]:

            tile_num = (i*HY.TILES_NUM_ROWS) + j  

            # count instructions for tile
            cmdtorun = "grep -c " + "\"Y=" + str(i) + " " + "X=" + str(j) + "\"" + " " + ins_inp
            make_process = subprocess.Popen(cmdtorun,shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
            (output, err) = make_process.communicate()
            line_count = int(output)
            # print(output)

            if(num_inst_per_tile==0):
                num_inst_per_tile = line_count
            else:
                if not((line_count == 0) or (num_inst_per_tile == line_count)):
                    print("automate.py [WARNING]: Number of Instructions no equal for all tiles.\nExpected: ", num_inst_per_tile, ", tile ", tile_num, ": ", line_count)

            # write CM files        
            cmdtorun = "grep -i " + "\"Y=" + str(i) + " " + "X=" + str(j) + "\"" + " " + ins_inp + " >" + " ./mem_files/tile" + str(tile_num) + "_inst.trc"
            make_process = subprocess.Popen(cmdtorun,shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
            (output, err) = make_process.communicate()

###### step 2: prune instructions, convert 64b to 16b
for cluster in HY.clusters_cm_to_write:
    for i in HY.CLUSTER_ROW_INDEXES[cluster]:
        for j in HY.CLUSTER_COL_INDEXES[cluster]:
            tile_num = (i*HY.TILES_NUM_ROWS) + j  

            cmdtorun = "sed -i " + "\'s/Y=" + str(i) + " " + "X=" + str(j) + ",//\'" + " ./mem_files/tile" + str(tile_num) + "_inst.trc"
            #print(cmdtorun)
            make_process = subprocess.Popen(cmdtorun,shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
            (output, err) = make_process.communicate()

            #### Append NOPs to fill until cubetime
            cmdtorun = "grep -c ^" +  " ./mem_files/tile" + str(tile_num) + "_inst.trc"
            make_process = subprocess.Popen(cmdtorun,shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
            (output, err) = make_process.communicate()
            line_count = int(output)

            line_count_diff = TIMEEXEC - int(line_count)
            if(line_count_diff > 0):
                for count in range(0, line_count_diff):
                    cmdtorun = "echo \\" + HY.NOP_INST + " >> " + " ./mem_files/tile" + str(tile_num) + "_inst.trc"
                    make_process = subprocess.Popen(cmdtorun,shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
                    (line_count, err) = make_process.communicate()

            #### convert to 16b
            # Construct cmdtorun according to bit widths
            split_iterations = int(HY.CM_WIDTH/HY.DATA_WIDTH)
            cmdtorun = "sed -i \'"
            for x in range(1, split_iterations):
                if(x!=1):
                    cmdtorun = cmdtorun + ";"
                cmdtorun = cmdtorun + "s/./&\\n/" + str(x*HY.DATA_WIDTH + (x-1))
            cmdtorun = cmdtorun + "\' ./mem_files/tile" + str(tile_num) + "_inst.trc" 
            # print(cmdtorun)

            make_process = subprocess.Popen(cmdtorun,shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
            (output, err) = make_process.communicate()

###### step 3: GENERATE CM binaries
cmdtorun = "cat "
for cluster in HY.clusters_cm_to_write:
    for i in HY.CLUSTER_ROW_INDEXES[cluster]:
        for j in HY.CLUSTER_COL_INDEXES[cluster]:
            tile_num = (i*HY.TILES_NUM_ROWS) + j
            cmdtorun = cmdtorun + "./mem_files/tile" + str(tile_num) + "_inst.trc "
cmdtorun = cmdtorun + "> ./mem_files/ins.trc"

make_process = subprocess.Popen(cmdtorun,shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
(output, err) = make_process.communicate()

if(HY.WRITE_CM):
    addr_mem_type = HY.ADDR_MEM_TYPE_ENCODING["CM"]
    SRAMCELL = int(HY.CM_WIDTH/HY.DATA_WIDTH)

    addr_tile_sel_format = '0' + str(HY.CM_SEL_BITS) + 'b'
    addr_row_sel_format = '0' + str(HY.CM_ROW_SEL_BITS) + 'b'
    addr_byte_sel_format = '0' + str(HY.CM_BYTE_SEL_BITS) + 'b'

    for cluster in HY.clusters_cm_to_write:
        for i in HY.CLUSTER_ROW_INDEXES[cluster]:
            # For each tile
            for j in HY.CLUSTER_COL_INDEXES[cluster]:
                tile_num = (i*HY.TILES_NUM_ROWS) + j
                # For each row
                for k in range(0,TIMEEXEC):
                    # For each byte
                    for l in range(SRAMCELL-1, -1, -1):
                        addr_tile_sel = format(tile_num, addr_tile_sel_format)
                        addr_row_sel = format(k,addr_row_sel_format)
                        addr_byte_sel = format(l,addr_byte_sel_format)

                        totaladdress = HY.ADDR_MSB_ZEROES + addr_mem_type + HY.ADDR_CM_TOP_ZEROES + addr_tile_sel + HY.ADDR_CM_MID1_ZEROES + addr_row_sel + HY.ADDR_CM_MID2_ZEROES + addr_byte_sel + HY.ADDR_CM_LSB_ZEROES
                        cm_addr_file.write(totaladdress + "\n")

cm_addr_file.close()

######################################
# DM BINARY
######################################
if(HY.WRITE_DM):
    addr_mem_type = HY.ADDR_MEM_TYPE_ENCODING["DM"]

    dataToWrite = {}
    dataVal = {}
    dataVal_Ordered = {}
    dataToWrite_backup = {}
    lines_n = []
    lines = dm_src_read_file.readlines()
########################################################################
#    for line in lines:
#        print(line)
    #Since the software is still 32 bit need to skip the MSB of each data word, Hence skipping the MSB lines
#    for idx,line in enumerate(lines):
#        allField = line.split(',')
#        if(allField[0].isdecimal()):
#            if (((int(int(allField[0])) % 4) ==2)):
#                lines.remove(line)
#            elif(((int(int(allField[0])) % 4) ==3)):
#                lines.remove(line)
#            else:
#                new_addr = (((int((int(allField[0]))/4))*2) + ((int(allField[0])) % 4));
#                #allField_modi = [str(new_addr),allField[1],allField[2]]
#                allField_modi = [str(new_addr),allField[1]]
#                allField_modi_str =','.join(allField_modi)
#                lines_n.append(allField_modi_str)
#        else:
#            lines_n.append(line);
#
#    #for line in lines_n:
#    #    print(line)
##########################################################################
    #if(HY.dmem_exec_end_line != None):
    #    lines.append(HY.dmem_exec_end_line)
    print("automate.py [INFO]: DM details  size-> ",len(lines))
    for line in lines:
        allField = line.split(',')
        match = re.match(r"([0-9]+)", allField[0], re.I)
        if(match):
            allField_dm =  int((int(allField[0])//(HY.DATA_WIDTH/8)) // HY.DM_BLOCK_DEPTH) # DM row index // DM depth
            allField_RowandByte = int((int(allField[0])) % (HY.DM_BLOCK_DEPTH * (HY.DATA_WIDTH/8)))
           # print("automate.py [INFO]: DM details  allField_dm-> ",allField_dm,"   allField_RowandByte->",allField_RowandByte)
            #if allField_dm not in dataToWrite.keys():
            #    dataToWrite[allField_dm] = []
            #    dataToWrite_backup[allField_dm] = []

            #dataToWrite_backup[allField_dm].append(allField_RowandByte)
            #dataToWrite[allField_dm].append([allField_RowandByte, allField[1], allField[2]])
            #dataToWrite[allField_dm].append([allField_RowandByte, allField[1]])
            key1 = 2*allField_dm;
            key2 = 2*allField_dm+1;
            if key1 not in dataToWrite.keys():
                dataToWrite[key1] = []
                dataToWrite_backup[key1] = []

            if key2 not in dataToWrite.keys():
                dataToWrite[key2] = []
                dataToWrite_backup[key2]= []

            dataToWrite_backup[key1].append(allField_RowandByte)
            dataToWrite_backup[key2].append(allField_RowandByte)

            dataToWrite[key1].append([allField_RowandByte, allField[1], allField[2]])
            dataToWrite[key2].append([allField_RowandByte, allField[1], allField[2]])

   #for dm_index in HY.dmems_to_write:
   #    if dm_index not in dataToWrite.keys():
            #print("automate.py [INFO]: DM details  allField_dm-> ",dm_index)
   #        dataToWrite[dm_index] = []
   #        dataToWrite_backup[dm_index] = [] 

    for dm_index in dataToWrite.keys():
        print("automate.py [INFO]: DM details  dm_index-> ",dm_index) 
        dataVal[dm_index] = {}
        dataVal_Ordered[dm_index] = {}

        for custom_range in HY.dmem_range_to_write_zeroes:
            for i in range(custom_range[0], custom_range[1]):
                if i not in dataToWrite_backup[dm_index]:
                    dataToWrite[dm_index].append([str(i), "0", "0"])

    for dm in dataToWrite:
        for i in dataToWrite[dm]:
            byte = int(i[0])
            row = int(i[0])//2
            if row not in dataVal[dm].keys():
                dataVal[dm][row] = [0, 0, 0, 0] # [LSB pre-run, LSB post-run, MSB pre-run, MSB post-run]
            if(byte%2):
                dataVal[dm][row][2] = int(i[1]) # pre-run data (byte)
                dataVal[dm][row][3] = int(i[2]) # post-run data (byte)
            else:
                dataVal[dm][row][0] = int(i[1]) # pre-run data (byte)
                dataVal[dm][row][1] = int(i[2]) # post-run data (byte)

        dataVal_Ordered[dm] = collections.OrderedDict(sorted(dataVal[dm].items()))

    '''
    # After sorting, dataVal_Ordered will be as follows:
    # OrderedDict([(0, [pre-LSB, post-LSB, pre-MSB, post-MSB]), 
    #              (2, [pre-LSB, post-LSB, pre-MSB, post-MSB]),
    #               ...])
    # pre: pre-run data
    # post: post-run data
    '''
    addr_dm_sel_format = '0' + str(HY.DM_SEL_BITS) + 'b'
    addr_row_sel_format = '0' + str(HY.DM_ROW_SEL_BITS) + 'b'
    addr_byte_sel_format = '0' + str(HY.DM_BYTE_SEL_BITS) + 'b'

    for dm in dataToWrite:
        if ((dm %2)==0): 
            addr_dm_sel = format(dm, addr_dm_sel_format)
            print("automate.py [INFO]: Writing to DM ", dm, ", Rows ", min(dataVal_Ordered[dm].keys()), " to ", max(dataVal_Ordered[dm].keys()))
            for i in dataVal_Ordered[dm].keys():
                addr_row_sel = format(i, addr_row_sel_format)
                addr_byte_sel = format(0, addr_byte_sel_format)
                totaladdress = HY.ADDR_MSB_ZEROES + addr_mem_type + HY.ADDR_DM_TOP_ZEROES + addr_dm_sel + HY.ADDR_DM_MID1_ZEROES + addr_row_sel + HY.ADDR_DM_MID2_ZEROES + addr_byte_sel + HY.ADDR_DM_LSB_ZEROES
                dm_addr_file.write(totaladdress + "\n")

                ValinKey = dataVal_Ordered[dm][i]

                pre_dataLSB = ValinKey[0]
                post_dataLSB = ValinKey[1]
                pre_dataMSB = ValinKey[2]
                post_dataMSB = ValinKey[3]

                pre_data2file = format(pre_dataMSB,'08b') + format(pre_dataLSB,'08b')
                post_data2file = format(post_dataMSB,'08b') + format(post_dataLSB,'08b')
                dm_data_file.write(pre_data2file + "\n")
                results_expected_file.write(post_data2file + "\n")
                if(i==4095):
                    print("automate.py [INFO]: DM details  addr-> ",totaladdress , "MSB ", pre_dataMSB, " LSB ", pre_dataLSB)
dm_addr_file.close()
dm_data_file.close()
results_expected_file.close()

######################################
# Collate for TOTALDATA, TOTALADDR
######################################
#cmdtorun = "cat ./mem_files/ins.trc ./mem_files/data.trc > totaldata.trc"
cmdtorun = "cat ./mem_files/data.trc > totaldata.trc"
make_process = subprocess.Popen(cmdtorun,shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
(output, err) = make_process.communicate()

#cmdtorun = "cat ./mem_files/addr_ins.trc ./mem_files/addr_data.trc > totaladdr.trc"
cmdtorun = "cat ./mem_files/addr_data.trc > totaladdr.trc"
make_process = subprocess.Popen(cmdtorun,shell=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
(output, err) = make_process.communicate()

if(HY.WRITE_LUT):
    totaldata = open("totaldata.trc","a+")
    totaladdr = open("totaladdr.trc","a+")
    extraaddr = "1000000000000000000\n1000000000000000001\n1000000000000000010\n1000000000000000011\n"
    extradata = "0000000000001011\n0000000000000000\n0000000000000000\n0000000000000000\n"
    totaldata.write(extradata)
    totaladdr.write(extraaddr)

if(HY.WRITE_CLUS_EN):
    totaldata = open("totaldata.trc","a+")
    totaladdr = open("totaladdr.trc","a+")
    extraaddr = "1100000000000000000"
    
    clus_en_word = HY.DATA_WIDTH * ['1']
    for cluster in HY.clusters_cm_to_write:
        clus_en_word[HY.DATA_WIDTH-1-cluster] = '0'
#    for cluster in range(no_clusters_on)
#        clus_en_word[HY.DATA_WIDTH-1-cluster] = '0'
    extradata = "".join(clus_en_word)
    totaldata.write(extradata)
    totaladdr.write(extraaddr)

