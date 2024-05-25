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
=
## Soal 1
<details><summary>Klik untuk melihat soal</summary>
</details>
 
### Penjelasan

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

### Kendala
Tidak ada kendala pada soal ini

### Revisi
Tidak ada revisi pada soal ini

