#include "printf.h"
#include "uart.h"
#include "common.h"

int  str2uint(char * buf, unsigned int *value);
void Trim(char* command);
int  getCommandSubStr(char* command, int num, char* subStr);
void parsePara(char* command, unsigned int *para1, unsigned int *para2);

void cmdLoop(void)
{
	unsigned int recvCommandBufLen = 0;
	char recvCommandBuf[128];
	char subStr[128];
	unsigned int loadBeginAddr = 0x80000000;
	unsigned int runAddr = 0x80000000;
	unsigned int memAddr = 0x80000000, memLen = 8;

	memset(recvCommandBuf, 0, sizeof(recvCommandBuf));
	uart_putc('c');
	uart_putc('m');
	uart_putc('d');

	while(1) {
		unsigned char ch;
		ch  = uart_getc();
		if ( ch == 8 ) // backspace
			{
			uart_putc(ch);
			uart_putc(' ');
			uart_putc(ch);
			if ( recvCommandBufLen > 0 )
			{
				recvCommandBufLen --;
				recvCommandBuf[recvCommandBufLen] = 0;
			}
			continue;
		}

		uart_putc(ch);
		recvCommandBuf[recvCommandBufLen ++] = ch;
			
		if ( ch == '\r' || ch == '\n')
		{
			uart_putc('\n');
			
			Trim(recvCommandBuf);

			if ( getCommandSubStr(recvCommandBuf, 0, subStr) == 0 )
			{
				if ( strcmp(subStr, "xload") == 0 )
				{
					parsePara(recvCommandBuf, &loadBeginAddr, NULL);
					xmodem_download(loadBeginAddr);
				}
				else if ( strcmp(subStr, "run") == 0 )
				{
					unsigned int (*funPtr)(void);

					parsePara(recvCommandBuf, &runAddr, NULL);
					printf("Run at 0x%x\r\n", runAddr);
						
					funPtr = (unsigned int (*)(void))runAddr;
					funPtr();
				}
				else if ( strcmp(subStr, "mem") == 0 )
				{
					memLen = 8;
					parsePara(recvCommandBuf, &memAddr, &memLen);
					memDisp((void *)memAddr, memLen);
					memAddr += memLen * 32;
				}
				else if ( strcmp(subStr, "memset") == 0 )
				{
					unsigned int value = 0;
					parsePara(recvCommandBuf, &memAddr, &value);
						
					*((unsigned int *)memAddr) = value;
				}
				else
				{
					printf("cmd err\r\n");
				}
			}
			memset(recvCommandBuf, 0, strlen(recvCommandBuf));
			recvCommandBufLen = 0;
			uart_putc('#');
		}
	}
	return ;
}

void Trim(char* command)
{
	int i = 0;
	char szBuf[128];
	
	i = strlen(command) - 1;
	while (i >= 0)
	{
		if (command[i] != ' ' && command[i] != '\t' && command[i] != '\n' && command[i] != '\r' && command[i] != '\0')
		{
			command[i+1] = '\0';
			break;
		}
		i--;
	}
	i = 0;
	while((command[i] == ' ') || (command[i] == '\n') || (command[i] == '\r') || (command[i] == '\t')) i++;

	strcpy(szBuf, (command + i));
	strcpy(command, szBuf);
}

int getCommandSubStr(char * command, int num, char * subStr)
{
	int i, j, temp, len;
	
	len = strlen(command);
	if ( len == 0 )
		return -1;
	
	temp = 0;
	j = 0;
	for ( i = 0; i < len; i ++)
	{
		if ( command[i] == ' ' || command[i] == '\t' )
			continue;
		if ( temp == num )
		{
			while( i < len )
			{
				subStr[j++] = command[i++];
				if ( command[i] == ' ' || command[i] == '\t' || command[i] == ',')
				{
					break;
				}
			}
			
			subStr[j] = 0;			
			return 0;
		}
		else
		{
			while( i < len )
			{
				if ( command[i] == ' ' || command[i] == '\t' || command[i] == ',' )
				{
					break;
				}
				i ++;
			}
		}
		temp ++;
	}
	
	return -1;
}

void parsePara(char* recvCommandBuf, unsigned int *para1, unsigned int *para2)
{
	char subStr[128];
	
	if ( para1 != NULL )
	{
		if ( getCommandSubStr(recvCommandBuf, 1, subStr) == 0 )
		{
			str2uint(subStr, para1);
		}
	}
	
	if ( para2 != NULL )
	{
		if ( getCommandSubStr(recvCommandBuf, 2, subStr) == 0 )
		{
			str2uint(subStr, para2);
		}
	}
}


int str2uint(char * buf, unsigned int *value)
{
	int i, len;
	int tmp;
	unsigned int sum = 0;
	
	len = strlen(buf);
	if ( len > 8 )
		return -1;

	for( i = 0; i < len; i++)
	{
		if( (buf[i] < 0x30 || buf[i] > 0x39) && (buf[i]  < 'a' || buf[i] > 'f') )
			return -1;

		if( buf[i] < 0x40 )
		{
			tmp = buf[i]-0x30;
		}else
		{
			tmp = buf[i]-0x60+9;
		}
		sum = (sum << 4) + tmp;
	}
	*value = sum;
	return  0;
}
