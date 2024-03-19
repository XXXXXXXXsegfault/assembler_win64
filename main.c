#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define getch _getch

#include "xmalloc.c"
#include "string.c"
FILE *fpi,*fpo,*fpe;
char *get_s(void)
{
	char *str;
	int c;
	str=NULL;
	while((c=getchar())>0&&c!='\n')
	{
		str=str_c_app(str,c);
	}
	return str;
}
void error(int line,char *msg)
{
	char *str;
	str=xstrdup("line ");
	str=str_i_app(str,line);
	str=str_s_app(str,": error: ");
	str=str_s_app(str,msg);
	puts(str);
	puts("Please press any key to continue.");
	getch();
	exit(2);
}
void open_files(void)
{
	char *str;
	printf("Source file: ");
	str=get_s();
	if(str!=NULL)
	{
		fpi=fopen(str,"r");
	}
	if(fpi==NULL)
	{
		puts("Cannot open file.");
		puts("Please press any key to continue.");
		getch();
		exit(3);
	}
	free(str);
	printf("Output file: ");
	str=get_s();
	if(str!=NULL)
	{
		fpo=fopen(str,"wb");
	}
	if(fpo==NULL)
	{
		puts("Cannot create file.");
		puts("Please press any key to continue.");
		getch();
		exit(3);
	}
	free(str);
	printf("Debug file (optional): ");
	str=get_s();
	if(str!=NULL)
	{
		fpe=fopen(str,"wb");
	}
	free(str);
}
int name_hash(char *str)
{
	unsigned int hash;
	hash=20000;
	while(*str)
	{
		hash=(hash<<11|hash>>21)+*str;
		++str;
	}
	return hash%1021;
}
unsigned long long int pc,data_size,data_addr;
struct lines *l;
int stage;
int generate_cui;
#define PE_OFF 0x401000
#define PE_BASE 0x400000
#include "load_file.c"
#include "dll.c"
#include "pe.c"
#include "read_word.c"
int format_hash(char *format)
{
	char c;
	int brackets;
	unsigned int hash;
	brackets=0;
	hash=301;
	while(c=*format)
	{
		if(c=='(')
		{
			++brackets;
		}
		else if(c==')')
		{
			--brackets;
		}
		else if(!brackets)
		{
			if(c=='*')
			{
				hash=hash*97+31;
			}
			else if(c==',')
			{
				hash=hash*89+19;
			}
			else if(c=='%')
			{
				hash=hash*47+173;
			}
		}
		++format;
	}
	return hash%1021;
}
#include "label.c"
#include "ins/ins.c"
#include "pseudo_op.c"
int _start(void)
{
	char *str,*word;
	struct label *label;
	int s,n;
	open_files();
	data_addr=0x20000000;
	load_file();
	pe_header.entry=0;
	pc=PE_OFF;
	ins_init();
	spos=0;
	l=lines_head;
	while(l)
	{
		str=skip_spaces(l->str);
		l->ins_len=0;
		l->ins_buf2=0;
		l->needs_recompile=0;
		if(*str&&*str!='#')
		{
			if(*str=='.')
			{
				parse_pseudo_op(str+1);
			}
			else if(*str=='@')
			{
				++str;
				word=read_word(&str);
				if(label_tab_find(word))
				{
					error(l->line,"label redefined.");
				}
				label_tab_add(word);
				l->needs_recompile=1;
			}
			else
			{
				ins_write(str);
			}
		}
		l=l->next;
	}
	stage=1;
	data_addr=size_align(spos)+0x1000+PE_BASE;
	do
	{
		s=0;
		n=0;
		spos=0;
		pe_header.entry=0;
		pc=PE_OFF;
		l=lines_head;
		while(l)
		{
			l->ins_pos=pc;
			if(l->needs_recompile)
			{
				str=skip_spaces(l->str);
				if(*str=='@')
				{
					++str;
					word=read_word(&str);
					if(label=label_tab_find(word))
					{
						if(label->value!=pc)
						{
							label->value=pc;
							s=1;
						}
					}
					free(word);
				}
				else
				{
					l->ins_len=0;
					free(l->ins_buf2);
					l->ins_buf2=0;
					if(*str=='.')
					{
						parse_pseudo_op(str+1);
					}
					else
					{
						ins_write(str);
					}
				}
			}
			else
			{
				pc+=l->ins_len;
				spos+=l->ins_len;
			}
			l=l->next;
			++n;
		}
	}
	while(s);
	l=lines_head;
	spos=0;
	while(l)
	{
		spos+=l->ins_len;
		l=l->next;
	}
	dll_import_write();
	mkpe();
	fclose(fpi);
	fclose(fpo);
	if(fpe!=NULL)
	{
		fclose(fpe);
	}
	return 0;
}
