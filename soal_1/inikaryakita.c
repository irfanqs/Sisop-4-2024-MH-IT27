#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

static const char *dirpath = "/home/irfanqs/modul4/soal_1/portofolio/";

static int keyfee_getattr(const char *path, struct stat *stbuf)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;
    return 0;
}

static int keyfee_mkdir(const char *path, mode_t mode)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = mkdir(fpath, mode);
    if (res == -1)
        return -errno;
    return 0;
}

static int keyfee_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    DIR *dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }
    closedir(dp);
    return 0;
}

static void add_watermark(const char *filepath) {
    char command[1000];
    sprintf(command, "convert %s -gravity South -pointsize 36 -fill white -draw \"text 0,10 'inikaryakita.id'\" %s", filepath, filepath);
    system(command);
}

static int keyfee_rename(const char *from, const char *to) {
    char ffrom[1000];
    char fto[1000];
    sprintf(ffrom, "%s%s", dirpath, from);
    sprintf(fto, "%s%s", dirpath, to);

    int res = rename(ffrom, fto);
    if (res == -1)
        return -errno;

    if (strstr(to, "wm-") != NULL) {
        add_watermark(fto);
    }
    return 0;
}

static int keyfee_open(const char *path, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = open(fpath, fi->flags);
    if (res == -1)
        return -errno;
    close(res);
    return 0;
}

static int keyfee_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

static void reverse_string(char *str, size_t length) {
    size_t start = 0;
    size_t end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static int keyfee_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);

    int fd = open(fpath, O_WRONLY);
    if (fd == -1)
        return -errno;

    const char *filename = strrchr(path, '/') + 1;
    if (strncmp(filename, "test", 4) == 0) {
        char *reversed_buf = (char *)malloc(size);
        if (!reversed_buf) {
            close(fd);
            return -ENOMEM;
        }
        memcpy(reversed_buf, buf, size);
        reverse_string(reversed_buf, size);

        int res = pwrite(fd, reversed_buf, size, offset);
        free(reversed_buf);

        if (res == -1) {
            res = -errno;
        }
        close(fd);
        return res;
    }

    int res = pwrite(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

static int keyfee_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);

    int fd = creat(fpath, mode);
    if (fd == -1)
        return -errno;

    close(fd);
    return 0;
}

static int keyfee_truncate(const char *path, off_t size) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = truncate(fpath, size);
    if (res == -1)
        return -errno;
    return 0;
}

static int keyfee_chmod(const char *path, mode_t mode) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = chmod(fpath, mode);
    if (res == -1)
        return -errno;
    return 0;
}

static struct fuse_operations keyfee_oper = {
    .getattr    = keyfee_getattr,
    .mkdir      = keyfee_mkdir,
    .readdir    = keyfee_readdir,
    .rename     = keyfee_rename,
    .open       = keyfee_open,
    .read       = keyfee_read,
    .write      = keyfee_write,
    .create     = keyfee_create,
    .truncate   = keyfee_truncate,
    .chmod      = keyfee_chmod,
};

int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &keyfee_oper, NULL);
}
