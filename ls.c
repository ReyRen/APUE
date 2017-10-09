#include <time.h>
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

typedef struct mc_col mc_Col;//the struct of split the fence
typedef struct mc_dir mc_Dir;//a directory all information structure
typedef struct mc_file mc_File;//the specific information of files

struct mc_col
{
	int32_t cnt;//fence count or column count
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
static int32_t (*Printfunc)(mc_Dir *);
static int32_t (*Cmpfunc)(const void *, const void *);


static int32_t cmpname(const void *s1, const void *s2)
{
	return strcmp(((mc_File *)s1)->name,((mc_File *)s2)->name);
}

/*
split fench
success: return the mc_Col struct
fail: return NULL
*/
static mc_Col* divcol(mc_Dir *dp)
{
	mc_Col *cp;
	mc_File *fp;
	int32_t r, c;//r is row, c is column
	int32_t i, l, x;

	if(dp == NULL || (fp = dp->fp) == NULL)
	{
		return NULL;	
	}
//	for(r = 1; ; r++)
//	{
//		l = 0;//the length of each row
//		c = dp->cnt / r;
//		for(i = 0; i < c; i++)		
//		{
//			w = 0;
//			for(j = i; j < dp->cnt; j += c)//get the length(max) of filename in each column
//			{
//				if((fp[j].wid + 1) > w)
//				{
//					w = fp[j].wid + 1;
//				}
//				x += 1;
//			}
			//l += w + 1;//got the length(max) of row
//			l = x * (w + 1);
//		}
//		if(l < col)
//		{
//			break;//perfect if the max length of row less then the length of terminal
//		}
		//or, increase the row number and loop again.
//	}
	x = 0;//got the max length of the filename
	for(i = 0; i < dp->cnt; i++)
	{
		if(fp[i].wid > x)
		{
			x = fp[i].wid;
		}
	}
	x = x + 3;
	for(r = 1;  ; r++)
	{
		c = dp->cnt / r;
		l = c * x;
		if(l < col)
		{
			break;
		}
	}
	if((cp = malloc(sizeof(mc_Col))) == NULL)
	{
		return NULL;
	}
	cp->cnt = c;
	if((cp->lsp = malloc(c * sizeof(int32_t))) == NULL)
	{
		free(cp);
		return NULL;
	}
	for(i = 0; i < c; i++)
	{
		cp->lsp[i] = x;
	}
//	for(i = 0; i < c; i++)
//	{
//		w = 0;
//		for(j = i; j < dp->cnt; j += c)
//		{
//			if((fp[j].wid) > w)
//			{
//				w = fp[j].wid;
//			}
//		}
//		cp->lsp[i] = w;
//	}
	
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

static int32_t printBlock(mc_Dir *dp)
{
	mc_Col *cp;
	int32_t i, j;
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
	for(i = 0; i < dp->cnt; i++)	
	{		
		for(j = 0; j < dp->cnt; j++)
                {   
                        total += (dp->fp[j].blockSize);    
                }   
                if(i == 0)
                {   
                        printf("total %ld\n", total);
                }
		sprintf(p, "%ld", dp->fp[i].blocks);
		buf = strcat(p, " ");
		buf = strcat(buf, dp->fp[i].name);
		strcpy(dp->fp[i].name,buf);
	}
	if((cp = divcol(dp)) == NULL)
	{	
		return 1;
	}
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
	freecol(cp);
	return 0;
}

static int32_t printSpecial(mc_Dir *dp)
{
	int32_t i;
	mc_Col *cp;
	char *buf;
	
	if(dp == NULL || dp->fp == NULL)
	{
		return 1;
	}
	if(dp->cnt == 0)
	{
		printf("%%\n");
		return 0;
	}
	if((cp = divcol(dp)) == NULL)
	{
            return 1;
        }
        for(i = 0; i < dp->cnt; i++)
        {
		if(S_ISDIR(dp->fp[i].mode))
		{
			buf = strcat(dp->fp[i].name, "/");	
			printf("%-*s", cp->lsp[i % cp->cnt], buf);
		//	putchar('/');
		}
		if(S_ISLNK(dp->fp[i].mode))
		{
			buf = strcat(dp->fp[i].name, "@");
			printf("%-*s", cp->lsp[i % cp->cnt], buf);
		}
		if(S_ISSOCK(dp->fp[i].mode))
		{
			buf = strcat(dp->fp[i].name, "=");
			printf("%-*s", cp->lsp[i % cp->cnt], buf);
		}
		if(S_ISFIFO(dp->fp[i].mode))
		{
			buf = strcat(dp->fp[i].name, "|");
			printf("%-*s", cp->lsp[i % cp->cnt], buf);
		}
		if(S_ISREG(dp->fp[i].mode))
                {   
                        if(access(dp->fp[i].name,  X_OK) == 0)
                        {   
				buf = strcat(dp->fp[i].name, "*");
				printf("%-*s", cp->lsp[i % cp->cnt], buf);
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
        freecol(cp);
	return 0;
}

static int32_t printone(mc_Dir *dp)
{
        int32_t i;
        if(dp == NULL || dp->fp == NULL)
        {
                return 1;
        }
        if(dp->cnt == 0)
        {
                putchar('\n');
                return 0;
        }
        for(i = 0; i < dp->cnt; i++)
        {
                if(iflg)
                {
                        printf("%-*ld", dp->iwid, dp->fp[i].ino);
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

static int32_t printcol(mc_Dir *dp)
{
	int32_t i;
	mc_Col *cp;

	if(iflg)
	{
		return printone(dp);
	}
	if(Fflg || fflg)
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
		putchar('\n');
		return 0;
	}
	if((cp = divcol(dp)) == NULL)
	{
		return 1;
	}
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

static int32_t cmpctime(const void *s1, const void *s2)
{
	return ((mc_File *)s1)->ctime - ((mc_File *)s2)->ctime;
}

static int32_t cmpmtime(const void *s1, const void *s2)
{
	return ((mc_File *)s1)->mtime - ((mc_File *)s2)->mtime;
}

static int32_t cmpatime(const void *s1, const void *s2)
{
	return ((mc_File *)s1)->atime - ((mc_File *)s2)->atime;
}

static void modetostr(int32_t mode, char *buf)
{
	strcpy(buf, "----------");  
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
    	if(mode & S_ISUID)  
	{
        	buf[3] = 's';  
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
    	if(mode & S_ISGID)  
	{
        	buf[6] = 's';  
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
    	if(mode & S_ISVTX)  
	{
        	buf[9] = 't';
	}
}

static void printtime(int32_t t)
{
        char buf[BUFSIZ];
        struct tm   *tp;
    
        if((tp = gmtime((time_t *)&t)) == NULL)
        {   
                snprintf(buf, BUFSIZ, "Wrong time");
        }   
        else
        {   
                snprintf(buf, BUFSIZ, "%d.%d.%d %d:%d:%d", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
        }   
        printf("%-18s ", buf);
}

static int32_t printlong(mc_Dir *dp)
{
	int32_t i, j;
	char buf[10];
	char tmp[BUFSIZ];
	int64_t total;

	if(dp == NULL || dp->fp == NULL)
	{
		return 1;
	}
	if(dp->cnt == 0)
	{  
       		putchar('\n');  
        	return 0;  
    	} 
	for(i = 0; i < dp->cnt; i++)
	{
		modetostr(dp->fp[i].mode, buf);
		if(iflg)
		{
			printf("%-*ld ", dp->iwid, dp->fp[i].ino);
		}
		if(sflg)
		{
			for(j = 0; j < dp->cnt; j++)
			{
				total += (dp->fp[j].blockSize);		
			}
			if(i == 0)
			{
				printf("total %ld\n", total);
			}
			printf("%-*ld %s %-*u %-*d %-*d ",dp->swid, dp->fp[i].blocks, buf, dp->lwid, dp->fp[i].links, dp->uwid, dp->fp[i].uid, dp->gwid, dp->fp[i].gid);		
		}
		else
		{
			printf("%s %-*u %-*d %-*d ", buf, dp->lwid, dp->fp[i].links, dp->uwid, dp->fp[i].uid, dp->gwid, dp->fp[i].gid);
		}
		if((S_ISCHR(dp->fp[i].mode)) || (S_ISBLK(dp->fp[i].mode)))
		{
			snprintf(tmp, BUFSIZ, "%d,%d ", major(dp->fp[i].dev), minor(dp->fp[i].dev));
			printf("%-*s ", dp->swid, tmp);
		}
		else
		{
			printf("%-*ld ", dp->swid, dp->fp[i].size);
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
		printf("%s\n", dp->fp[i].name);
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

static void copyinfo(mc_File *fp, const struct stat *sp, const char *pathname)
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
	for(rtv->cnt = 0; (dirp = readdir(dp)); rtv->cnt++)
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
        Printfunc = printcol;
        Aflg = aflg = Cflg = cflg = dflg = Fflg = fflg = hflg = iflg = kflg = lflg = nflg = qflg = Rflg = rflg = Sflg = sflg = tflg = uflg = wflg = xflg = oneflg = 0;
        for(i = 1; i < argc; i++)
        {
                if(argv[i][0] == '-')
                {       
                        while((c = *++argv[i]))
                        {       
                                switch(c)
                                {
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
                                        case 'l':
                                                Printfunc = printlong;
                                                break;
                                        case 'L':
                                                Lflg = 1;
                                                break;
                                        case 't':
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
                                                break;
                                }
                        }
                }
                else
                {
                        break;
		}
        }
        c = i < argc - 1 ? 1 : 0;//c set to 1 means print the each output directory
        if(i == argc)//No any other file or directory as the argument
        {
                if((dp = OpenDirs(".")))
                {
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
                        if(c)
                        {
                                printf("%s:\n", argv[i]);
                        }
                        if(Cmpfunc)
                        {
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
        return 0;
}

