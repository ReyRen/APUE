#include "decrypt.h"

/*
 *	FUNC DESCRIPTION:
 *		This func is used to decrypt the data which
 *		encrypted. Using aes-256-cbc and sha1 to decrypt
 *		All other decrypted parameter description is on
 *		the man page.  
 *	PARAMETER:
 *		char * out_buf:
 *			the data read from stdin (encrypted)
 *		int * real_bytes_command:
 *			the cbc encrypt and decrypt would plus
 *			data's length to the multiple of 16. This
 *			is not the real bytes. But the real bytes
 *			is necessary. So this is a storage to store
 *			the realbytes 
 *		int len:
 *			len is the length read from stdin.
 *	RETURN VALUE:
 *		err: NULL
 *		success: decrypted data
 *
 * */
char * decry(char * out_buf, int * real_bytes_command, int len)
{
	char handle_buf[BUFSIZE_DECRYPT];
	const char * env_pass;
	const char * salt;
	char * decryptedData;
	unsigned char key[EVP_MAX_KEY_LENGTH];
	unsigned char iv[EVP_MAX_IV_LENGTH];	
	EVP_CIPHER_CTX ctx;
	const EVP_MD *dgst = NULL;
	const EVP_CIPHER *cipher_type;
	int isSuccess;
	int decryptedLength = 0;
	int allocateSize;
	int lastDecryptLength;
		

	bzero(handle_buf, sizeof(handle_buf));	
	bzero(key, sizeof(key));
	bzero(iv, sizeof(iv));


	strncpy(handle_buf, out_buf, SALT_LENGTH + SALT_FLAG);
	salt =(const char *)(handle_buf + SALT_LENGTH);
	OpenSSL_add_all_algorithms();
	cipher_type = EVP_get_cipherbyname(CIPHER_TYPE);
	if(!cipher_type)
	{
		perror("no such cipher");
		return NULL;
	}
	dgst=EVP_get_digestbyname(DGST);
	if(!dgst)
	{
		perror("no such md");
		return NULL;
	}
	env_pass = (const char *)getenv("CRYSH_PASSWORD");
	if(env_pass == NULL)
	{
		env_pass = getpass("input decrypt password:");
		if(env_pass == NULL)
		{
			perror("getpass err");
			return NULL;	
		}	
			
	}
	if(!EVP_BytesToKey(cipher_type, dgst, (unsigned char *)salt, 
				(unsigned char *)env_pass, strlen(env_pass), 
				1, key, iv))
	{
		perror("EVP_BytesToKey err");
		return NULL;
	}
	EVP_CIPHER_CTX_init(&ctx);
	EVP_CIPHER_CTX_set_padding(&ctx, 0);/*padding clos*/
	isSuccess = EVP_DecryptInit_ex(&ctx, cipher_type, NULL, key, iv);
	if(!isSuccess)
	{
		perror("EVP_DecryptInit_ex err");
		EVP_CIPHER_CTX_cleanup(&ctx);
		return NULL;
	}
	allocateSize = (len - SALT_LENGTH - SALT_FLAG) * sizeof(char);
	lastDecryptLength = 0;
	decryptedData = (char *) malloc (allocateSize);
	memset(decryptedData, 0x00, allocateSize);
	isSuccess = EVP_DecryptUpdate(&ctx, 
			(unsigned char *)decryptedData, &decryptedLength, 
			(unsigned char *)(out_buf + SALT_LENGTH + SALT_FLAG), 
			len - SALT_LENGTH - SALT_FLAG);
	if(!isSuccess)
	{
		perror("EVP_DecryptUpdate err");
		EVP_CIPHER_CTX_cleanup(&ctx);
		return NULL;
	}
	isSuccess = EVP_DecryptFinal_ex(&ctx, 
			(unsigned char *)(decryptedData + decryptedLength), 
			&lastDecryptLength);
	if(!isSuccess)
	{
		EVP_CIPHER_CTX_cleanup(&ctx);
		return NULL;
	}
	*real_bytes_command = decryptedLength + lastDecryptLength - 1;	
	EVP_CIPHER_CTX_cleanup(&ctx);
	return decryptedData; 
}
