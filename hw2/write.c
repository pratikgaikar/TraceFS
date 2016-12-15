#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include<stdlib.h>
int main (void) {
        int fp,fp1,fp2;
        ssize_t bytes_written=0;
        char *str = (char *)malloc(30);

        strcpy(str,"Test Data");

        fp = open ("/mnt/trfs/test1.txt",O_RDWR);
        if (fp<0) {
                printf("File not created okay, errno = %d\n", errno);
                return 1;
        }
        bytes_written = write(fp,str,20);
        close(fp);
        return 0;
}

