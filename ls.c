#include <time.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define MAXPATHLEN 4096
#define MAXNewArray 2048
char *pathnameDir;//record the root directory

typedef struct mc_col mc_Col;//the struct of split the fence
typedef struct mc_dir mc_Dir;//a directory all information structure
typedef struct mc_file mc_File;//the specific information of files

struct mc_col
{
	int32_t cnt;//fence count or column count
	int32_t rnt;//row count
	int32_t *lsp;//the width of each fence
};

struct mc_dir
{
	int32_t cnt;//the total amount of files in the directory
	int32_t iwid;//inode's width(max)
	int32_t lwid;//links'width(max)
	int32_t uwid;//uid width(max)
	int32_t gwid;//gid width(max)
	int32_t swid;//size(dev) width(max)
	mc_File *fp;//point to the files' pecific information in this directory
//	char name[MAXNAMLEN + 1];//dir name
};

struct mc_file
{
	mc_Dir *dp;//if this file is a directory, not NULL
	int32_t wid;//the length of filename.
	int32_t uid;
	int32_t gid;
	int32_t dev;
	int32_t mode;
	int32_t links;
	int64_t ino;
	int64_t size;
	int64_t atime;
	int64_t mtime;
	int64_t blockSize;
	int64_t blocks;
	int64_t ctime;
	char name[MAXNAMLEN + 1];
};

static int32_t col;//the terminal width
static int32_t Aflg;
static int32_t aflg;
static int32_t Cflg;
static int32_t cflg;
static int32_t dflg;
static int32_t Fflg;
static int32_t fflg;
static int32_t hflg;
static int32_t iflg;
static int32_t kflg;
static int32_t lflg;
static int32_t nflg;
static int32_t qflg;
static int32_t Rflg;
static int32_t rflg;
static int32_t Sflg;
static int32_t sflg;
static int32_t Lflg;
static int32_t tflg;
static int32_t uflg;
static int32_t wflg;
static int32_t xflg;
static int32_t oneflg;
static int32_t (*Printfunc)(mc_Dir *);//define the print function
static int32_t (*Cmpfunc)(const void *, const void *);//define the compare function
//ignore upper and lower case
char up_ascii(char tmp)
{
	if(tmp <= 'z' && tmp >= 'a')
	{
		return tmp - 32;
	}	
	return tmp;
}
//compare name funciton
static int32_t cmpname(const void *s1, const void *s2)
{
	char buf1[MAXPATHLEN];
	char buf2[MAXPATHLEN];
	char buf3[MAXPATHLEN];
	char buf4[MAXPATHLEN];
	int i;
	
	strcpy(buf1, ((mc_File *)s1)->name);
	strcpy(buf2, ((mc_File *)s2)->name);
	i = 0;
	do
	{
		buf3[i] = up_ascii(buf1[i]);
	}
	while(buf1[i++]);
	i = 0; 
        do   
        {    
                buf4[i] = up_ascii(buf2[i]);
        }    
        while(buf2[i++]);
	
	return strcmp(buf3, buf4);	
}
//compare name function plus -r
static int32_t cmpnameReverse(const void *s1, const void *s2)
{
	char buf1[MAXPATHLEN];
        char buf2[MAXPATHLEN];
        char buf3[MAXPATHLEN];
        char buf4[MAXPATHLEN];
        int i;
     
        strcpy(buf2, ((mc_File *)s1)->name);
        strcpy(buf1, ((mc_File *)s2)->name);
        i = 0; 
        do   
        {    
                buf3[i] = up_ascii(buf1[i]);
        }    
        while(buf1[i++]);
        i = 0; 
        do   
        {    
                buf4[i] = up_ascii(buf2[i]);
        }    
        while(buf2[i++]);
     
        return strcmp(buf3, buf4);
}
//compare size function
static int32_t cmpSize(const void *s1, const void *s2)
{
	return (((mc_File *)s2)->size - ((mc_File *)s1)->size);
}
//compare size funtion plus -r
static int32_t cmpSizeReverse(const void *s1, const void *s2)
{
	return (((mc_File *)s1)->size - ((mc_File *)s2)->size);
}


/*
split fench default(down page)
success: return the mc_Col struct
fail: return NULL
*/
static mc_Col* divcolDefault(mc_Dir *dp)
{
	mc_Col *cp;
	mc_File *fp;
	int32_t r, c;//r is row, c is column
	int32_t i,j, w, l;

	if(dp == NULL || (fp = dp->fp) == NULL)
	{
		return NULL;	
	}
	for(r = 1; ; r++)
	{
		l = 0;
		c = (dp->cnt / r) + (dp->cnt % r);
		for(j = 0; j < c; j++)
		{
			w = 0;
			for(i = j*r; i < j*r+r; i++)
			{
				if(i >= dp->cnt)
				{
					break;
				}
				if((fp[i].wid) > w)
				{
					w = fp[i].wid;
				}
			}
			l += w + 1;
		}
		if(l < col)
		{
			break;
		}
		
	}
	if((cp = malloc(sizeof(mc_Col))) == NULL)
	{
		return NULL;
	}
	cp->rnt = r;
	cp->cnt = c;
	if((cp->lsp = malloc(c * sizeof(int32_t))) == NULL)
	{
		free(cp);
		return NULL;
	}
	for(i = 0; i < c; i++)
	{
		w = 0;
		for(j = i*r; j < i*r+r; j++)
		{
			if(j >= dp->cnt)
			{
				break;
			}
			if((fp[j].wid) > w)
			{
				w = fp[j].wid;
			}
		}
		cp->lsp[i] = w;
	}
	
	return cp;
}
/*
split fench(across page)
success: return the mc_Col struct
fail: return NULL
*/
static mc_Col* divcol(mc_Dir *dp)
{
	mc_Col *cp;
	mc_File *fp;
	int32_t r, c;//r is row, c is column
	int32_t i,j, w, l;

	if(dp == NULL || (fp = dp->fp) == NULL)
	{
		return NULL;	
	}
	for(r = 1; ; r++)
	{
		l = 0;//the length of each row
		c = dp->cnt / r;
		for(i = 0; i < c; i++)		
		{
			w = 0;
			for(j = i; j < dp->cnt; j += c)//get the length(max) of filename in each column
			{
				if((fp[j].wid) > w)
				{
					w = fp[j].wid;
				}
			}
			l += w + 1;//got the length(max) of row
		}
		if(l < col)
		{
			break;//perfect if the max length of row less then the length of terminal
		}
		//or, increase the row number and loop again.
	}
	if((cp = malloc(sizeof(mc_Col))) == NULL)
	{
		return NULL;
	}
	cp->rnt = r;
	cp->cnt = c;
	if((cp->lsp = malloc(c * sizeof(int32_t))) == NULL)
	{
		free(cp);
		return NULL;
	}
	for(i = 0; i < c; i++)
	{
		w = 0;
		for(j = i; j < dp->cnt; j += c)
		{
			if((fp[j].wid) > w)
			{
				w = fp[j].wid;
			}
		}
		cp->lsp[i] = w;
	}
	
	return cp;
}

static int32_t freecol(mc_Col *cp)  
{  
    if(cp == NULL)  
    {   
        return 1;  
    }   
    if(cp->lsp)  
    {   
        free(cp->lsp);  
    }   
    free(cp);  
    return 0;  
}
//print Block for each file
static int32_t printBlock(mc_Dir *dp)
{
	mc_Col *cp;
	int32_t i, j, r,c;
	int64_t total;
	char *buf;
	char p[MAXNewArray + 1];

	if(dp == NULL || dp->fp == NULL)
	{
		return 1;
	}
	if(dp->cnt == 0)
	{
		printf("total 0\n");
		return 0;
	}
	total = 0;
	for(j = 0; j < dp->cnt; j++) 
        {    
                total += (dp->fp[j].size);    
        } 
	for(i = 0; i < dp->cnt; i++)	
	{		
                if(i == 0)
                {   
                        printf("total %ld\n", total);
                }
		sprintf(p, "%ld", dp->fp[i].blocks);
		buf = strcat(p, " ");
		buf = strcat(buf, dp->fp[i].name);
		strcpy(dp->fp[i].name,buf);
		dp->fp[i].wid = strlen(dp->fp[i].name);
	}
	if(xflg)
	{
		if((cp = divcol(dp)) == NULL)
		{
			return 1;
		}	
		if(oneflg)
		{
			 for(i = 0; i < dp->cnt; i++)
			{
				printf("%-*s\n", cp->lsp[i % cp->cnt], dp->fp[i].name);
			}
		}	
		else
		{
			for(i = 0; i < dp->cnt; i++)
			{
				printf("%-*s", cp->lsp[i % cp->cnt], dp->fp[i].name);
				if((i == 0 && dp->cnt > 1) || (i + 1) % cp->cnt)
				{
					putchar(' ');
				}
				else
				{
					putchar('\n');
				}
			}
			 if(dp->cnt % cp->cnt)
			{
				putchar('\n');	
			}
		}
	}
	else
	{
		if((cp = divcolDefault(dp)) == NULL)
		{	
			return 1;
		}
		if(oneflg)
		{
			for(i = 0; i < dp->cnt; i++)
			{
				printf("%-*s\n", cp->lsp[i % cp->cnt], dp->fp[i].name);
			}
		}
		else
		{
		 	for(r = 0; r < cp->rnt; r++) 
               		{	    
                       		i = r; 
                        	for(c = 0; c < cp->cnt; c++) 
                        	{    
     
                                	printf("%-*s ", cp->lsp[c], dp->fp[i].name);
                                	i  = i + cp->rnt;
                                	if(i >= dp->cnt)
                                	{    
                                        	break;
                                	}    
                        	}    
                        	putchar('\n');
     
    		       }
		}
	}
	freecol(cp);
	return 0;
}
//-F
static int32_t printSpecial(mc_Dir *dp)
{
	int32_t i,r,c;
	mc_Col *cp;
	char *buf;
	
	if(dp == NULL || dp->fp == NULL)
	{
		return 1;
	}
	if(dp->cnt == 0)
	{
		printf("total 0\n");
		return 0;
	}
	if(oneflg)//-1
	{
		if((cp = divcol(dp)) == NULL)
		{
			return 1;
		}
		for(i = 0; i < dp->cnt; i++)
		{
			if(S_ISDIR(dp->fp[i].mode))
			{
				buf = strcat(dp->fp[i].name, "/");
				printf("%-*s\n", cp->lsp[i % cp->cnt], buf);	
			}
			if(S_ISLNK(dp->fp[i].mode))
			{
				buf = strcat(dp->fp[i].name, "@");
				printf("%-*s\n", cp->lsp[i % cp->cnt], buf);
			}
			if(S_ISSOCK(dp->fp[i].mode))
			{
				buf = strcat(dp->fp[i].name, "=");
				printf("%-*s\n", cp->lsp[i % cp->cnt], buf);
			}
			if(S_ISFIFO(dp->fp[i].mode))
			{
				buf = strcat(dp->fp[i].name, "|");
				printf("%-*s\n", cp->lsp[i % cp->cnt], buf);
			}
			if(S_ISREG(dp->fp[i].mode))
			{
				if(access(dp->fp[i].name,  X_OK) == 0)
				{
					buf = strcat(dp->fp[i].name, "*");
					printf("%-*s\n", cp->lsp[i % cp->cnt], buf);
				}
				else
				{
					printf("%-*s\n", cp->lsp[i % cp->cnt], dp->fp[i].name);
				}
			}
			
		}
	}
	else
	{
		for(i = 0; i < dp->cnt; i++)
                {       
                        if(S_ISDIR(dp->fp[i].mode))
                        {       
				strcat(dp->fp[i].name, "/");
				dp->fp[i].wid = strlen(dp->fp[i].name);	
                        }
                        if(S_ISLNK(dp->fp[i].mode))
                        {       
				strcat(dp->fp[i].name, "@");    
                                dp->fp[i].wid = strlen(dp->fp[i].name);
                        }
                        if(S_ISSOCK(dp->fp[i].mode))
                        {       
				strcat(dp->fp[i].name, "=");
                                dp->fp[i].wid = strlen(dp->fp[i].name);
                        }
                        if(S_ISFIFO(dp->fp[i].mode))
                        {       
				strcat(dp->fp[i].name, "|");    
                                dp->fp[i].wid = strlen(dp->fp[i].name);
                        }
                        if(S_ISREG(dp->fp[i].mode))
                        {       
                                if(access(dp->fp[i].name,  X_OK) == 0)
                                {       
					strcat(dp->fp[i].name, "*");    
                                	dp->fp[i].wid = strlen(dp->fp[i].name);
                                }    
                                else 
                                {       
					dp->fp[i].wid = strlen(dp->fp[i].name);
                                }
                        }
		}
		if(xflg)
		{
			if((cp = divcol(dp)) == NULL)
                	{
                        	return 1;
                	}
        		for(i = 0; i < dp->cnt; i++)
        		{
				if(S_ISDIR(dp->fp[i].mode))
				{
					printf("%-*s", cp->lsp[i % cp->cnt], dp->fp[i].name);
				}
				if(S_ISLNK(dp->fp[i].mode))
				{
					printf("%-*s", cp->lsp[i % cp->cnt], dp->fp[i].name);
				}
				if(S_ISSOCK(dp->fp[i].mode))
				{
					printf("%-*s", cp->lsp[i % cp->cnt], dp->fp[i].name);
				}
				if(S_ISFIFO(dp->fp[i].mode))
				{
					printf("%-*s", cp->lsp[i % cp->cnt], dp->fp[i].name);
				}
				if(S_ISREG(dp->fp[i].mode))
               			{   
                        		if(access(dp->fp[i].name,  X_OK) == 0)
                        		{   
						printf("%-*s", cp->lsp[i % cp->cnt], dp->fp[i].name);
                        		}   
                       			else
                       			{   
						printf("%-*s", cp->lsp[i % cp->cnt], dp->fp[i].name);
                        		}	   
                		} 

                		if((i == 0 && dp->cnt > 1) || (i + 1) % cp->cnt)
                		{
                       			putchar(' ');
                		}
                		else
                		{
                       			putchar('\n');
                		}
        		}
        		if(dp->cnt % cp->cnt)
        		{
                		putchar('\n');
        		}
		}
		else
		{
			i = 0;
			if((cp = divcolDefault(dp)) == NULL)
                        {    
                                return 1;
                        }    
                        for(r = 0; r < cp->rnt; r++) 
                        {    
				i = r;
				for(c = 0; c < cp->cnt; c++)
				{
                                	if(S_ISDIR(dp->fp[i].mode))
                                	{    
                                       		printf("%-*s", cp->lsp[c] + 1, dp->fp[i].name);
                                	}    
                                	if(S_ISLNK(dp->fp[i].mode))
                                	{    
                                        	printf("%-*s", cp->lsp[c] + 1, dp->fp[i].name);
                                	}    
                                	if(S_ISSOCK(dp->fp[i].mode))
                                	{    
                                       		printf("%-*s", cp->lsp[c] + 1, dp->fp[i].name);
                                	}    
                                	if(S_ISFIFO(dp->fp[i].mode))
                                	{    
                                        	printf("%-*s", cp->lsp[c] + 1, dp->fp[i].name);
                                	}    
                                	if(S_ISREG(dp->fp[i].mode))
                                	{    
                                        	if(access(dp->fp[i].name,  X_OK) == 0)
                                        	{    
                                                	printf("%-*s", cp->lsp[c] + 1, dp->fp[i].name);
                                        	}    
                                        	else 
                                        	{    
                                                	printf("%-*s", cp->lsp[c] + 1, dp->fp[i].name);
                                        	}     
                                	}    
					i  = i + cp->rnt;
					if(i >= dp->cnt)
					{
						break;
					}
				}
				putchar('\n');
                        }    
		}
	}
        freecol(cp);
	return 0;
}

//print each file per line
static int32_t printone(mc_Dir *dp)
{
        int32_t i;
        if(dp == NULL || dp->fp == NULL)
        {
                return 1;
        }
        if(dp->cnt == 0)
        {
		printf("total 0\n");
                return 0;
        }
        for(i = 0; i < dp->cnt; i++)
        {
                if(iflg)
                {
                        printf("%-*ld", dp->iwid + 2, dp->fp[i].ino);
                }
                printf("%s\n", dp->fp[i].name);
        }
        if(Rflg == 0)
        {
                return 0;
        }
        for(i = 0; i < dp->cnt; i++)
        {
                if(strcmp(dp->fp[i].name, ".") == 0 || strcmp(dp->fp[i].name, "..") == 0)//infinite
                {
                        continue;
                }
                if(S_ISDIR(dp->fp[i].mode))
                {
                        printf("%s\n", dp->fp[i].name);
                        printone(dp->fp[i].dp);
                }
        }
        return 0;
}

//using the divcolDefault function print 
static int32_t printcolDown(mc_Dir *dp)
{
	int32_t i,r,c;
	mc_Col *cp;

	if(iflg)
	{
		return printone(dp);
	}
	if(Fflg)
	{
		return printSpecial(dp);
	}
	if(sflg)
	{
		return printBlock(dp);
	}
	if(dp == NULL || dp->fp == NULL)
	{
		return 1;
	}
	if(dp->cnt == 0)
    {
		printf("total 0\n");
		return 0;
	}
	if((cp = divcolDefault(dp)) == NULL)
	{
		return 1;
	}
	if(oneflg)
	{
		for(i = 0; i < dp->cnt; i++)
		{
			 printf("%-*s\n", cp->lsp[i % cp->cnt], dp->fp[i].name);
		}
	}
	else
	{	
	//	i = 0;
		for(r = 0; r < cp->rnt; r++)
		{
			i = r;
			for(c = 0; c < cp->cnt; c++)
			{
				
				printf("%-*s ", cp->lsp[c], dp->fp[i].name);
//				if((i == 0 && dp->cnt > 1) || (i + 1) % cp->rnt)
//				{
//					putchar(' ');
//				}
				i  = i + cp->rnt;
				if(i >= dp->cnt)
				{
					break;
				}
			}
			putchar('\n');
			
		}
	}
	freecol(cp);
	if(Rflg == 0)
	{
		return 0;
	}  
        for(i = 0; i < dp->cnt; i++)//-R
	{ 
                if(strcmp(dp->fp[i].name, ".") == 0 || strcmp(dp->fp[i].name, "..") == 0)  
		{
	               continue;  
		}                
		if(S_ISDIR(dp->fp[i].mode))
		{  
                        printf("%s:\n", dp->fp[i].name);  
                        printcolDown(dp->fp[i].dp);  
                }  
        }  
        return 0; 	
}
//print across
static int32_t printcol(mc_Dir *dp)
{
	int32_t i;
	mc_Col *cp;

	if(iflg)
	{
		return printone(dp);
	}
	if(Fflg)
	{
		return printSpecial(dp);
	}
	if(sflg)
	{
		return printBlock(dp);
	}
	if(dp == NULL || dp->fp == NULL)
	{
		return 1;
	}
	if(dp->cnt == 0)
	{
		printf("total 0\n");
		return 0;
	}
	if((cp = divcol(dp)) == NULL)
	{
		return 1;
	}
	if(oneflg)
	{
		for(i = 0; i < dp->cnt; i++)
		{
			 printf("%-*s\n", cp->lsp[i % cp->cnt], dp->fp[i].name);
		}
	}
	else
	{
		for(i = 0; i < dp->cnt; i++)
		{
			printf("%-*s", cp->lsp[i % cp->cnt], dp->fp[i].name);
			if((i == 0 && dp->cnt > 1) || (i + 1) % cp->cnt)
			{
				putchar(' ');
			}
			else
			{
				putchar('\n');
			}
		}
		if(dp->cnt % cp->cnt)
		{
			putchar('\n');
		}
	}
	freecol(cp);
	if(Rflg == 0)
	{
		return 0;
	}  
        for(i = 0; i < dp->cnt; i++)//-R
	{ 
                if(strcmp(dp->fp[i].name, ".") == 0 || strcmp(dp->fp[i].name, "..") == 0)  
		{
	               continue;  
		}                
		if(S_ISDIR(dp->fp[i].mode))
		{  
                        printf("%s:\n", dp->fp[i].name);  
                        printcol(dp->fp[i].dp);  
                }  
        }  
        return 0; 	
}
//compare ctime function
static int32_t cmpctime(const void *s1, const void *s2)
{
	return ((mc_File *)s2)->ctime - ((mc_File *)s1)->ctime;
}
//compare ctime function plus -r
static int32_t cmpctimeReverse(const void *s1, const void *s2)
{
	return ((mc_File *)s1)->ctime - ((mc_File *)s2)->ctime;
}
//compare mtime function 
static int32_t cmpmtime(const void *s1, const void *s2)
{
	return ((mc_File *)s2)->mtime - ((mc_File *)s1)->mtime;
}
//compare mtime function plus -r
static int32_t cmpmtimeReverse(const void *s1, const void *s2)
{
	return ((mc_File *)s1)->mtime - ((mc_File *)s2)->mtime;
}
//compare atime funciton
static int32_t cmpatime(const void *s1, const void *s2)
{
	return ((mc_File *)s2)->atime - ((mc_File *)s1)->atime;
}
//compare atime function plus -r
static int32_t cmpatimeReverse(const void *s1, const void *s2)
{
	return ((mc_File *)s1)->atime - ((mc_File *)s2)->atime;
}
//the format ls
static void modetostr(int32_t mode, char *buf)
{
	strcpy(buf, "----------"); 
	if(S_ISWHT(mode))
	{
		buf[0] = 'w';
	}
	if(((mode) & _S_ARCH1) == _S_ARCH1)
	{
		buf[0] = 'a';
	}
	if(((mode) & _S_ARCH2) == _S_ARCH2)
	{
		buf[0] = 'A';
	}
	if(S_ISSOCK(mode))
	{
		buf[0] = 's';
	}
   	if(S_ISDIR(mode))  
        {
		buf[0] = 'd';  
	}
    	if(S_ISCHR(mode))  
       	{
	 	buf[0] = 'c';  
	}
    	if(S_ISBLK(mode))  
	{
        	buf[0] = 'b';  
	}
    	if(S_ISLNK(mode))  
	{
	        buf[0] = 'l';  
	}
   	if(S_ISFIFO(mode))  
	{
        	buf[0] = 'p';  
	}
    	if(mode & S_IRUSR)  
	{
        	buf[1] = 'r';  
	}
    	if(mode & S_IWUSR)  
	{
        	buf[2] = 'w';  
	}
    	if(mode & S_IXUSR)  
	{
        	buf[3] = 'x';  
	}
    	if((mode & S_ISUID) && (mode & S_IXUSR))  
	{
        	buf[3] = 's';  
	}
	if((mode & S_ISUID) && (!(mode & S_IXUSR)))
	{
		buf[3] = 'S';
	}
    	if(mode & S_IRGRP)  
	{
        	buf[4] = 'r';  
	}
    	if(mode & S_IWGRP)  
	{
        	buf[5] = 'w';  
	}
    	if(mode & S_IXGRP)  
	{
        	buf[6] = 'x';  
	}
    	if((mode & S_ISGID) && (mode & S_IXGRP))  
	{
        	buf[6] = 's';  
	}
	if((mode & S_ISGID) && (!(mode & S_IXGRP)))
	{
		buf[6] = 'S';
	}
    	if(mode & S_IROTH)  
	{
        	buf[7] = 'r';  
	}
    	if(mode & S_IWOTH)  
	{
        	buf[8] = 'w';  
	}
    	if(mode & S_IXOTH) 
	{
        	buf[9] = 'x';  
	}
    	if((mode & S_ISVTX) && (mode & S_IXOTH))  
	{
        	buf[9] = 't';
	}
	if((mode & S_ISVTX) && (!(mode & S_IXOTH)))
	{
		buf[9] = 'T';	
	}
}
//print time format
static void printtime(int32_t t)
{
        char buf[BUFSIZ];
        struct tm   *tp;
	//char *p;
//	char p2[BUFSIZ];
    
        if((tp = localtime((time_t *)&t)) == NULL)
        {   
                snprintf(buf, BUFSIZ, "Wrong time");
        }   
        else
        {   
		strftime(buf, BUFSIZ, "%b %d %H:%M", tp);
                //snprintf(buf, BUFSIZ, "%s\t%d  %d:%d", p, tp->tm_mday, tp->tm_hour, tp->tm_min);
        }   
        printf("%-18s ", buf);
}
//-l
static int32_t printlong(mc_Dir *dp)
{
	int32_t i, j, rst;
	char buf[10];
	char tmp[BUFSIZ];
	int64_t total;
	char *p;
	struct group *sp;
	struct passwd *pd;
	char p2[MAXNewArray + 1];
	char buf2[MAXPATHLEN];
	char *buf3;
//	char *buf4;
	int fd;

	if(dp == NULL || dp->fp == NULL)
	{
		return 1;
	}
	if(dp->cnt == 0)
	{  
		printf("total 0\n");
        	return 0;  
    	}
	if(nflg)
	{
		for(i = 0; i < dp->cnt; i++)
		{
			modetostr(dp->fp[i].mode, buf);
			if(sflg == 0 && iflg == 1)
			{
				printf("%-*ld ", dp->iwid, dp->fp[i].ino);
			}
			for(j = 0; j < dp->cnt; j++)
			{
				total += (dp->fp[j].blocks);
			}
			if(i == 0)
			{
				if(sflg != 1)
				{
					printf("total %ld \n", total);
				}
			}
			if(sflg)
			{
				for(j = 0; j < dp->cnt; j++)
				{
					total += (dp->fp[j].size);		
				}
				if(i == 0)
				{
					if(hflg)
					{
						sprintf(p2, "%ld", total);
						p = strcat(p2, "B");
						printf("total %s \n", p);
					}
					else if(kflg)
					{
						sprintf(p2, "%ld", (total/1024));
						p = strcat(p2, "K");
						printf("total %s \n", p);
					}
					else
					{		
						printf("total %ld\n", total);
					}
				}
				if(iflg)
				{
					printf("%-*ld ", dp->iwid, dp->fp[i].ino);
				}
				if(hflg)
				{
					dp->fp[i].blocks = (dp->fp[i].blocks)*512;
					sprintf(p2, "%ld", dp->fp[i].blocks);
					p = strcat(p2, "B");
					printf("%-*s %s %-*u %-*d %-*d ",dp->swid + 3, p, buf, dp->lwid + 1, dp->fp[i].links, dp->uwid, dp->fp[i].uid, dp->gwid, dp->fp[i].gid);
				}
				else if(kflg)
				{
					dp->fp[i].blocks = (dp->fp[i].blocks) / 2;
					sprintf(p2, "%ld", dp->fp[i].blocks);
					p = strcat(p2, "K");
					printf("%-*s %s %-*u %-*d %-*d ",dp->swid, p, buf, dp->lwid + 1, dp->fp[i].links, dp->uwid, dp->fp[i].uid, dp->gwid, dp->fp[i].gid);
				}
				else
				{
					printf("%-*ld %s %-*u %-*d %-*d ",dp->swid, dp->fp[i].blocks, buf, dp->lwid + 1, dp->fp[i].links, dp->uwid, dp->fp[i].uid, dp->gwid, dp->fp[i].gid);		
				}
			}
			else
			{
				printf("%s %-*u %-*d %-*d ", buf, dp->lwid + 1, dp->fp[i].links, dp->uwid, dp->fp[i].uid, dp->gwid, dp->fp[i].gid);
			}
			if((S_ISCHR(dp->fp[i].mode)) || (S_ISBLK(dp->fp[i].mode)))
			{
				snprintf(tmp, BUFSIZ, "%d,%d ", major(dp->fp[i].dev), minor(dp->fp[i].dev));
				printf("%-*s ", dp->swid, tmp);
			}
			else
			{
				if(hflg)
				{
					sprintf(p2, "%ld", dp->fp[i].size);
                			p = strcat(p2, "B");
					printf("%-*s ", dp->swid + 2, p);
				}
				else if(kflg)
				{
					sprintf(p2, "%ld", ((dp->fp[i].size) / 1024));
					p = strcat(p2, "K");
					printf("%-*s ", dp->swid, p);
				}
				else
				{
					printf("%-*ld ", dp->swid, dp->fp[i].size);
				}
			}	
			if(cflg)
			{
				printtime(dp->fp[i].ctime);
			}
			else if(uflg)
			{
				printtime(dp->fp[i].atime);
			}
			else
			{
				printtime(dp->fp[i].mtime);
			}
			if(Rflg)
			{
			if(S_ISLNK(dp->fp[i].mode))
			{
				if((fd = open(pathnameDir, O_DIRECTORY)) == -1)
				{
					continue;
				}
				else
				{
					fd = open(pathnameDir, O_DIRECTORY);
					rst = readlinkat(fd,dp->fp[i].name, buf2, MAXPATHLEN);//direct to the link file itself
					if(rst == -1)
					{	
						perror("readlink error:\n");
						return 1;
					}
					buf3 = strcat(dp->fp[i].name, " -> ");
					buf3 = strcat(buf3, buf2);
					printf("%s \n", buf3);
				}
			}
			else
			{
				printf("%s \n", dp->fp[i].name);
			}
			}
		}
		if(Rflg == 0)
		{
			return 0;
		}
		for(i = 0; i < dp->cnt; i++)
		{
			if(strcmp(dp->fp[i].name, ".") == 0 || strcmp(dp->fp[i].name, "..") == 0)
			{
				continue;
			}
			if(S_ISDIR(dp->fp[i].mode))
			{
				printf("%s:\n", dp->fp[i].name);
				printlong(dp->fp[i].dp);
			}
		}
	}
	else
	{
		for(i = 0; i < dp->cnt; i++)
		{
			if((sp = getgrgid(dp->fp[i].gid)) == NULL)
			{
				printf("gid error:\n");
				return 1;
			}
			if((pd = getpwuid(dp->fp[i].uid)) == NULL)
			{
				printf("uid error:\n");
				return 1;
			}
			if(sflg == 0 && iflg == 1)
			{
				printf("%-*ld ", dp->iwid, dp->fp[i].ino);
			}
			for(j = 0; j < dp->cnt; j++) 
                        {    
                                total += (dp->fp[j].blocks);
                        }    
                        if(i == 0)
                        {    
                                if(sflg != 1)
                                {    
                                        printf("total %ld \n", total);
                                }    
                        }
			modetostr(dp->fp[i].mode, buf);
			if(sflg)
			{
				for(j = 0; j < dp->cnt; j++)
				{
					total += (dp->fp[j].size);		
				}
				if(i == 0)
				{
					if(hflg)
					{
						sprintf(p2, "%ld", total);
						p = strcat(p2, "B");
						printf("total %s \n", p);
					}
					else if(kflg)
					{
						sprintf(p2, "%ld", (total/1024));
						p = strcat(p2, "K");
						printf("total %s \n", p);
					}
					else
					{		
						printf("total %ld\n", total);
					}
				}
				if(iflg)
				{
					printf("%-*ld ", dp->iwid, dp->fp[i].ino);
				}
				if(hflg)
				{
					dp->fp[i].blocks = (dp->fp[i].blocks)*512;
					sprintf(p2, "%ld", dp->fp[i].blocks);
					p = strcat(p2, "B");
					printf("%-*s %s %-*u %-*s %-*s ",dp->swid + 3, p, buf, dp->lwid + 1, dp->fp[i].links, dp->uwid + 5, pd->pw_name, dp->gwid + 5, sp->gr_name);
				}
				else if(kflg)
				{
					dp->fp[i].blocks = (dp->fp[i].blocks) / 2;
					sprintf(p2, "%ld", dp->fp[i].blocks);
					p = strcat(p2, "K");
					printf("%-*s %s %-*u %-*s %-*s ",dp->swid, p, buf, dp->lwid + 1, dp->fp[i].links, dp->uwid + 5, pd->pw_name, dp->gwid + 5, sp->gr_name);
				}
				else
				{
					printf("%-*ld %s %-*u %-*s %-*s ",dp->swid, dp->fp[i].blocks, buf, dp->lwid + 1, dp->fp[i].links, dp->uwid + 5,  pd->pw_name, dp->gwid + 5, sp->gr_name);		
				}
			}
			else
			{
				printf("%s %-*u %-*s %-*s ", buf, dp->lwid + 1, dp->fp[i].links, dp->uwid + 5, pd->pw_name, dp->gwid + 5, sp->gr_name);
			}
			if((S_ISCHR(dp->fp[i].mode)) || (S_ISBLK(dp->fp[i].mode)))
			{
				snprintf(tmp, BUFSIZ, "%d,%d ", major(dp->fp[i].dev), minor(dp->fp[i].dev));
				printf("%-*s ", dp->swid, tmp);
			}
			else
			{
				if(hflg)
				{
					sprintf(p2, "%ld", dp->fp[i].size);
                			p = strcat(p2, "B");
					printf("%-*s ", dp->swid + 2, p);
				}
				else if(kflg)
				{
					sprintf(p2, "%ld", ((dp->fp[i].size) / 1024));
					p = strcat(p2, "K");
					printf("%-*s ", dp->swid, p);
				}
				else
				{
					printf("%-*ld ", dp->swid, dp->fp[i].size);
				}
			}
			if(cflg)
			{
				printtime(dp->fp[i].ctime);
			}
			else if(uflg)
			{
				printtime(dp->fp[i].atime);
			}
			else
			{
				printtime(dp->fp[i].mtime);
			}
			if(Rflg == 0)
			{
			if(S_ISLNK(dp->fp[i].mode))
                        {
                                if((fd = open(pathnameDir, O_DIRECTORY)) == -1)
                                {
					continue;
                                }
                                else
                                {
                                        rst = readlinkat(fd,dp->fp[i].name, buf2, MAXPATHLEN);
                                        if(rst == -1)
                                        {     
                                                perror("readlink error:\n");
                                                return 1;
                                        }    
                                        buf3 = strcat(dp->fp[i].name, " -> ");
                                        buf3 = strcat(buf3, buf2);
                                        printf("%s \n", buf3);
                                }    
                        }    
			
                        else 
                        {    
                                printf("%s \n", dp->fp[i].name);
                        }
			}
		}
		if(Rflg == 0)
		{
			return 0;
		}
		for(i = 0; i < dp->cnt; i++)
		{
			if(strcmp(dp->fp[i].name, ".") == 0 || strcmp(dp->fp[i].name, "..") == 0)
			{
				continue;
			}
			if(S_ISDIR(dp->fp[i].mode))
			{
				printf("%s:\n", dp->fp[i].name);
				printlong(dp->fp[i].dp);
			}
		}
	}
	return 0;
}

/*
release all information concluded in the directory
success 0
fail 1
*/
static int FreeDirs(mc_Dir *dp)
{
        int32_t i;
        if(dp == NULL || dp->fp == NULL)
        {
                return 1;
        }
        for(i = 0; i < dp->cnt; i++)
        {
                if(dp->fp[i].dp)
                {
                        FreeDirs(dp->fp[i].dp);
                }
        }
        free(dp->fp);
        free(dp);
        return 0;
}

//got the actually length
static int64_t digitlen(int64_t n)
{
        char buf[BUFSIZ];
        snprintf(buf, BUFSIZ, "%ld", n);
        return strlen(buf);
}

static void copyinfo(mc_File *fp, const struct stat *sp,const char *pathname)
{       
        fp->dp = NULL;
        fp->ino = sp->st_ino;
        fp->uid = sp->st_uid;
        fp->gid = sp->st_gid;
        fp->dev = sp->st_rdev;
        fp->mode = sp->st_mode;
        fp->size = sp->st_size;
        fp->atime = sp->st_atime;
        fp->mtime = sp->st_mtime;
        fp->ctime = sp->st_ctime;
        fp->links = sp->st_nlink;
	fp->blockSize = sp->st_blksize;
	fp->blocks = sp->st_blocks;
        fp->wid = strlen(pathname);
        strcpy(fp->name, pathname);
}

/*
read all files into the file struct
success: return the struct
fail: return NULL
*/
static mc_Dir* OpenDirs(const char *pathname)
{
	int32_t w;
	DIR *dp;
	mc_Dir  *rtv;
	mc_File *tmp;
	struct stat stbuf;
	struct dirent   *dirp;
	char    home[MAXPATHLEN + 1];
	
	if(getcwd(home, MAXPATHLEN + 1) == NULL)//the ./ls's directory
	{
		return NULL; 
	}
	if(lstat(pathname, &stbuf) == -1)
	{
		return NULL;
	}
	if((rtv = malloc(sizeof(mc_Dir))) == NULL)
	{
		return NULL;
	}
	memset(rtv, 0, sizeof(mc_Dir));
	if((rtv->fp = malloc(sizeof(mc_File))) == NULL)
	{
		free(rtv);
		return NULL;
	}
//	if(S_ISDIR(stbuf.st_mode))
//	{
		//rtv->name = pathname;	
//		strcpy(rtv->name, pathname);
//	}
	if(!S_ISDIR(stbuf.st_mode))//if it's not a directory, then get the file information directly
	{
		rtv->cnt = 1;
		copyinfo(rtv->fp, &stbuf, pathname);
		return rtv;
	}		
	if(chdir(pathname) == -1)
	{
		free(rtv->fp);
		free(rtv);
		return NULL;
	}
	if((dp = opendir(".")) == NULL)
	{
		free(rtv->fp);
		free(rtv);
		chdir(home);
		return NULL;
	}
	for(rtv->cnt = 0; (dirp = readdir(dp)) != NULL; rtv->cnt++)
	{
		if((dirp->d_name[0] == '.') && (Aflg == 0) && (aflg == 0))
		{
			rtv->cnt--;
			continue;
		}
		if((strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) && aflg == 0)
		{
			rtv->cnt--;
			continue;
		}
		if((tmp = realloc(rtv->fp, (rtv->cnt + 1) * sizeof(mc_File))) == NULL)
		{
			FreeDirs(rtv);
			chdir(home);	
			closedir(dp);
			return NULL;
		}
		rtv->fp = tmp;
		if(lstat(dirp->d_name, &stbuf) == -1)
		{
			FreeDirs(rtv);
			chdir(home); 
			closedir(dp);
			return NULL;
		}
		copyinfo(rtv->fp + rtv->cnt, &stbuf, dirp->d_name);
		if(iflg && (w = digitlen(rtv->fp[rtv->cnt].ino)) > rtv->iwid)
		{
			rtv->iwid = w; //the max inode length
		}
		if(Printfunc == printlong)
		{
			if((w = digitlen(rtv->fp[rtv->cnt].links)) > rtv->lwid)
			{
				rtv->lwid = w;//got the max link length
			}
			if((w = digitlen(rtv->fp[rtv->cnt].uid)) > rtv->uwid)
			{
				rtv->uwid = w;//max uid length
			}
			if((w = digitlen(rtv->fp[rtv->cnt].gid)) > rtv->gwid)
			{
				rtv->gwid = w;//max git
			}
			if(S_ISCHR(stbuf.st_mode) || S_ISBLK(stbuf.st_mode))
			{
				w = digitlen(major(rtv->fp[rtv->cnt].dev));
				w += digitlen(minor(rtv->fp[rtv->cnt].dev)) + 1;//+1 means the .between major device id and minor device id
				if(w > rtv->swid)
				{
					rtv->swid = w;
				}
			}
			else
			{
				if((w = digitlen(rtv->fp[rtv->cnt].size)) > rtv->swid)
				{
					rtv->swid = w; 
				}
			}
		}
		if(Rflg == 0)
		{
			continue;
		}
		 if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		{
			continue;
		}
		if(S_ISDIR(stbuf.st_mode) && (rtv->fp[rtv->cnt].dp = OpenDirs(dirp->d_name)) == NULL)
		{
			FreeDirs(rtv);
			chdir(home);
			closedir(dp);
			return NULL;
		}
	}
	chdir(home);
	closedir(dp);
	return rtv;
}
//useing the company function to sort the diretory. 
static int SortDirs(mc_Dir *dp, int (*cmp)(const void *, const void *))
{
	int32_t i;
	
	if(dp == NULL || dp->fp == NULL)
	{
		return 1;	
	}
	for(i = 0; i < dp->cnt; i++)
	{
		if(dp->fp[i].dp)//include the subdirectory
		{
			SortDirs(dp->fp[i].dp, cmp);
		}
	}
	qsort(dp->fp, dp->cnt, sizeof(mc_File), cmp);
	return 0;
}

int main(int argc, char *argv[])
{
        mc_Dir *dp;
        int i, c;
        struct winsize siz;

        if(argc < 1 || argv == NULL)
        {       
                printf("./ls [−AacCdFfhiklnqRrSstuwx1] [file...]\n");
                return 1;
        }
        if(ioctl(STDOUT_FILENO, TIOCGWINSZ, (char *)&siz) == -1)
        {       
                printf("ioctl error:\n");
                return 1;
        }
        col = siz.ws_col;
        Cmpfunc = cmpname;
        Printfunc = printcolDown;
        Aflg = aflg = Cflg = cflg = dflg = Fflg = fflg = hflg = iflg = kflg = lflg = nflg = qflg = Rflg = rflg = Sflg = sflg = tflg = uflg = wflg = xflg = oneflg = 0;
        for(i = 1; i < argc; i++)
        {
                if(argv[i][0] == '-')
                {       
                        while((c = *++argv[i]))
                        {       
                                switch(c)
                                {
					case 'x':
						xflg = 1;
						Printfunc = printcol;
						break;
					case '1':
						oneflg = 1;
						break;
					case 'C':
						Cflg = 1;
						break;
					case 'S':
						Sflg = 1;
						Cmpfunc = cmpSize;
						break;
					case 'r':
						rflg = 1;
						Cmpfunc = cmpnameReverse;
						break;
					case 'k':
						kflg = 1;
						break;
					case 'h':
						hflg = 1;
						break;
					case 's':
						sflg = 1;
						break;
					case 'f':
						fflg = 1;
						break;
					case 'F':
						Fflg = 1;	
					//	Printfunc = printSpecial;
						break;
					case 'd':
						dflg = 1;
						break;
                                        case 'a':
                                                aflg = 1;
                                                break;
                                        case 'A':
                                                Aflg = 1;
                                                break;
					case 'c':
                                                cflg = 1;
                                                Cmpfunc = cmpctime;
                                                break;
                                        case 'U':
                                                Cmpfunc = NULL;
                                                break;
                                        case 'i':
                                                iflg = 1;
                                                break;
					case 'n':
						nflg = 1;
						Printfunc = printlong;
                                        case 'l':
						lflg = 1;
                                                Printfunc = printlong;
                                                break;
                                        case 'L':
                                                Lflg = 1;
                                                break;
                                        case 't':
						tflg = 1;
                                                Cmpfunc = cmpmtime;
                                                break;
                                        case 'u':
                                                uflg = 1;
                                                cflg = 0;
                                                Cmpfunc = cmpatime;
                                                break;
                                        case 'R':
                                                Rflg = 1;
                                                break;
                                        default:
						printf("./ls invalid option\n");
                                               // break;
						return 1;
                                }
                        }
                }
                else
                {
                        break;
		}
        }
	pathnameDir = (char *)malloc(8*MAXPATHLEN);
        c = i < argc - 1 ? 1 : 0;//c set to 1 means print the each output directory
        if(i == argc)//No any other file or directory as the argument
        {
                if((dp = OpenDirs(".")))
                {
		//	pathnameDir = ".";
			pathnameDir = strcpy(pathnameDir, "./");
			if(dflg)
			{
				printf(".\n");
				FreeDirs(dp);
				return 0;
			}
			if(fflg)
			{
				Printfunc(dp);
				FreeDirs(dp);
				return 0;
			}
                        else if(Cmpfunc)
                        {
				if(Sflg == 1 && rflg == 1)
				{
					Cmpfunc = cmpSizeReverse;
				}
				else if(tflg == 1 && rflg == 1)
				{
					Cmpfunc = cmpmtimeReverse;
				}
				else if(cflg == 1 && rflg == 1)
				{
					Cmpfunc = cmpctimeReverse;
				}
				else if(uflg == 1 && rflg == 1)
				{
					Cmpfunc = cmpatimeReverse;
				}
				if(uflg == 1 && lflg ==1 && tflg == 0)
				{
					Cmpfunc = cmpname;
				}
				if(uflg == 1 && lflg == 1 && tflg ==1)
				{
					Cmpfunc = cmpatime;
				}
                                SortDirs(dp, Cmpfunc);//sort all files in the directory.
                        }
                        Printfunc(dp);
                        FreeDirs(dp);
                        return 0;
                }
                printf("Error: cannot ls '.'\n");
                return 1;
        }
        for( ; i < argc; i++)
        {
		if(dflg)
		{
			if((dp = OpenDirs(argv[i])))
			{
				pathnameDir = argv[i];
				printf("%s\n", argv[i]);
				continue;
			}
			else
			{
				printf("Error: cannot ls '%s'\n", argv[i]);
				continue;
			}
		}
                if((dp = OpenDirs(argv[i])))
                {
			pathnameDir = argv[i];
                        if(c)
                        {
                                printf("%s:\n", argv[i]);
				if(fflg)
				{
					Printfunc(dp);
					FreeDirs(dp);
					printf("\n");
					continue;
				}
                        }
                        if(Cmpfunc)
                        {
				if(Sflg == 1 && rflg == 1)
				{
					Cmpfunc = cmpSizeReverse;
				}
				else if(tflg == 1 && rflg == 1)
				{
					Cmpfunc = cmpmtimeReverse;
				}
				else if(cflg == 1 && rflg == 1)
				{
					Cmpfunc = cmpctimeReverse;
				}
				else if(uflg == 1 && rflg == 1)
				{
					Cmpfunc = cmpatimeReverse;
				}
				if(uflg == 1 && lflg ==1 && tflg == 0)
				{
					Cmpfunc = cmpname;
				}
				if(uflg == 1 && lflg == 1 && tflg ==1)
				{
					Cmpfunc = cmpatime;
				}
                                SortDirs(dp, Cmpfunc);
                        }
                        Printfunc(dp);
                        FreeDirs(dp);
                }
                else
                {
                        printf("Error: cannot ls '%s'\n", argv[i]);
                }
                if(c)
                {
                        putchar('\n');
                }
        }
	free(pathnameDir);
        return 0;
}

