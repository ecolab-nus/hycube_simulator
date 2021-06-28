import os 

# cgra tile
# 1 2
# 3 4 
# tile1_x = lambda x,y:
mem_each_tile = 16384 
mem_alloc = []
alloc_file = open('./mem_alloc.txt', 'r')
alloc_file.readline()
line = alloc_file.readline()
while line:
  mem_alloc.append(int(line.split(",")[1]))
  line = alloc_file.readline()
print(mem_alloc)

def add_config(configs, x_shift_value, y_shift_value, allo_shift_value, file_name):
  config_file = open(file_name, 'r')
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
    
      # loop start address change
      if bits[1] == "1" and ((bits[29:34]=="00001" and const in mem_alloc) or const == 16383 or const == 16382):
        #configuration valid
        
        #loop start: 16383 loop end:16382
        print( "const", const)
        if const == 16383: 
          new_const =  (allo_shift_value + 1 )  * mem_each_tile + - 1
        elif const == 16382:
          new_const =  (allo_shift_value + 1 )  * mem_each_tile + - 2
        else:
          new_const =   (allo_shift_value  )  * mem_each_tile + const # for 16 bit
        print( "new_const", new_const)
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



# duplicate configurations
configs = [] 
left_config_file = open('./left.bin', 'r')
config_lines  = left_config_file.readlines() # skip first line
context_index = -1
for line in config_lines:
  if "Y=0" in line and "X=0" in line:
    context_index += 1
    configs.append("")
  if "Y=" in line:
    configs[context_index] += line

configs = add_config(configs, 0 , 4,  2, "./right.bin")
configs = add_config(configs, 4 , 0, 1, "./left.bin")
configs = add_config(configs, 4 , 4,  3, "./right.bin")


# for config in configs:
#   print(config)


dumplicated_config = open("duplicated_config.bin", "w")
dumplicated_config.write("NPB,CONSTVALID,CONST,OPCODE,REGWEN,TREGWEN,REGBYPASS,PRED,OP1,OP2,NORTH,WEST,SOUTH,EAST\n")
for i in range(0,context_index+1):
  dumplicated_config.write(str(i)+"\n")
  dumplicated_config.write(configs[i])
  dumplicated_config.write("\n")
dumplicated_config.close()
