#define FUSE_USE_VERSION 29
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>

static const char *relics_dir = "/home/dw/dani/relics"; 

static int concatenate_file_parts(const char *path, char **buffer, size_t *size) {
    char part_path[PATH_MAX];
    FILE *fp;
    size_t total_size = 0;
    int part_num = 0;

    *buffer = NULL;

    while (1) {
        sprintf(part_path, "%s.%03d", path, part_num);
        fp = fopen(part_path, "rb");
        if (!fp)
            break;

        fseek(fp, 0, SEEK_END);
        size_t part_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *buffer = realloc(*buffer, total_size + part_size);
        fread(*buffer + total_size, 1, part_size, fp);
        total_size += part_size;

        fclose(fp);
        part_num++;
    }

    *size = total_size;
    return (part_num > 0) ? 0 : -ENOENT;
}

static int relics_getattr(const char *path, struct stat *stbuf) {
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {
        char relic_path[PATH_MAX];
        sprintf(relic_path, "%s%s", relics_dir, path);

        res = lstat(relic_path, stbuf);
        if (res == -1) {
            char *buffer;
            size_t size;
            res = concatenate_file_parts(relic_path, &buffer, &size);
            free(buffer);

            if (res == 0) {
                stbuf->st_mode = S_IFREG | 0444;
                stbuf->st_nlink = 1;
                stbuf->st_size = size;
                return 0;
            } else {
                return -errno;
            }
        }
    }

    return res;
}

static int relics_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi) {
    (void)offset;
    (void)fi;

    DIR *dp;
    struct dirent *de;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    dp = opendir(relics_dir);
    if (!dp)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_REG) {
            char *ext = strrchr(de->d_name, '.');
            if (ext && strlen(ext) == 4 && isdigit(ext[1]) && isdigit(ext[2]) && isdigit(ext[3])) {
                char filename[PATH_MAX];
                strncpy(filename, de->d_name, ext - de->d_name);
                filename[ext - de->d_name] = '\0';
                filler(buf, filename, NULL, 0);
            }
        }
    }

    closedir(dp);
    return 0;
}

static int relics_open(const char *path, struct fuse_file_info *fi) {
    char relic_path[PATH_MAX];
    sprintf(relic_path, "%s%s", relics_dir, path);

    char *buffer;
    size_t size;
    int res = concatenate_file_parts(relic_path, &buffer, &size);
    free(buffer);

    if (res == -1)
        return -ENOENT;

    return 0;
}

static int relics_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    char relic_path[PATH_MAX];
    sprintf(relic_path, "%s%s", relics_dir, path);

    char *buffer;
    size_t file_size;
    int res = concatenate_file_parts(relic_path, &buffer, &file_size);

    if (res == -1)
        return -ENOENT;

    if (offset < file_size) {
        if (offset + size > file_size)
            size = file_size - offset;
        memcpy(buf, buffer + offset, size);
    } else {
        size = 0;
    }

    free(buffer);
    return size;
}

static struct fuse_operations relics_operations = {
    .getattr = relics_getattr,
    .readdir = relics_readdir,
    .open = relics_open,
    .read = relics_read,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &relics_operations, NULL);
}
