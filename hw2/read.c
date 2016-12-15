#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include<stdlib.h>

int main (void) {
    int fp;
    char *str;
    str = (char *)malloc(100);
    ssize_t bytes_read=0;
    fp = open ("/mnt/trfs/test1.txt",O_RDONLY);
    if (fp<0) {
        printf ("File not created  okay, errno = %d\n", errno);
        return 1;
    }

    bytes_read = read(fp,str,100);
    free(str);
    close(fp);
    return 0;
}

