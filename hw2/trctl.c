/*
 * user level program to call ioctl
 * pass bitmap to kernel
 * @argc number of arguments
 * @argv array of arguments
 */

/*
 * bitmap definition
 * 0	: MKDIR
 * 1	: RMDIR
 * 2	: UNLINK
 * 3	: HARDLINK
 * 4	: SYMLINK
 * 5	: RENAME
 * 6	: READ
 * 7	: WRITE
 * 8	: OPEN 
 * 9	: CLOSE
 * 10	: READLINK 
 * 11	:
 * 12	:
 * 13	:
 * 14	:
 * 15	:
 * 16	:
 * 17	:
 * 18	:
 * 19	:
 * 20	:
 * 21	:
 * 22	:
 * 23	:
 * 24	:
 * 25	:
 * 26	:
 * 27	:
 * 28	:
 * 29	:
 * 30	:
 * 31	: 
 */


#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <sys/statfs.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

#include "../fs/trfs/trctl.h"
#include "../fs/trfs/record.h"

#ifdef EXTRA_CREDIT
/* 
 * function to parse additional arguments 
 */
int find_syscall_bit(char *syscall_name)	{
	int err = 0;

	if (strcmp(&(syscall_name[1]), "mkdir") == 0)	{
		printf("mkdir\n");
		err = pow(2, TRFS_MKDIR);
	}
	
	else if (strcmp(&(syscall_name[1]), "rmdir") == 0)	{
		printf("rmdir\n");
		err = pow(2, TRFS_RMDIR);

	}

	else if (strcmp(&(syscall_name[1]), "unlink") == 0)	{
		err = pow(2, TRFS_UNLINK);
	}

	else if (strcmp(&(syscall_name[1]), "hardlink") == 0)	{
		err = pow(2, TRFS_HARDLINK);
	}

	else if (strcmp(&(syscall_name[1]), "symlink") == 0)   {
                err = pow(2, TRFS_SYMLINK);
        }

	else if (strcmp(&(syscall_name[1]), "rename") == 0)   {
		err = pow(2, TRFS_RENAME);
	}

	else if (strcmp(&(syscall_name[1]), "read") == 0)   {
                err = pow(2, TRFS_READ);
        }

	else if (strcmp(&(syscall_name[1]), "write") == 0)   {
                err = pow(2, TRFS_WRITE);
        }
	
	else if (strcmp(&(syscall_name[1]), "open") == 0)   {
                err = pow(2, TRFS_OPEN);
        }
	 else if (strcmp(&(syscall_name[1]), "close") == 0)   {
                err = pow(2, TRFS_CLOSE);
        }
	
	else if (strcmp(&syscall_name[1], "readlink") == 0)	{
		err = pow(2, TRFS_READLINK);
	}

	else	{
		printf("undefined system call specified\n");
		err = -1;
		goto out;
	}

out:
	return err;
}

#endif

/*
 * function to convert ascii to integer
 */
int convert_ascii_to_int(char value)
{
	if (value > 0x46)	{
		/*value bigger than F*/
		printf("value is greater than F\n");
		value = -1;
		goto exit_with_err;
	}
	if (value > '9')
		value -= ('A' - '9' - 1);
	value -= 0x30;

exit_with_err:
	//printf("value is: %d\n", value);
	return value;
}

/*
 * function to parse the hexadecimal number and convert to 32 bit bitmap
 */
unsigned long convert_to_bitmap(char *hex)
{
	int bitmap, value;

	bitmap = value = 0;

	while (*(hex++) != 'x');
	/*printf("%s\n",hex);*/
	/*hex points to the number*/
	/*convert it to an integer*/

	while (*hex != '\0') {
		//printf("value of hex is %d", atoi(hex));
		value = convert_ascii_to_int(*hex);
		if (value < 0)	{
			bitmap = -1;
			goto exit_with_err;
		}
		bitmap = bitmap * 16 + value;
		hex++;
	}
exit_with_err:
	return bitmap;
}

/*
 * function to validate path given by user
 */
int validate_path(char *path)
{
	int err;
	
	err = access(path, O_RDWR);
	if (err < 0)	{
		printf("access failed");
		goto exit_with_err;
	}

exit_with_err:
	return err;
}

/*
 * function to validate whether file system mounted
 */
int validate_mount(char *path)
{
	int err;
	struct statfs fs_stats;

	err = statfs(path, &fs_stats);
	if (err < 0)	{
		/* statfs failed */
		printf("statfs failed\n");
		goto exit_with_err;
	} 

	if (fs_stats.f_type)	{
		/* file system is mounted */
		/* printf("file system mounted %x\n", fs_stats.f_type); */
		goto exit_with_err;
	}

	err = -1;

exit_with_err:
	return err;
}

int main(int argc, char** argv)
{	
	/* declaration */
	int err;
	struct bitmap_struct *bitmap_struct = NULL;
	int fd;
	char *path;

	#ifdef EXTRA_CREDIT
	unsigned long syscall_bit;
	#endif

	/* initialization */
	bitmap_struct = (struct bitmap_struct *)malloc(sizeof(struct bitmap_struct));
	if (bitmap_struct == NULL)	{
		printf("couldn't allocate memory to bitmap");
		err = -1;
		goto out;
	}

	//printf("%c\n", argv[optind][0]);
	if (argc == 3 && (argv[optind][0] != '+' &&  argv[optind][0] != '-'))
	{
		/* write bitmap */

		if (argv[optind][0] == '0' && argv[optind][1] == 'x')
                {
                        //printf("bitmap is: %s\n", argv[optind]);
			bitmap_struct->bitmap = convert_to_bitmap(argv[optind]);
			if (bitmap_struct->bitmap < 0)	{
				err = -1;
				goto out;
			}
			
		}

		else if (strcmp(argv[optind],"all") == 0)	{
			printf("intercept all functions\n");
			bitmap_struct->bitmap = 4294967295;
		}

		else if (strcmp(argv[optind],"none") == 0)	{
			printf("intercept no functions\n");
			bitmap_struct->bitmap = 0;
		}

		else	{
			printf("undefined argument\n");
			err = -1;
			goto out;
		}

		printf("bitmap is: %lu\n", bitmap_struct->bitmap);
		
	}
	
	else if (argc == 2)	{
		/* read bitmap from file system */
		/* printf("read bitmap from kernel\n"); */
		

	}
	
	#ifdef EXTRA_CREDIT
	else if (argc >= 3)	{
		/* printf("EC is defined\n"); */
		if (argv[optind][0] == '0' && argv[optind][1] == 'x')	{
			err = -1;
			printf("invalid arguments entered\n");
		}

	}
	#endif

	else	{
		printf("invalid arguments\n");
		err = -1;
		goto out;
	}
	
	/* copy path and open fd */
	/* printf("path to copy: %s\n", argv[argc - 1]); */
	
	path = (char *) malloc(strlen(argv[argc - 1]) + 1);
	if (path == NULL)	{
		//printf("couldn't allocate enough memory for path\n");
		err = -1;
		goto out;
	}
	strcpy(path, argv[argc - 1]);

        if (path == NULL)       {
		err = -1;
		goto free_path;
	}

	/* path copied */

	err = validate_path(path);
	if (err < 0)   {
		/* it is a directory */
		err = -1;
		goto free_path;
	}

	/* path validated, directory exists */

	err = validate_mount(path);
	if (err < 0)	{
		/* file system has been mounted */
		err = -1;
		goto free_path;
	}

	/* file system is mounted */

	/* printf("file system mounted\n"); */
	fd = open(path, O_RDONLY);
	if (fd < 0)     {
		/* file open failed */
		printf("file open failed\n");
		err = -1;
		goto free_path;
	}

	/* call ioctl with flag set */
	if (argc == 2)	{
		err = ioctl(fd, IOCTL_READ_BITMAP, bitmap_struct);
		if (err < 0)	{
			/* ioctl read failed */
			printf("ioctl read failed\n");
			err = -1;
			goto close_file;
		}

		printf("read from user: %lu\n", bitmap_struct->bitmap); 
	}

	else if (argc == 3 && (argv[optind][0] != '+' && argv[optind][0] != '-'))	{
		err = ioctl(fd, IOCTL_WRITE_BITMAP, bitmap_struct);
		if (err < 0)	{
			/* ioctl write failed */
			printf("ioctl write failed\n");
			err = -1;
			goto close_file;
		}

		/* printf("written bitmap successfully\n"); */
	}

	#ifdef EXTRA_CREDIT
	else	{
		
		err = ioctl(fd, IOCTL_READ_BITMAP, bitmap_struct);
		if (err < 0)	{
			err = -1;
			printf("ioctl read failed");
			goto out;
		}

		while (optind < argc - 1)	{

			if (argv[optind][0] != '+' && argv[optind][0] != '-')
			{
/* COP   */				

				if (argv[optind][0] == '0'
					&& argv[optind][1] == 'x')
                		{
                        		//printf("bitmap is: %s\n", argv[optind]);
					bitmap_struct->bitmap =
						convert_to_bitmap(argv[optind]);
						if (bitmap_struct->bitmap < 0)  {
							err = -1;
							goto out;
						}
                        	}  

				else if (strcmp(argv[optind],"all") == 0)       {
					printf("intercept all functions\n");
                                        bitmap_struct->bitmap = 4294967295;
				}
				
				else if (strcmp(argv[optind],"none") == 0)      {
                        		printf("intercept no functions\n");
                        		bitmap_struct->bitmap = 0;
                		}

				else    {
                        		printf("undefined argument\n");
                        		err = -1;
                        		goto out;
                		}

                		printf("bitmap is: %lu\n", bitmap_struct->bitmap);

/* COP */


			}
			else	{
				syscall_bit = find_syscall_bit(argv[optind]);
				if (syscall_bit < 0)	{
					err = -1;
					goto out;
				}

				if (argv[optind][0] == '+')	{
					/* set value */
					bitmap_struct->bitmap = bitmap_struct->bitmap | syscall_bit;
				}

				else	{
					/* reset value */
					syscall_bit = syscall_bit ^ 4294967295;
					bitmap_struct->bitmap =  bitmap_struct->bitmap
						& syscall_bit;
		
				}
			}
			
			++optind;
		}

		/* printf("bitmap is : %lu \n", *bitmap); */
		err = ioctl(fd, IOCTL_WRITE_BITMAP, bitmap_struct);
		
		if (err < 0)	{
			/* ioctl update failed */
			printf("ioctl update failed\n");
			err = -1;
			goto close_file;
		}

		/* printf("ioctl updated successfully\n"); */
	}
	#endif

	perror("\nresult");

close_file:
	close(fd);

free_path:
	free(path);

out:
	if (bitmap_struct)
		free(bitmap_struct);
	printf("value returned: %d\n", err);
	return err;

}
