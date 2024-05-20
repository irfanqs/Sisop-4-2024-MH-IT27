#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <b64/cdecode.h>

#define LOG_FILE "/home/winds/SISOP/modul4/soal_2/logs-fuse.log"
#define SENSITIVE_FOLDER_PREFIX "/rahasia"

static const char *dirpath = "/home/winds/SISOP/modul4/soal_2/sensitif";
static const char *password = "winds123";

static int check_password(const char *input) {
    return strcmp(input, password) == 0;
}

static int ask_password() {
    char input[100];
    printf("Enter password: ");
    if (scanf("%99s", input) != 1) {
        return 0;
    }
    return check_password(input);
}

void log_access(const char *tag, const char *info) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char time_str[26];
        strftime(time_str, 26, "%d/%m/%Y-%H:%M:%S", tm_info);
        fprintf(log_file, "[%s]::%s::%s\n", time_str, tag, info);
        fclose(log_file);
    } else {
        fprintf(stderr, "Gagal membuka file log.\n");
    }
}

void base64_decode(const char *input, char *output) {
    base64_decodestate state;
    base64_init_decodestate(&state);
    int output_len = base64_decode_block(input, strlen(input), output, &state);
    output[output_len] = '\0';
}

void rot13_decode(const char *input, char *output) {
    int i;
    int len = strlen(input);
    for (i = 0; i < len; i++) {
        if (isalpha(input[i])) {
            if (islower(input[i])) {
                output[i] = ((input[i] - 'a' + 13) % 26) + 'a';
            } else {
                output[i] = ((input[i] - 'A' + 13) % 26) + 'A';
            }
        } else {
            output[i] = input[i];
        }
    }
    output[len] = '\0';
}

void hex_decode(const char *input, char *output) {
    int i, j = 0;
    int len = strlen(input);
    for (i = 0; i < len; i += 2) {
        sscanf(input + i, "%2hhx", &output[j]);
        j++;
    }
    output[j] = '\0';
}

void reverse_decode(const char *input, char *output) {
    int len = strlen(input);
    int i;
    for (i = 0; i < len; i++) {
        output[i] = input[len - i - 1];
    }
    output[i] = '\0';
}

static int xmp_getattr(const char *path, struct stat *stbuf) {
    int res;
    char fpath[1000];
    snprintf(fpath, sizeof(fpath), "%s%s", dirpath, path);

    res = lstat(fpath, stbuf);
    if (res == -1) {
        log_access("GETATTR", "FAILED to retrieve file attributes");
        return -errno;
    }

    log_access("GETATTR", "SUCCESS to retrieve file attributes");
    printf("Fuse: Get File Attribute\n");
    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    char fpath[1000];

    if (strcmp(path, "/") == 0) {
        path = dirpath;
        snprintf(fpath, sizeof(fpath), "%s", path);
    } else {
        snprintf(fpath, sizeof(fpath), "%s%s", dirpath, path);
    }

    if (strncmp(path, SENSITIVE_FOLDER_PREFIX, strlen(SENSITIVE_FOLDER_PREFIX)) == 0) {
        if (!ask_password()) {
            log_access("READDIR", "FAILED due to incorrect password");
            return -EACCES;
        }
    }

    DIR *dp;
    struct dirent *de;

    dp = opendir(fpath);
    if (dp == NULL) {
        log_access("READDIR", "FAILED to open directory");
        return -errno;
    }

    log_access("READDIR", "SUCCESS to open directory");
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0)) {
            break;
        }
    }

    closedir(dp);
    printf("Fuse: Directory Read\n");
    log_access("READDIR", "SUCCESS to read directory contents");
    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) {
    int res;
    char fpath[1000];
    snprintf(fpath, sizeof(fpath), "%s%s", dirpath, path);
    res = open(fpath, fi->flags);
    if (res == -1) {
        log_access("OPEN", "FAILED to open file");
        return -errno;
    }

    close(res);
    printf("Fuse: File Open\n");
    log_access("OPEN", "SUCCESS to open file");
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fpath[1000];
    snprintf(fpath, sizeof(fpath), "%s%s", dirpath, path);

    int res;
    int fd = open(fpath, O_RDONLY);
    if (fd == -1) {
        log_access("READ", "FAILED to open file for reading");
        return -errno;
    }

    if (strncmp(path, SENSITIVE_FOLDER_PREFIX, strlen(SENSITIVE_FOLDER_PREFIX)) == 0) {
        if (!ask_password()) {
            return -EACCES;
        }
    }

    if (strstr(path, "base64")) {
        char encoded[2048];
        res = pread(fd, encoded, sizeof(encoded) - 1, offset);
        if (res == -1) {
            log_access("READ", "FAILED to read base64 file");
            res = -errno;
            goto out;
        }
        encoded[res] = '\0';
        base64_decode(encoded, buf);
        size_t decoded_size = strlen(buf);
        log_access("READ", "SUCCESS to read and decode base64 file");
        res = decoded_size;
    } else if (strstr(path, "rot13")) {
        char encoded[2048];
        res = pread(fd, encoded, sizeof(encoded) - 1, offset);
        if (res == -1) {
            log_access("READ", "FAILED to read rot13 file");
            res = -errno;
            goto out;
        }
        encoded[res] = '\0';
        rot13_decode(encoded, buf);
        size_t decoded_size = strlen(buf);
        log_access("READ", "SUCCESS to read and decode rot13 file");
        res = decoded_size;
    } else if (strstr(path, "hex")) {
        char encoded[2048];
        char decoded[1024];
        res = pread(fd, encoded, sizeof(encoded) - 1, offset);
        if (res == -1) {
            log_access("READ", "FAILED to read hex file");
            res = -errno;
            goto out;
        }
        encoded[res] = '\0';
        hex_decode(encoded, decoded);
        size_t decoded_size = strlen(decoded);
        memcpy(buf, decoded, decoded_size);
        log_access("READ", "SUCCESS to read and decode hex file");
        res = decoded_size;
    } else if (strstr(path, "rev")) {
        char encoded[2048];
        res = pread(fd, encoded, sizeof(encoded) - 1, offset);
        if (res == -1) {
            log_access("READ", "FAILED to read reverse file");
            res = -errno;
            goto out;
        }
        encoded[res] = '\0';
        reverse_decode(encoded, buf);
        size_t decoded_size = strlen(buf);
        log_access("READ", "SUCCESS to read and decode reverse file");
        res = decoded_size;
    } else {
        res = pread(fd, buf, size, offset);
        if (res == -1) {
            log_access("READ", "FAILED to read file");
            res = -errno;
        } else {
            log_access("READ", "SUCCESS to read file");
        }
    }

out:
    printf("Fuse: File Read\n");
    close(fd);
    return res;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open = xmp_open,
    .read = xmp_read,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
