# Sisop-4-2024-MH-IT27
Anggota Kelompok :
|  NRP|Nama Anggota  |
|--|--|
|5027231079|Harwinda|
|5027221058|Irfan Qobus Salim|
|5027231038|Dani Wahyu Anak Ary|

List Soal:

 - [Soal 1](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/tree/main/soal_1)
 - [Soal 2](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/tree/main/soal_2)
 - [Soal 3](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/tree/main/soal_3)

## Soal 1
<details><summary>Klik untuk melihat soal</summary>
</details>
 
### Penjelasan
Berikut merupakan isi dari **inikaryakita.c**

```c
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
```

- Pertama-tama, kita menentukan direktori yang akan kita mount nantinya

```c
static const char *dirpath = "/home/irfanqs/modul4/soal_1/portofolio/";
```

- Kemudian, kita membuat fungsi `keyfee_getattr` untuk mendapatkan atribut file atau direktori

```c
static int keyfee_getattr(const char *path, struct stat *stbuf)
{
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;
    return 0;
}
```

- Kemudian, kita membuat fungsi `keyfee_mkdir` agar fuse dapat membuat folder baru

```c
static int keyfee_mkdir(const char *path, mode_t mode) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = mkdir(fpath, mode);
    if (res == -1)
        return -errno;
    return 0;
}
```
- Kemudian, kita membuat fungsi `keyfee_readdir` agar fuse bisa membaca isi direktori

```c
static int keyfee_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
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
```

- Kemudian, kita membuat fungsi `add_watermark` untuk menambahkan watermark ke gambar

```c
static void add_watermark(const char *filepath) {
    char command[1000];
    sprintf(command, "convert %s -gravity South -pointsize 36 -fill white -draw \"text 0,10 'inikaryakita.id'\" %s", filepath, filepath);
    system(command);
}
```

- Kemudian, kita membuat fungsi `keyfee_rename` untuk mengganti nama file atau direktori. Apabila user memindahkan gambar ke folder dengan prefix "wm", maka file tersebut akan diberi watermark

```c
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
```

- Kemudian, kita membuat fungsi `keyfee_open` untuk membuka file

```c
static int keyfee_open(const char *path, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = open(fpath, fi->flags);
    if (res == -1)
        return -errno;
    close(res);
    return 0;
}
```

- Kemudian, kita membuat fungsi `keyfee_read` agar fuse dapat membaca data sebuah file

```c
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
```

- Kemudian, kita membuat fungsi `reverse_string` untuk mengubah isi file menjadi terbalik

```c
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
```

- Kemudian, kita membuat fungsi `keyfee_write` untuk menuliskan isi file. Jika file yang ditulis tersebut memiliki prefix "test", maka file tersebut otomatis terbalik isi stringnya apabila disave.

```c
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
```

- Kemudian, kita membuat fungsi `keyfee_create` untuk membuat file baru

```c
static int keyfee_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);

    int fd = creat(fpath, mode);
    if (fd == -1)
        return -errno;

    close(fd);
    return 0;
}
```

- Kemudian, kita membuat fungsi `keyfee_truncate` untuk mengubah ukuran file

```c
static int keyfee_truncate(const char *path, off_t size) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = truncate(fpath, size);
    if (res == -1)
        return -errno;
    return 0;
}
```

- Kemudian, kita membuat fungsi `keyfee_chmod` untuk mengubah perizinan sebuah file

```c
static int keyfee_chmod(const char *path, mode_t mode) {
    char fpath[1000];
    sprintf(fpath, "%s%s", dirpath, path);
    int res = chmod(fpath, mode);
    if (res == -1)
        return -errno;
    return 0;
}
```

- Terakhir, kita perlu membuat struktur fuse operations agar semua fungsi di atas bisa dijalankan

```c
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
```

- Lanjut ke dalam fungsi main, kita call `fuse_main` beserta dengan parameternya

```c
int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &keyfee_oper, NULL);
}
```

### Output
Pertama-tama, kita perlu mengcompile program c dengan command `gcc -Wall `pkg-config fuse --cflags` inikaryakita.c -o ikk `pkg-config fuse --libs``, setelah itu jalankan programnya.<br>
![image](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/assets/130438307/179b2ed0-fa06-4b8b-878c-d6395bc14734)
<br>Hasil gambar yang diberi watermark setelah dipindah ke folder dengan prefix "wm"<br>
![image](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/assets/130438307/ff502287-3de3-46ac-a9fd-c576fddd6af9)
<br>Berikut output untuk mengubah string menjadi terbalik jika file yang disave memiliki prefix "test" serta menjalankan sebuah file script.sh<br>
![image](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/assets/130438307/2caaf456-1cf2-4a60-b248-71432df198ba)
<br>Berikut output dari file yang tidak memiliki prefix "test" apabila disave (tidak ada perubahan)<br>
![image](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/assets/130438307/d5611bb2-3a36-4044-884d-4e0ce497c07b)

### Kendala
Tidak ada kendala pada soal ini

### Revisi
Tidak ada revisi pada soal ini

## Soal 2
<details><summary>Klik untuk melihat soal</summary>
Masih dengan Ini Karya Kita, sang CEO ingin melakukan tes keamanan pada folder sensitif Ini Karya Kita. Karena Teknologi Informasi merupakan departemen dengan salah satu fokus di Cyber Security, maka dia kembali meminta bantuan mahasiswa Teknologi Informasi angkatan 2023 untuk menguji dan mengatur keamanan pada folder sensitif tersebut. Untuk mendapatkan folder sensitif itu, mahasiswa IT 23 harus kembali mengunjungi website Ini Karya Kita pada www.inikaryakita.id/schedule . Silahkan isi semua formnya, tapi pada form subject isi dengan nama kelompok_SISOP24 , ex: IT01_SISOP24 . Lalu untuk form Masukkan Pesanmu, ketik “Mau Foldernya” . Tunggu hingga 1x24 jam, maka folder sensitif tersebut akan dikirimkan melalui email kalian. Apabila folder tidak dikirimkan ke email kalian, maka hubungi sang CEO untuk meminta bantuan.   
Pada folder "pesan" Adfi ingin meningkatkan kemampuan sistemnya dalam mengelola berkas-berkas teks dengan menggunakan fuse.
Jika sebuah file memiliki prefix "base64," maka sistem akan langsung mendekode isi file tersebut dengan algoritma Base64.
Jika sebuah file memiliki prefix "rot13," maka isi file tersebut akan langsung di-decode dengan algoritma ROT13.
Jika sebuah file memiliki prefix "hex," maka isi file tersebut akan langsung di-decode dari representasi heksadesimalnya.
Jika sebuah file memiliki prefix "rev," maka isi file tersebut akan langsung di-decode dengan cara membalikkan teksnya.
Contoh:
File yang belum didecode/ dienkripsi rot_13


File yang sudah didecode/ dienkripsi rot_13


Pada folder “rahasia-berkas”, Adfi dan timnya memutuskan untuk menerapkan kebijakan khusus. Mereka ingin memastikan bahwa folder dengan prefix "rahasia" tidak dapat diakses tanpa izin khusus. 
Jika seseorang ingin mengakses folder dan file pada “rahasia”, mereka harus memasukkan sebuah password terlebih dahulu (password bebas). 
Setiap proses yang dilakukan akan tercatat pada logs-fuse.log dengan format :
[SUCCESS/FAILED]::dd/mm/yyyy-hh:mm:ss::[tag]::[information]
Ex:
[SUCCESS]::01/11/2023-10:43:43::[moveFile]::[File moved successfully]
Catatan: 
Tolong cek email kalian secara berkala untuk mendapatkan folder sensitifnya
Jika ada pertanyaan, bisa menghubungi Adfi via WA aja, nanti dikasih hint

</details>

### Penjelasan
buat file `pastibisa.c`
#### 1. Header dan Konstanta
```c
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
```
- mendefinisikan konstanta untuk menuntukan path file log dan prefix folder sensitif
```c
#define LOG_FILE "/home/winds/SISOP/modul4/soal_2/logs-fuse.log"
#define SENSITIVE_FOLDER_PREFIX "/rahasia"
```
- path dan password untuk mengakses folder sensitif
```c
static const char *dirpath = "/home/winds/SISOP/modul4/soal_2/sensitif";
static const char *password = "winds123";
```

#### 2. Pengelolaan Password
- Fungsi untuk memeriksa password `check_password` apakah sesuai dengan password yang sudah ditetapkan
 ```c
  static int check_password(const char *input) {
    return strcmp(input, password) == 0;
}
```
- Fungsi untuk meminta password `ask_password` dari pengguna dan memeriksa kesesuaiannya.
```c
static int ask_password() {
    char input[100];
    printf("Enter password: ");
    if (scanf("%99s", input) != 1) {
        return 0;
    }
    return check_password(input);
}
```
#### 3. Fungsi Logging
- `log_access`: Mencatat usaha akses ke file log yang ditentukan dengan waktu, tag, dan pesan info
```c
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
```
#### 4. Fungsi Dekoding
- `base64_decode`: Mendekode file yang dikodekan dengan base64
```c
void base64_decode(const char *input, char *output) {
    base64_decodestate state;
    base64_init_decodestate(&state);
    int output_len = base64_decode_block(input, strlen(input), output, &state);
    output[output_len] = '\0';
}
```
- `rot13_decode`: Mendekode file yang dikodekan dengan ROT13
```c
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
```
- `hex_decode`: Mendekode file yang dikodekan dengan hexadecimal
```c
void hex_decode(const char *input, char *output) {
    int i, j = 0;
    int len = strlen(input);
    for (i = 0; i < len; i += 2) {
        sscanf(input + i, "%2hhx", &output[j]);
        j++;
    }
    output[j] = '\0';
}
```
- `reverse_decode`: Mendekode file yang dikodekan secara terbalik
```c
void reverse_decode(const char *input, char *output) {
    int len = strlen(input);
    int i;
    for (i = 0; i < len; i++) {
        output[i] = input[len - i - 1];
    }
    output[i] = '\0';
}
```
#### 5. Operasi FUSE
- `xmp_getattr`: Mendapatkan atribut file dan mencatat aksesnya
```C
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
```
- `xmp_readdir`: Membaca isi direktori. Jika direktori sensitif, meminta password
```C
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
```
- `xmp_open`: Membuka file dan mencatat operasi
```C
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
```
- `xmp_read`: Membaca konten file. Jika file dikodekan dalam base64, ROT13, hexadecimal, atau terbalik, maka file didekode sebelum dikembalikan
```c
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
```
#### 6. Struktur Operasi Fuse
- xmp_oper: Mendefinisikan operasi FUSE, memetakan masing-masing fungsi ke operasi terkait
```c
static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open = xmp_open,
    .read = xmp_read,
};
```
#### 7. Fungsi Utama
- Menginisialisasi filesystem FUSE dengan `fuse_main` menggunakan operasi yang telah didefinisikan
```c
int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
```

#### 8. untuk Kompilasi
- untuk mengcompile code gunakan:
```c
gcc -Wall -I/path/to/libb64/include -L/path/to/libb64/lib pastibisa.c -o pastibisa pkg-config fuse --cflags --libs -lb64
```

#### 9. untuk menjalankan
- untuk run program tersebut:
```c
/pastibisa -f /path/to/mount
```
Pastikan path file log dan direktori dasar diatur dengan benar sesuai

### Dokumentasi:
![Screenshot 2024-05-25 211626](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/assets/79549192/564f99a2-9b53-481e-bddd-6b50aee184d5)

*tampilan mengakses folder `pesan` atau dekode file yang ada dalam folder `pesan`:*
![Screenshot 2024-05-25 211942](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/assets/79549192/0d206df8-062f-4a58-ab17-d107e19e39e0)

*tampilan mengakses folder sensitif:*
![Screenshot 2024-05-25 212228](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/assets/79549192/5003634f-673f-4cfc-b8da-b91bf72237b1)

*tampilan LOG:*
![85871718-c20c-4587-b615-ee1ceb539706](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/assets/79549192/e22521a6-5c4a-4c82-9cbc-20202971ff70)



### Kendala
program ini tidak dapat bekerja sempurna di kali, khususnya untuk folder sensitif yang memerlukan password. 
setiap mengetik password ngelooping terus menerus

### Revisi
Tidak ada revisi pada soal ini

## Soal 3
<details><summary>Klik untuk melihat soal</summary>
	
</details>

### Penjelasan

## Instalasi

1. **Instal libfuse**:
    ```bash
    sudo apt-get install fuse libfuse-dev
    ```

2. **Kloning repositori ini**:
    ```bash
    git clone https://github.com/username/relicsfs.git
    cd relicsfs
    ```

3. **Kompilasi kode sumber**:
    ```bash
    gcc -Wall relicsfs.c `pkg-config fuse --cflags --libs` -o relicsfs
    ```

## Penggunaan

1. **Buat direktori untuk mount point**:
    ```bash
    mkdir /path/to/mountpoint
    ```

2. **Jalankan file system**:
    ```bash
    ./relicsfs /path/to/mountpoint
    ```

3. **Akses file sistem**:
    - File yang disimpan di dalam direktori `relics_dir` (dalam contoh ini: `/home/dw/dani/relics`) akan muncul sebagai file utuh di dalam mount point.

4. **Unmount file system**:
    ```bash
    fusermount -u /path/to/mountpoint
    ```

## Struktur Kode

### Global Variables

- `relics_dir`: Direktori tempat menyimpan bagian-bagian file.

### Fungsi Utama

#### `concatenate_file_parts`

Menyatukan bagian-bagian file yang dinamai dengan format `path.000`, `path.001`, dst., dan menyimpannya dalam buffer.
```c
static int concatenate_file_parts(const char *path, char **buffer, size_t *size) {
   
}
```

#### `relics_getattr`

Mengambil atribut file atau direktori.
```c
static int relics_getattr(const char *path, struct stat *stbuf) {

}
```

#### `relics_readdir`

Membaca isi direktori.
```c
static int relics_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

}
```

#### `relics_open`

Membuka file dan memastikan bagian-bagiannya bisa disatukan.
```c
static int relics_open(const char *path, struct fuse_file_info *fi) {

}
```

#### `relics_read`

Membaca data dari file yang sudah disatukan.
```c
static int relics_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

}
```

### Struktur FUSE Operations

Menghubungkan operasi FUSE dengan fungsi yang sesuai.
```c
static struct fuse_operations relics_operations = {
    .getattr = relics_getattr,
    .readdir = relics_readdir,
    .open = relics_open,
    .read = relics_read,
};
```

### `main` Function

Titik masuk untuk file system FUSE.
```c
int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &relics_operations, NULL);
}
```


### Kendala
Soal tidak daoat berjalan karena terkendala permission

### Revisi
Tidak ada revisi pada soal in

