#include "printf.h"
#include "uart.h"
#include "imx6ul.h"
#include "common.h"

#define SOH		0x01
#define	EOT		0x04
#define	ACK		0x06
#define NAK		0x15
#define	CAN		0x18
#define	CSYM	0x43

int xmodem_download(unsigned int addr)
{
	unsigned char buf[256], temp, seq;
	unsigned int i, offset;
	int result;
	int flag = 0;
	
	printf("addr: 0x%x ", addr);

	/* clear uart1 receive buf */
	while(1) {
		if ((UART1->USR2 & 0x1) == 0)
			break;
		i = UART1->URXD;
	}
	printf("Rdy\n");
	
	/* wait for download */
	offset = 0;
	seq = 0;	
	while(1)
	{
		result = getUartReceiveBufLenTimeout(buf, 132);
		if ( result == 0 )
		{
			if ( flag == 0 )
			{
				uart_putc(NAK);
			}
			else
			{
				uart_putc(ACK);
				uart_putc(ACK);
			}				
			
			continue;
		}
		else if ( result < 132 )
		{
			if ( buf[0] == EOT )
			{
				uart_putc(ACK);
				printf("\nEND\n");
				break;		
			}
			else if ( buf[0] == CAN )
			{
				uart_putc(ACK);
				printf("CNL\n");
				break;		
			}
			else
			{
				uart_putc(ACK);
			}
			
		}
		else
		{
			if ( flag == 0 )
			{
				flag = 1;
				uart_putc(ACK);
				uart_putc(ACK);
			}
			
			uart_putc(ACK);
			if ( buf[0] != SOH )
			{
				continue;
			}
		
			temp = 0;
			for ( i = 3; i < 131 ; i ++)
			{
				temp += buf[i];
			}
			if ( temp != buf[i] )
			{
				uart_putc(CAN);
				printf("err\r\n");		
				break;
			}
			else
			{
				seq ++;
				if ( seq != buf[1] )
				{
					uart_putc(CAN);
					printf("err\r\n");		
					break;
				}
	
				memcpy((char *)(addr + offset), buf + 3, 128);
				if ( memcmp((char *)(addr + offset), (char *)buf + 3, 128) != 0 )
				{
					uart_putc(CAN);
					printf("err\r\n");		
					break;
				}
				
				offset += 128;
			}
		}
	}
	return 0;
}

