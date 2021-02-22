import os 

# cgra tile
# 1 2
# 3 4 
# tile1_x = lambda x,y: 
mem_alloc = []
alloc_file = open('./pedometer_with_morpher/loop_pedometer_INNERMOST_LN1_0_mem_alloc.txt', 'r')
alloc_file.readline()
line = alloc_file.readline()
while line:
  mem_alloc.append(int(line.split(",")[1]))
  line = alloc_file.readline()
print(mem_alloc)

def add_config(configs, x_shift_value, y_shift_value, allo_shift_value):
  config_file = open('./pedometer_with_morpher/pedometer_INNERMOST_LN1_PartPred_DFG.xml_DP1_XDim=4_YDim=4_II=7_MTP=1_binary.bin', 'r')
  config_lines  = config_file.readlines() # skip first line
  context_index = -1
  for line in config_lines:
    if "Y=0" in line and "X=0" in line:
      context_index += 1
    
    if "Y=" in line:
      print("old line", line)
      x_y = line.split(",")[0]
      bits = line.split(",")[1]
      x = x_y.split(" ")[0].split("=")[1]
      y = x_y.split(" ")[1].split("=")[1]
      # print(x,y, bits)
      # print(bits[2:29])
      const = int(bits[2:29],2)
      
      newline = "Y=" + str( int(x)+x_shift_value)+ " X=" + str( int(y)+y_shift_value)+","
      # this may not be totally right, but should be fine for pedotmeter and gemm
      if bits[1] == "1" and bits[29:34]=="00001" and const in mem_alloc:
        #configuration valid
        
        print( "const", const)
        new_const =  const + allo_shift_value
        binstr  = "{0:b}".format(new_const) 
        complement_len = 27 - len(binstr)
        binstr =  complement_len * "0" +binstr
        # print(binstr)
        newline += bits[0:2]+ binstr + bits[29:]
      else:
        newline += bits
      configs [context_index] += newline
      print("new line", newline)
      print()
  return configs

configs = [] 
config_file = open('./pedometer_with_morpher/pedometer_INNERMOST_LN1_PartPred_DFG.xml_DP1_XDim=4_YDim=4_II=7_MTP=1_binary.bin', 'r')
config_lines  = config_file.readlines() # skip first line
context_index = -1
for line in config_lines:
  if "Y=0" in line and "X=0" in line:
    context_index += 1
    configs.append("")
  if "Y=" in line:
    configs[context_index] += line

configs = add_config(configs, 4 , 0, 0)
for config in configs:
  print(config)


