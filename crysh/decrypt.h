#ifndef CRYSH_DECRYPT_
#define CRYSH_DECRYPT_
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE_DECRYPT 512 
#define SALT_LENGTH 8
#define SALT_FLAG 8
#define CIPHER_TYPE "aes-256-cbc" 
#define DGST "sha1"

char * decry(char * out_buf, int * real_bytes_command, int len);

#endif
