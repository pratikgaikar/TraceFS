/*
 * user level program to replay documented operations
 * reads record from file and replays it on the file system
 */

#include <stdio.h>
#include <zlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include "../fs/trfs/record.h"

#define ROOT_PATH  "/usr/src/dir2"

void state_list(void *x); /* function to print current state of fd_list */

/* EXTRA_CREDIT */
#ifdef EXTRA_CREDIT
int generate_checksum(char *buff);
#endif

int fd;
char *first_pathname = NULL;
char *sencond_pathname = NULL;
struct fd_node *fd_list_head = NULL; /* points to head of linked list containing open file descriptors */
struct fd_node *fd_list_tail = NULL; /* points to tail of the linked list */
void *x; /* dummy argument to pass to blank(void *) */

int main(int argc, char *argv[])
{
	int c, err = 0;
	FILE *fp = NULL;
	long  lsize;
	int record_size = 0;
	int bytes_read = 0;
	char *trace_file_path = NULL, *buffer = NULL;
	int record_checksum;
	struct trace_record_file *trace_record_file = NULL;
	bool replayFlag = true;
	bool printFlag = true;
	bool nflag = false;
	bool sflag = false;
	while((c = getopt(argc,argv,"ns"))!=-1){
		switch(c){
			case 'n':
				printFlag=true;
				nflag=true;
				replayFlag=false;
				break;
			case 's':
				printFlag=false;
				sflag=true;
				break;
			default:
				err=1;
				break;
		}
	}

	if(optind != argc -1)
	{
		printf("\nInvalid argument");
		goto out;
	}
	if(sflag&&nflag)
	{
		printf("Both N and S cannot be displayed together\n");
		goto out;
	}
	if (err==1)
	{
		printf("Options other than n or s or no flags can not be used.\n");	
		goto out;	
	}

	/* allocate and validate input path to trace file */
	trace_file_path = (char *) malloc(strlen(argv[argc - 1]) + 1);
	if (trace_file_path == NULL)	{
		printf("\nTrace file path couldn't be malloced\n");
		err = -1;
		goto out;
	}


	trace_file_path = strcpy(trace_file_path, argv[argc - 1]);
	if (trace_file_path == NULL)	{
		printf("\nTrace file path couldn't be copied");
		err = -1;
		goto out;
	}

	err = access(trace_file_path, O_RDONLY);
	if (err < 0)	{
		printf("\nInvalid trace file path");
		err = -1;
		goto out;
	}

	fp = fopen(trace_file_path, "r" );
	if (fp == NULL)	{
		printf("\nCouldn't open file");
		err = -1;
		goto out;
	}

	err = fseek (fp , 0 , SEEK_END);
	if (err != 0)	{
		printf("\nFailed to fseek");
		err = -1;
		goto out;
	}

	/*Get total size of trace file*/
	lsize = ftell (fp);
	if (lsize)
		rewind (fp);
	else	{
		printf("\nZero records found in traced files.\n");
		err = -1;
		goto out;
	}

	first_pathname = (char *)malloc(4096);
	if (first_pathname == NULL)   {
		printf("\nCouldn't allocate first pathname\n");
		err = -1;
		goto out;
	}

	/*Allocate memory for 2nd pathname.*/
	sencond_pathname = (char *)malloc(4096);
	if(sencond_pathname == NULL){
		err = -1;
		goto out;
	}

	/* allocate buffer */
	buffer = (char*) malloc (sizeof(char)*lsize);	
	if (buffer == NULL) { 
		printf("\nMemory error");
		err = -1;
		goto out;
	}

	/* read content of trace file*/
	err = fread(buffer, lsize, 1, fp);
	if (err == 0)	{
		printf("\nfread failed or file is empty");
		err = -1;
		goto out;
	}

	while (bytes_read < lsize - 1)
	{
		/* reading record size */
		memcpy(&record_size, 
				buffer + RECORD_SIZE_OFFSET + bytes_read + CHECKSUM_OFFSET, 2);

		/*Allocating memory to trace_record struct*/
		trace_record_file = (struct trace_record_file *) malloc(record_size);
		if (trace_record_file == NULL)	{
			printf("\ncouldn't allocate trace_record_file");
			err = -1;
			goto out;
		}

		/* reading the entire record based on record size */
		memcpy(trace_record_file, buffer + bytes_read + CHECKSUM_OFFSET,
				record_size);

		/*Copy actual checksum of record from buffer into record_checksum */
		memcpy(&record_checksum, buffer + bytes_read, CHECKSUM_OFFSET);

		/* Generate checksum of record */
#ifdef EXTRA_CREDIT
		int checksum = generate_checksum((char *)trace_record_file);

		printf("\nCOMPUTED CHECKSUM = %d, ACTUAL CHECKSUM = %d\n",
				checksum, record_checksum);


		/* Check for integrity of record*/
		if (checksum != record_checksum && sflag == true)	{
			/* 
			   checksum is checked, abort if s flag is set and
			   checksum doesn't match
			   */
			printf("\nData record is corrupted, checksum doesn't match for record id - %d",trace_record_file->record_no);

			goto out;
		}

#endif

		/* check type of trace_record_file */
		switch(trace_record_file->type)
		{
			case TRFS_READ:
				if(printFlag)
					show_read(trace_record_file);	
				if(replayFlag)
					err = replay_read(trace_record_file);
				break;

			case TRFS_WRITE:
				if(printFlag)
					show_write(trace_record_file);
				if(replayFlag)
					err = replay_write(trace_record_file);
				break;

			case TRFS_UNLINK:
				if(printFlag)
					show_unlink(trace_record_file);
				if(replayFlag)
					err = replay_unlink(trace_record_file);
				break;

			case TRFS_SYMLINK:
				if(printFlag)
					show_symlink(trace_record_file);
				if(replayFlag)
					err = replay_symlink(trace_record_file);
				break;

			case TRFS_HARDLINK:
				if(printFlag)
					show_hardlink(trace_record_file);
				if(replayFlag)
					err = replay_hardlink(trace_record_file);
				break;

			case TRFS_RENAME:
				if(printFlag)
					show_rename(trace_record_file);
				if(replayFlag)
					err = replay_rename(trace_record_file);
				break;

			case TRFS_CREATE:
				if(printFlag)
					show_create(trace_record_file);
				if(replayFlag)
					err = replay_create(trace_record_file);
				break;

			case TRFS_MKDIR:
				if(printFlag)
					show_mkdir(trace_record_file);
				if(replayFlag)
					err = replay_mkdir(trace_record_file);
				break;

			case TRFS_RMDIR:
				if(printFlag)
					show_rmdir(trace_record_file);
				if(replayFlag)
					err = replay_rmdir(trace_record_file);
				break;
			case TRFS_OPEN:
				if(printFlag)
					show_open(trace_record_file);
				if(replayFlag)
					err = replay_open(trace_record_file);
				break;			
			case TRFS_CLOSE:
				if(printFlag)
					show_close(trace_record_file);
				if(replayFlag)
					err = replay_close(trace_record_file);
				break;

			case TRFS_READLINK:
				if(printFlag)
					show_readlink(trace_record_file);
				if(replayFlag)
					err = replay_readlink(trace_record_file);
				break;

		}

		/* If any replay failed and S flag is set, then abort*/	
		if (err < 0 && sflag)	{
			printf("\nAbort !!!! Return value not matched for record %d \n",trace_record_file->record_no);
			goto out;
		}

		blank(x);
		/* note : this is inside the while loop */
		bytes_read = bytes_read + record_size + CHECKSUM_OFFSET;

	}

out:
	/* Clear all open FDs*/
	if (fd_list_head)
		clear_fd_list(fd_list_head);

	/*Clear buffer memory */
	if (buffer)
		free(buffer);

	/*Close trace file*/
	if (fp)
		fclose(fp);

	/*Clear trace_file_path memory */
	if (trace_file_path)
		free(trace_file_path);

	/*Clear first_pathname memory */
	if (first_pathname)
		free(first_pathname);

	/*Clear sencond_pathname memory */
	if (sencond_pathname)
		free(sencond_pathname);

	/*Clear trace_record_file memory */
	if (trace_record_file)
		free(trace_record_file);
	return 0;
}

int replay_read(struct trace_record_file *trace_record_file)
{
	int ret = 0;
	char *buff = NULL;

	buff = (char *)malloc(trace_record_file->return_value);
	if (buff == NULL)     {
		printf("\nNo memory");
		ret = -1;
		goto out;
	}

	first_pathname = get_first_path(trace_record_file);
	if (first_pathname == NULL)	{
		printf("\nGetting first path failed");
		ret = -1;
		goto out;
	}

	printf("\nREPLAYING - READING FROM FILE %s", first_pathname);

	fd = get_fd(trace_record_file);
	if (fd == -1)	{
		ret = -1;
		printf("\nOPEN BITSMAP IS OFF. SKIPPING RECORD");
		goto out;
	}
	if(fd)	{
		ret = read(fd, buff, trace_record_file->return_value);
		if (ret != trace_record_file->return_value)	{
			printf("\tREPLAY READ FAILED");
			ret = -1;
			goto out;
		}
		else
		{
			if(strncmp(buff,trace_record_file->path_name+strlen(trace_record_file->path_name) + 1, trace_record_file->return_value)==0) {
				printf("\tREPLAY READ SUCCESS");
			}

		}		
	}
out:
	if(buff)
		free(buff);

	return ret;
}

int replay_write(struct trace_record_file *trace_record_file)
{
	int err = 0;	
	char *data_written = NULL;

	/*Allocate data with all 0's in memory*/
	data_written = (char *)calloc(trace_record_file->return_value, sizeof(char));

	first_pathname = get_first_path(trace_record_file);
	printf("\nREPLAYING WRITING TO FILE %s",first_pathname);
	if (first_pathname == NULL)   {
		printf("\nGetting first path failed");
		err = -1;
		goto out;
	}

	strncpy(data_written, trace_record_file->path_name+strlen(trace_record_file->path_name) + 1 ,trace_record_file->return_value);

	fd = get_fd(trace_record_file);
	if (fd == -1)	{
		printf("\nOPEN BITSMAP IS OFF.SKIPPING RECORD");
		err = -1;
		goto out;
	}
	if(fd)	{
		err  = write(fd, data_written, trace_record_file->return_value);

		if (err != trace_record_file->return_value)	{
			printf("\tREPLAY WRITE FAILED");
			err = -1;
			return err;
		}
		else
		{
			printf("\tREPLAY WRITE SUCCESS");
		}
	}	
out:
	if(data_written)
		free(data_written);

	return err;
}

int replay_rmdir(struct trace_record_file *trace_record_file)
{
	int ret = 0;
	first_pathname = get_first_path(trace_record_file);
	printf("\nREPLAYING RMDIR %s", first_pathname);
	ret = rmdir(first_pathname);
	if (ret < 0)	{
		perror("Error: ");
	}
	else
	{
		printf("\tREPLAY RMDIR SUCESS.");
	}
	return ret == trace_record_file->return_value ? 0 : -1;
}

int replay_mkdir(struct trace_record_file *trace_record_file)
{
	int ret = 0;
	first_pathname = get_first_path(trace_record_file);
	printf("\nREPLAYING MKDIR %s", first_pathname);
	ret = mkdir(first_pathname,trace_record_file->permission_mode);
	if (ret < 0)	{
		perror("Error: ");
	}
	else
	{
		printf("\tREPLAY MKDIR SUCCESS.");
	}
	return ret == trace_record_file->return_value ? 0 : -1;
}

int replay_unlink(struct  trace_record_file  *trace_record_file)
{
	int ret = 0;
	first_pathname = get_first_path(trace_record_file);

	printf("\nREPLAYING UNLINK %s", trace_record_file->path_name);

	ret = unlink(first_pathname);
	if (ret < 0)    {
		perror("Error: ");
	}
	else
	{
		printf("\tREPLAYING UNLINK SUCCESS.");
	}
	return ret == trace_record_file->return_value ? 0 : -1;
}

int replay_hardlink(struct trace_record_file  *trace_record_file)
{
	int ret = 0;

	first_pathname = get_first_path(trace_record_file);
	sencond_pathname = get_second_path(trace_record_file);
	printf("\nREPLAYING HARDLINK %s  %s",first_pathname, sencond_pathname);
	ret  = link(first_pathname, sencond_pathname);
	if (ret < 0)	{
		perror("\t");
	}
	else
	{
		printf("\tREPLAY LINK SUCEESS");
	}

	return ret == trace_record_file->return_value ? 0 : -1;
}

int replay_symlink(struct trace_record_file  *trace_record_file)
{
	int ret = 0, sym_len = 0;
	char *sym_sec_path = NULL;

	char *root_path = NULL;

	root_path = (char *)malloc(strlen(ROOT_PATH)+1);
	strcpy(root_path,ROOT_PATH);
	strcat(root_path,"/");

	printf("\nREPLAYING Symlink %s %s",
			trace_record_file->path_name + 
			strlen(trace_record_file->path_name) + 1, 
			trace_record_file->path_name);

	first_pathname = get_first_path(trace_record_file);

	sym_len = strlen(root_path) + strlen( trace_record_file->path_name+strlen(trace_record_file->path_name) + 1);
	sym_sec_path = (char*)malloc(sizeof(char)* sym_len);
	strncpy(sym_sec_path, root_path, strlen(root_path));	
	strcpy(sym_sec_path + strlen(root_path) ,trace_record_file->path_name+strlen(trace_record_file->path_name) + 1);	

	ret = symlink(sym_sec_path, first_pathname);
	if (ret < 0)	{
		perror("\t");
		goto out;
	}
	else
	{
		printf("\tSYMLINK SUCEESS");
	}

out:
	if(sym_sec_path)
		free(sym_sec_path);
	return ret == trace_record_file->return_value ? 0 : -1;
}

int replay_rename(struct trace_record_file  *trace_record_file){

	int ret = 0;
	first_pathname = get_first_path(trace_record_file);
	if (first_pathname == NULL)	{
		printf("\nGetting first path failed");
		ret = -1;
		goto out;
	}

	sencond_pathname = get_second_path(trace_record_file);
	if (sencond_pathname == NULL)	{
		printf("\nGetting second path failed");
		ret = -1;
		goto out;
	}

	printf("\nREPLAYING RENAME %s to %s", first_pathname, sencond_pathname);

	ret = rename(first_pathname, sencond_pathname);
	if (ret < 0)	{
		perror("\t");
	}
	else
	{
		printf("\tRENAME SUCEESS");
	}
out:
	return ret == trace_record_file->return_value ? 0 : -1;
}

int replay_create(struct trace_record_file  *trace_record_file)
{	 
	int ret = 0;
	first_pathname = get_first_path(trace_record_file);
	printf("\nREPLAYING CREATE %s", first_pathname);
	fd = creat(first_pathname,trace_record_file->permission_mode);
	if (fd < 0)	{
		perror("");
		ret = -1;
	}

	if ((ret>=0 && trace_record_file->return_value>=0) || (ret == trace_record_file->return_value)) {
		printf("\tREPLAY CREATE SUCCESS");
		return 0;
	}

	return ret;	
}

int replay_open(struct trace_record_file  *trace_record_file)
{	
	int err = 0;
	first_pathname = get_first_path(trace_record_file);
	printf("\nREPLAYING OPENING FILE %s", first_pathname);
	fd = open(first_pathname, trace_record_file->flags,trace_record_file->permission_mode);
	if (fd < 0)     {
		perror("");
		err = -1;
		goto out;
	}
	err = add_fd_node(fd, trace_record_file);
out:
	if((fd > -1 && trace_record_file->return_value == 0) || (fd == trace_record_file->return_value)) 
	{
		printf("\tREPLAY OPEN SUCCESS");
		return 0;
	}
	
	return err;
}

int replay_close(struct trace_record_file  *trace_record_file)
{
	int ret = 0;
	first_pathname = get_first_path(trace_record_file);
	printf("\nREPLAYING CLOSE FILE - %s", first_pathname);
	fd = get_fd(trace_record_file); 
	if(fd > 0)	{
		ret = close(fd);
		if (ret < 0)	{
			printf("\t Couldn't close file");
			goto out;
		}
		printf("\tREPLAY CLOSE SUCCESS");
		delete_fd_node(trace_record_file);

	}
out:
	return ret == trace_record_file->return_value ? 0: -1;
}

int replay_readlink(struct trace_record_file *trace_record_file)
{
	int ret = 0;
	char *buf= NULL; /* dynamically allocate later */
	first_pathname = get_first_path(trace_record_file);
	int root_pathlen = strlen(ROOT_PATH) + 1;

	buf = (char *) malloc(trace_record_file->return_value + root_pathlen);
	if (buf == NULL)	{
		printf("\nCouldn't allocate buffer");
		ret = -1;
		goto out;
	}
	printf("\nREPLAYING READLINK - %s\n", first_pathname);
	ret = readlink(first_pathname, buf, trace_record_file->return_value + root_pathlen);
	if (ret < 0)	{
		printf("\nPROBLEM REPLAYING READLINK");
	}
	if (ret !=  trace_record_file->return_value + root_pathlen)     {
		printf("\tREPLAY ReEADLINK FAILED");
		ret = -1;
		goto out;
	}
	else
	{
		printf("\tREPLAY READLINK SUCESS");
	}
out:
	if (buf)
		free(buf);
	return ret;
}

/*WHEN -n flag is set we call below methods */

void show_read(struct trace_record_file * trace_record_file)
{

	first_pathname = get_first_path(trace_record_file);
	printf("\nREADING FROM File - %s",first_pathname); 
	common(trace_record_file);
	extra_params(trace_record_file);
}

void show_write(struct trace_record_file * trace_record_file)
{
	first_pathname = get_first_path(trace_record_file);
	printf("\nWRITING TO FILE - %s", first_pathname);
	common(trace_record_file);
	extra_params(trace_record_file);
}

void show_rmdir(struct trace_record_file * trace_record_file)
{
	first_pathname = get_first_path(trace_record_file);
	printf("\nRMDIR - %s", first_pathname);
	common(trace_record_file);
	extra_perms(trace_record_file);
}

void show_mkdir(struct trace_record_file * trace_record_file)
{
	first_pathname = get_first_path(trace_record_file);
	printf("\nMKDIR - %s", first_pathname);
	common(trace_record_file);
	extra_perms(trace_record_file);
}

void show_unlink(struct trace_record_file * trace_record_file)
{
	first_pathname = get_first_path(trace_record_file);
	printf("\nUNLINK - %s", first_pathname);
	common(trace_record_file);
	extra_perms(trace_record_file);
}

void show_hardlink(struct trace_record_file * trace_record_file)
{
	sencond_pathname = get_second_path(trace_record_file);
	first_pathname = get_first_path(trace_record_file);
	printf("\nHARDLINK %s %s", first_pathname,sencond_pathname);
	common(trace_record_file);
	extra_params(trace_record_file);
}

void show_symlink(struct trace_record_file * trace_record_file)
{
	first_pathname = get_first_path(trace_record_file);
	printf("\nSYMLINK %s  %s", trace_record_file->path_name
			+ strlen(trace_record_file->path_name)
			+ 1, first_pathname );
	common(trace_record_file);
	extra_params(trace_record_file);
}

void show_rename(struct trace_record_file * trace_record_file)
{
	char *second_name = NULL;
	first_pathname = get_first_path(trace_record_file);
	second_name = get_second_path(trace_record_file);
	printf("\nRENAME %s to %s", first_pathname,second_name);
	common(trace_record_file);
	extra_params(trace_record_file);
}

void show_create(struct trace_record_file * trace_record_file)
{
	first_pathname = get_first_path(trace_record_file);
	printf("\nCREATING FILE %s", first_pathname);
	common(trace_record_file);
	extra_perms(trace_record_file);
}

void show_open(struct trace_record_file * trace_record_file)
{
	first_pathname = get_first_path(trace_record_file);
	printf("\nOPENING FILE %s", first_pathname);
	common(trace_record_file);
	extra_perms(trace_record_file);
}

void show_close(struct trace_record_file * trace_record_file)
{
	first_pathname = get_first_path(trace_record_file);
	printf("\nClOSING FILE %s", first_pathname);
	common(trace_record_file);
	extra_perms(trace_record_file);
}

void show_readlink(struct trace_record_file *trace_record_file)
{
	first_pathname = get_first_path(trace_record_file);
	printf("\nREADLINK %s", first_pathname);
	common(trace_record_file);
	extra_params(trace_record_file);
}

#ifdef EXTRA_CREDIT
/* Generate Checksum using crc32 */
int generate_checksum(char *buff)
{	
	return crc32(0^0x00000000, (const void*)buff, strlen(buff));
}
#endif

void blank(void *x)
{
	printf("\n--------------\n");
}

/*
 * function to show common data
 */
void common(struct trace_record_file *trace_record_file)
{
	printf("\nRecord_no: %d", trace_record_file->record_no);
	printf("\nType: %d", trace_record_file->type);
	printf("\nReturn value: %u", trace_record_file->return_value);
	printf("\nPath_name: %s", trace_record_file->path_name);
}

/*
 * function to show permissions and flags
 * works for functions like open
 */
void extra_perms(struct trace_record_file *trace_record_file)
{
	printf("\nFlags: %d", trace_record_file->flags);
	printf("\nPermissions: %d", trace_record_file->permission_mode);
}

/*
 * function to show extra data
 * functions which need two paths or data
 * read, write
 */
void extra_params(struct trace_record_file *trace_record_file)
{
	printf("\nExtra path or data: %s", trace_record_file->path_name + strlen(trace_record_file->path_name)+1);
}

/*
 * function to get complete path of the file to be changed
 */
char *get_first_path(struct trace_record_file *trace_record_file)
{
	char *path = NULL;
	memset(first_pathname,0,4096); //clear memory
	strcpy(first_pathname, ROOT_PATH);

	path = (char *) malloc(4096);
	if (path == NULL)	{
		printf("\ncouldn't allocate path");
		goto free_path;
	}
	strncpy(path, trace_record_file->path_name,
			trace_record_file->path_len);
	memcpy(first_pathname + strlen(ROOT_PATH), path, strlen(path)); //copy file data.
free_path:
	free(path);
	return first_pathname;
}

/*
 * function to obtain the second path required by syscall
 * applicable to functions requiring two paths
 */
char *get_second_path(struct trace_record_file *trace_record_file)
{
	char *path = NULL;
	memset(sencond_pathname,0,4096);

	strcpy(sencond_pathname,ROOT_PATH);

	path = (char *) malloc(strlen(trace_record_file->path_name
				+ strlen(trace_record_file->path_name)
				+ 1));

	if (path == NULL)       
	{
		printf("\ncouldn't allocate path");
		goto out;
	}

	strcpy(path,trace_record_file->path_name
			+ strlen(trace_record_file->path_name)
			+ 1);
	memcpy(sencond_pathname+strlen(ROOT_PATH), path, strlen(path));
out:
	free(path);
	return sencond_pathname;
}

/*
 * function to add new node to linked list of file descriptors
 */
int add_fd_node(int fd, struct trace_record_file *trace_record_file)
{
	struct fd_node *new_node = NULL;
	int err = 0;

	new_node = (struct fd_node *) malloc(sizeof(struct fd_node));
	if (new_node == NULL)   {
		printf("\ncouldn't allocate new node");
		err = -1;
		goto out;
	}

	new_node->fd = fd;
	new_node->file = trace_record_file->file;
	new_node->next = NULL;

	if (fd_list_head == NULL)	{
		fd_list_head = new_node;
		fd_list_tail = new_node;
	}
	else	{
		fd_list_tail->next = new_node;
		fd_list_tail=fd_list_tail->next;
	}


out:
	return err;

}

/*
 * function to delete given node from the linked list of file descriptors
 */
int delete_fd_node(struct trace_record_file *trace_record_file)
{
	struct fd_node *marked = NULL;
	struct fd_node *prev = NULL;

	prev = marked = fd_list_head;

	if (marked->file == trace_record_file->file)	{
		/* found at head */
		fd_list_head = fd_list_head->next;
		marked->next=NULL;
		free(marked);
		return 0;
	}

	while (marked->file != trace_record_file->file )	{
		prev = marked;
		marked = marked->next;
	}

	prev->next = marked->next;
	free(marked);

	return 0;

}


/*
 * function to clear the entire linked list of file descriptors
 */
int clear_fd_list(struct fd_node *head)
{
	struct fd_node *marked = NULL;
	if (fd_list_head == NULL)
		return 0;
	do	{
		marked = fd_list_head;
		fd_list_head = fd_list_head->next;
		if (marked->fd)
			close(marked->fd);
		marked->next = NULL;
		free(marked);

	} while (fd_list_head != NULL);
	return 0;
}

int get_fd(struct trace_record_file *trace_record_file)
{
	struct fd_node *iterator = NULL;
	iterator = fd_list_head;
	while (iterator != NULL)	{
		if (iterator->file == trace_record_file->file)    {
			return iterator->fd;
		}
		iterator = iterator->next;     
	}
	return -1;
}

/* Diaply Status of list. This function is om;y used for debugging purpose. */
void state_list(void *x)
{
	struct fd_node *iterator;
	iterator = fd_list_head;

	if (iterator == NULL)	{
		printf("\niterator is NULL\n");
	}
	printf("\nCurrent state of linked list\n");
	while(iterator != NULL)	{
		printf("%d --> ", iterator->fd);
		iterator = iterator->next;
	}
	blank(x);
}
