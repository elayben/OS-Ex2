// Elay Ben Yehoshua 214795668
#include "copytree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

// copy from src to dest
void copy_file(const char *src, const char *dest, int copy_symlinks, int copy_permissions) {
    struct stat stat_buf;
    if (lstat(src, &stat_buf) == -1) {
        perror("lstat failed");
        return;
    }

    if (copy_symlinks && S_ISLNK(stat_buf.st_mode)) {
        char link_target[PATH_MAX];
        ssize_t len = readlink(src, link_target, sizeof(link_target) - 1);
        if (len == -1) {
            perror("readlink failed");
            return;
        }
        link_target[len] = '\0';

        if (symlink(link_target, dest) == -1) {
            perror("symlink failed");
            return;
        }
    } else {
        int src_fd = open(src, O_RDONLY);
        if (src_fd == -1) {
            perror("open failed");
            return;
        }

        int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, stat_buf.st_mode);
        if (dest_fd == -1) {
            perror("open failed");
            close(src_fd);
            return;
        }

        char buffer[8192];
        ssize_t bytes;
        while ((bytes = read(src_fd, buffer, sizeof(buffer))) > 0) {
            if (write(dest_fd, buffer, bytes) != bytes) {
                perror("write failed");
                close(src_fd);
                close(dest_fd);
                return;
            }
        }
        if (bytes == -1) {
            perror("read failed");
        }

        close(src_fd);
        close(dest_fd);

        if (copy_permissions) {
            if (chmod(dest, stat_buf.st_mode) == -1) {
                perror("chmod failed");
            }
        }
    }
}

// create directories recursively
void create_directory(const char *path) {
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, S_IRWXU) == -1) {
                if (errno != EEXIST) {
                    perror("mkdir failed");
                    return;
                }
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, S_IRWXU) == -1) {
        if (errno != EEXIST) {
            perror("mkdir failed");
            return;
        }
    }
}

// copy directory 
void copy_directory(const char *src, const char *dest, int copy_symlinks, int copy_permissions) {
    DIR *dir = opendir(src);
    if (!dir) {
        perror("opendir failed");
        return;
    }

    create_directory(dest);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_path[PATH_MAX];
        char dest_path[PATH_MAX];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

        struct stat stat_buf;
        if (lstat(src_path, &stat_buf) == -1) {
            perror("lstat failed");
            continue;
        }

        if (S_ISDIR(stat_buf.st_mode)) {
            copy_directory(src_path, dest_path, copy_symlinks, copy_permissions);
        } else {
            copy_file(src_path, dest_path, copy_symlinks, copy_permissions);
        }
    }

    closedir(dir);
}