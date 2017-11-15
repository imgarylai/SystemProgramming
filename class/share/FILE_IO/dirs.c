/* Program illustrating reading of directories and mode bits */
/* This version uses chdir and relative path names */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
void main(int argc, char **argv) {

  DIR *dirp;
  struct dirent *dp;
  struct stat buf;

  if (!(dirp = opendir(argv[1]))) { /* open the directory */
    perror("opendir:");
    exit(1);
  }
  chdir(argv[1]); /* change to target directory, so filenames
                         are relative to where we are */

  while ((dp = readdir(dirp)) != NULL) { /* print names/inodes */
    printf("%-10d%s", (int)dp->d_ino, dp->d_name);

    if (stat(dp->d_name, &buf) == -1) { /* get inode info*/
      printf("\n");
      perror("Bad Stat");
    }

    /* print modes */
    printf("\tmode=%o R=%d D=%d B=%d C=%d L=%d\n", (int)buf.st_mode,
           S_ISREG(buf.st_mode), S_ISDIR(buf.st_mode), S_ISBLK(buf.st_mode),
           S_ISCHR(buf.st_mode), S_ISLNK(buf.st_mode));
  }
  closedir(dirp);
}
