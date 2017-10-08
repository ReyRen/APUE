#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 1024
#define PATH_LEN 128

int is_same_file(char *input_path,char *output_path){
    struct stat input_path_stat,output_path_stat;
    stat(input_path, &input_path_stat);
    stat(output_path, &output_path_stat);
    return input_path_stat.st_ino == output_path_stat.st_ino;
}
/*
	method: This is a method that print the standard error and the line number.
	arguments:	
		err_string: This is uesd to do the illustrate when using the perror to output the error message.
		line: print out the wrong line number.
*/
void my_err(char *err_string, int line )
{
    fprintf(stderr,"line:%d ",line);
    perror(err_string); 
    exit(1);
}
/*
	method:This function is used to copye the data from the src file to the dest file
	arguments:
		frd: This is the file descriptor of the src file
		fwd: This is the file descriptor of the dest file
		read_len: return the actually length of reading one time.
		write_len: return the actually length of writing one time.
		*p_buf: write from this buf 
		buf: read to this buf
*/
void copy_data(const int frd,const int fwd)
{
    int read_len = 0, write_len = 0;
    unsigned char buf[BUF_SIZE], *p_buf;

    while ( (read_len = read(frd,buf,BUF_SIZE)) ) {
        
        if (-1 == read_len) {
            my_err("Read error", __LINE__);
        }
        else if (read_len > 0) { //write to the dest file
            p_buf = buf;
            while ( (write_len = write(fwd,p_buf,read_len)) ) {
                if(write_len == read_len) {
                    break;
                }
                else if(-1 == write_len) {
                    my_err("Write error", __LINE__);
                }
            }
            if (-1 == write_len) break;
        }
    }
}
/*
	method: This function is the main function
	arguments:
		argc: length of the input
		**argv: point to the specific input
		frd: src's file descriptor
		fwd: dest's file descriptor
		*pSrc: point to the path of the src file
		*pDes:	point to the path of the dest file
		src_st: src file's status struct
		des_st:	dest file's status struct
*/
int main(int argc, char **argv) 
{
    
    int frd, fwd; 
    int len = 0;
    char *pSrc, *pDes; 
    struct stat src_st,des_st;
    
    if (argc < 3) {
        printf("./tcp src dest\n");
        my_err("arguments error ", __LINE__);
    }
    
    frd = open(argv[1],O_RDONLY);
    if (frd == -1) {
        my_err("Can not opne file", __LINE__);
    }

    if (fstat(frd,&src_st) == -1) {
        my_err("stat error",__LINE__);
    }
    /*src dir?*/
    if (S_ISDIR(src_st.st_mode)) {
        my_err("ignore the dir",__LINE__);
    }
    
    pDes = argv[2];
    stat(argv[2],&des_st);
    if (S_ISDIR(des_st.st_mode)) { //if the dest is a dir, use the filename of the src
        
        len = strlen(argv[1]);
        pSrc = argv[1] + (len-1); //point to the src path's final character. 
        /*get the filename of the src path*/
        while (pSrc >= argv[1] && *pSrc != '/') {
            pSrc--;
        }
        pSrc++;//point to the src filename
        
        len = strlen(argv[2]); 
        // . means copy to the cur dir
        if (1 == len && '.' == *(argv[2])) {
            len = 0; 
            pDes = pSrc;//dest point to the src's memory 
        }
        else { //copy to the dir/
            pDes = (char *)malloc(sizeof(char)*PATH_LEN);//#define PATH_LEN = 128
            if (NULL == pDes) {
                my_err("malloc error ", __LINE__);
            }
            
            strcpy(pDes,argv[2]);
        
            if ( *(pDes+(len-1)) != '/' ) { //if the end of the path doesn't have the '/', add
                strcat(pDes,"/");
            }
            strcat(pDes+len,pSrc);
        }
	//all of above is in the heap. Actually have not create the file. 
    }
    if(is_same_file(argv[1], pDes))
   {
    	return 1;
   }
    /* open dest file, the mode same with the src*/ 
    fwd = open(pDes,O_WRONLY | O_CREAT | O_TRUNC,src_st.st_mode);
    if (fwd == -1) {
        my_err("Can not creat file", __LINE__);
    }
    copy_data(frd,fwd);
    if (len > 0 && pDes != NULL)
        free(pDes);
    
    close(frd);
    close(fwd);

	return 0;
}
