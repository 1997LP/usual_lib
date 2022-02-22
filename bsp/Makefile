CROSS_COMPILE 	?= arm-linux-gnueabihf-
TARGET		  	?= bsp

CC 				:= $(CROSS_COMPILE)gcc
LD				:= $(CROSS_COMPILE)ld
OBJCOPY 		:= $(CROSS_COMPILE)objcopy
OBJDUMP 		:= $(CROSS_COMPILE)objdump

INCDIRS 		:= imx6ul \    #所有.h文件路径
				   bsp/clk \
				   bsp/led \
				   bsp/delay 
				   			   
SRCDIRS			:= project \  #所有.c,.h文件路径路径
				   bsp/clk \
				   bsp/led \
				   bsp/delay 
				   
				   
INCLUDE			:= $(patsubst %, -I %, $(INCDIRS))    #给头文件目录添加-T选项

SFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))  #所有.s文件，包含绝对路径
CFILES			:= $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))  #所有.c文件，包含绝对路径

SFILENDIR		:= $(notdir  $(SFILES))  #所有.s文件，不包含路径
CFILENDIR		:= $(notdir  $(CFILES))  #所有.c文件，不包含路径

SOBJS			:= $(patsubst %, obj/%, $(SFILENDIR:.S=.o))  #.s文件生成的.o文件存在路径
COBJS			:= $(patsubst %, obj/%, $(CFILENDIR:.c=.o))  #.c文件生成的.o文件存放路径
OBJS			:= $(SOBJS) $(COBJS)  

VPATH			:= $(SRCDIRS)            #指定搜索路径

.PHONY: clean
	
$(TARGET).bin : $(OBJS)
	$(LD) -Timx6ul.lds -o $(TARGET).elf $^
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

$(SOBJS) : obj/%.o : %.S
	$(CC) -Wall -nostdlib -c -O2  $(INCLUDE) -o $@ $<

$(COBJS) : obj/%.o : %.c
	$(CC) -Wall -nostdlib -c -O2  $(INCLUDE) -o $@ $<
	
clean:
	rm -rf $(TARGET).elf $(TARGET).dis $(TARGET).bin $(COBJS) $(SOBJS)

	
