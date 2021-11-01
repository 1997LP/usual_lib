TARGET   ?=bsp

CC		 := gcc

#.h文件的路径
INCDIRS	 := include 

#.c .s .S文件的路径
SRCDIRS  := src

INCLUDDE := $(patsubst %,-I %, $(INCDIRS))				  

CFILES   := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

CFILENDIR:= $(notdir $(CFILES))

COBJS    := $(patsubst %, obj/%, $(CFILENDIR:.c=.o))
OBJS     := $(COBJS)

#指定搜索目录
VPATH    := $(SRCDIRS)

$(TARGET).bin : $(OBJS)
	$(CC)  -o $@ $^ 


$(COBJS) : obj/%.o : %.c
	$(CC) -c $(INCLUDE) -o $@ $<	

clean:
	rm -rf  $(TARGET).bin $(OBJS)
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
