Anggota Kelompok :
|  NRP|Nama Anggota  |
|--|--|
|5027231079|Harwinda|
|5027221058|Irfan Qobus Salim|
|5027231038|Dani Wahyu Anak Ary|

List Soal:

 - [Soal 1](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/tree/main#soal_1)
 - [Soal 2](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/tree/main#soal_2)
 - [Soal 3](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/tree/main#soal_3)
 - [Soal 4](https://github.com/irfanqs/Sisop-4-2024-MH-IT27/tree/main#soal_4)

## Soal 1
<details><summary>Klik untuk melihat soal</summary>
Pada zaman dahulu pada galaksi yang jauh-jauh sekali, hiduplah seorang Stelle. Stelle adalah seseorang yang sangat tertarik dengan Tempat Sampah dan Parkiran Luar Angkasa. Stelle memulai untuk mencari Tempat Sampah dan Parkiran yang terbaik di angkasa. Dia memerlukan program untuk bisa secara otomatis mengetahui Tempat Sampah dan Parkiran dengan rating terbaik di angkasa. Programnya berbentuk microservice sebagai berikut:
- Dalam auth.c pastikan file yang masuk ke folder new-entry adalah file csv dan berakhiran trashcan dan parkinglot. Jika bukan, program akan secara langsung akan delete file tersebut. 
- Contoh dari nama file yang akan diautentikasi:
  - belobog_trashcan.csv
  - osaka_parkinglot.csv
- File csv yang lolos tahap autentikasi akan dikirim ke shared memory. 
- Dalam rate.c, proses akan mengambil data csv dari shared memory dan akan memberikan output Tempat Sampah dan Parkiran dengan Rating Terbaik dari data tersebut.
- Pada db.c, proses bisa memindahkan file dari new-data ke folder microservices/database, WAJIB MENGGUNAKAN SHARED MEMORY.
- Log semua file yang masuk ke folder microservices/database ke dalam file db.log dengan contoh format sebagai berikut:
**[DD/MM/YY hh:mm:ss] [type] [filename]**
ex : **[07/04/2024 08:34:50] [Trash Can] [belobog_trashcan.csv]**
</details>
 
### Penjelasan
Berikut merupakan kode dari **auth.c**, berfungsi untuk memfiltrasi file csv yang valid dari folder `new-data`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_FILENAME_LENGTH 512
#define MAX_FILES 10
#define SHARED_MEMORY_KEY 1234

typedef struct {
    char filenames[MAX_FILES][MAX_FILENAME_LENGTH];
    int count;
} SharedData;

void copy_to_shared_memory(const char *filename) {
    int shmid = shmget(SHARED_MEMORY_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *) -1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    if (strstr(filename, "trashcan")) {
        strcpy(shared_data->filenames[shared_data->count], "trashcan");
        shared_data->count++;
    } else if  (strstr(filename, "parkinglot")) {
        strcpy(shared_data->filenames[shared_data->count], "parkinglot");
        shared_data->count++;
    }

    strcpy(shared_data->filenames[shared_data->count], filename);
    shared_data->count++;

    if (shmdt(shared_data) == -1) {
        perror("Error detaching shared memory");
        exit(EXIT_FAILURE);
    }
}

void removing_non_trash_parking_file() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("new-data")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == 8) {
                char *filename = ent->d_name;
                char *extension = strrchr(filename, '.');
                char *name = strtok(filename, "_");

                if (extension != NULL && strcmp(extension, ".csv") == 0 &&
                    (strstr(name, "trashcan") || strstr(name, "parkinglot"))) {
                    copy_to_shared_memory(name);
                } else {
                    char filepath[MAX_FILENAME_LENGTH];
                    snprintf(filepath, sizeof(filepath), "new-data/%s", ent->d_name);
                    remove(filepath);
                }
            }
        }
        closedir(dir);
    } else {
        perror("Folder tidak dapat dibuka");
        exit(EXIT_FAILURE);
    }
}

int main() {
    removing_non_trash_parking_file();

    DIR *dp = opendir("new-data");
    struct dirent *ep;
    while ((ep = readdir(dp)) != NULL) {
        if (ep->d_type == 8) {
            copy_to_shared_memory(ep->d_name);
        }
    }
    closedir(dp);

    // Menampilkan isi array filenames ke terminal
    printf("Files in shared memory:\n");
    int shmid = shmget(SHARED_MEMORY_KEY, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("Failed to get shared memory segment");
        exit(EXIT_FAILURE);
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Failed to attach shared memory segment");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < shared_data->count; ++i) {
        printf("%s\n", shared_data->filenames[i]);
    }
    
    if (shmdt(shared_data) == -1) {
        perror("Failed to detach shared memory segment");
        exit(EXIT_FAILURE);
    }

    return 0;
}
```
- Pertama, kita membuat struct untuk menyimpan nama file dan kata dari trashcan maupun parkinglot. Kemudian data struct ini akan kita simpan ke dalam shared memory. Penyimpanan shared memory dilakukan oleh fungsi `copy_to_shared_memory()`.

```c
typedef struct {
    char filenames[MAX_FILES][MAX_FILENAME_LENGTH];
    int count;
} SharedData;

void copy_to_shared_memory(const char *filename) {
    int shmid = shmget(SHARED_MEMORY_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *) -1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    if (strstr(filename, "trashcan")) {
        strcpy(shared_data->filenames[shared_data->count], "trashcan");
        shared_data->count++;
    } else if  (strstr(filename, "parkinglot")) {
        strcpy(shared_data->filenames[shared_data->count], "parkinglot");
        shared_data->count++;
    }

    strcpy(shared_data->filenames[shared_data->count], filename);
    shared_data->count++;

    if (shmdt(shared_data) == -1) {
        perror("Error detaching shared memory");
        exit(EXIT_FAILURE);
    }
}
```
- Kemudian, kita membuat fungsi `removing_non_trash_parking_file()`. Fungsi ini berfungsi untuk menghapus file yang tidak memiliki format csv dan/atau tidak memiliki nama file trashcan atau parkinglot.

```c
void removing_non_trash_parking_file() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("new-data")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == 8) {
                char *filename = ent->d_name;
                char *extension = strrchr(filename, '.');
                char *name = strtok(filename, "_");

                if (extension != NULL && strcmp(extension, ".csv") == 0 &&
                    (strstr(name, "trashcan") || strstr(name, "parkinglot"))) {
                    copy_to_shared_memory(name);
                } else {
                    char filepath[MAX_FILENAME_LENGTH];
                    snprintf(filepath, sizeof(filepath), "new-data/%s", ent->d_name);
                    remove(filepath);
                }
            }
        }
        closedir(dir);
    } else {
        perror("Folder tidak dapat dibuka");
        exit(EXIT_FAILURE);
    }
}
```

- Masuk ke dalam fungsi main, kita akan menghapus file yang tidak sesuai kriteria, kemudian kita akan memindahkan nama dari file yang valid ke dalam shared memory dengan cara membuka folder `new-data` lalu memanggil fungsi `copy_to_shared_memory()`. Lalu kita akan menampilkan ke terminal, hal apa saja yang tersimpan di shared memory.

```c
int main() {
    removing_non_trash_parking_file();

    DIR *dp = opendir("new-data");
    struct dirent *ep;
    while ((ep = readdir(dp)) != NULL) {
        if (ep->d_type == 8) {
            copy_to_shared_memory(ep->d_name);
        }
    }
    closedir(dp);

    // Menampilkan isi array filenames ke terminal
    printf("Files in shared memory:\n");
    int shmid = shmget(SHARED_MEMORY_KEY, sizeof(SharedData), 0666);
    if (shmid == -1) {
        perror("Failed to get shared memory segment");
        exit(EXIT_FAILURE);
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Failed to attach shared memory segment");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < shared_data->count; ++i) {
        printf("%s\n", shared_data->filenames[i]);
    }
    
    if (shmdt(shared_data) == -1) {
        perror("Failed to detach shared memory segment");
        exit(EXIT_FAILURE);
    }

    return 0;
}
```

Berikut merupakan kode dari **rate.c**, berfungsi untuk menampilkan rating tertinggi dari tiap file csv yang telah terfiltrasi.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_filenames_LENGTH 512
#define MAX_FILES 10

typedef struct {
    char filenames[MAX_FILES][MAX_filenames_LENGTH];
    int count;
} SharedData;

typedef struct {
    char name[100];
    float rating;
} PlaceData;

int main() {
    key_t key = 1234;
    if (key == -1) {
        fprintf(stderr, "Failed to generate key\n");
        return 1;
    }

    int shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        fprintf(stderr, "Failed to create shared memory segment\n");
        return 1;
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        fprintf(stderr, "Failed to attach shared memory segment\n");
        return 1;
    }

    // start code
    for (int i = 0; i < shared_data->count; i += 2) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "../new-data/%s", shared_data->filenames[i+1]);

        FILE *file = fopen(fullpath, "r");
        if (!file) {
            fprintf(stderr, "Could not open file %s\n", fullpath);
        }

        char line[100];
        if (fgets(line, sizeof(line), file) == NULL) {
            fprintf(stderr, "Empty file or error reading header\n");
            fclose(file);
            return 1;
        }

        float max_rating = -1;
        PlaceData max_rating_data;

        while (fgets(line, sizeof(line), file) != NULL) {
            char *name = strtok(line, ",");
            char *rating_str = strtok(NULL, ",");

            if (rating_str == NULL) {
                fprintf(stderr, "Invalid file format\n");
                fclose(file);
                return 1;
            }

            float rating = atof(rating_str);

            if (rating > max_rating) {
                max_rating = rating;
                strcpy(max_rating_data.name, name);
                max_rating_data.rating = rating;
            }
        }

        fclose(file);

        printf("Type: %s\n", shared_data->filenames[i]);
        printf("filenames: %s\n", shared_data->filenames[i+1]);
        printf("----------------------\n");
        printf("Name: %s\n", max_rating_data.name);
        printf("Rating: %.1f\n\n", max_rating_data.rating);
        
    }

    if (shmdt(shared_data) == -1) {
        fprintf(stderr, "Failed to detach shared memory segment\n");
        return 1;
    }

    return 0;
}
```
- Pertama, kita menginisiasi struct untuk menyimpan data dari shared memory yang telah diunggah sebelumnya di program **auth.c** dan juga menyimpan nama dan rating tertinggi dari tiap file.

```c
typedef struct {
    char filenames[MAX_FILES][MAX_filenames_LENGTH];
    int count;
} SharedData;

typedef struct {
    char name[100];
    float rating;
} PlaceData;
```

- Kedua, kita membuat fungsi main untuk mengambil data dari shared memory, kemudian kita menginisiasi loop untuk mengecek tiap file di dalam folder `new-data`. Lalu di dalam for loop terdapat while loop yang akan mengecek baris tiap file dan menemukan skor tertingginya. Jika baris tersebut merupakan skor tertinggi, maka baris tersebut akan disimpan ke dalam struct. Jika looping telah selesai, program menampilkan output rating tertinggi ke terminal beserta nama dan tipe data file tersebut.

```c
int main() {
    key_t key = 1234;
    if (key == -1) {
        fprintf(stderr, "Failed to generate key\n");
        return 1;
    }

    int shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        fprintf(stderr, "Failed to create shared memory segment\n");
        return 1;
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        fprintf(stderr, "Failed to attach shared memory segment\n");
        return 1;
    }

    // start code
    for (int i = 0; i < shared_data->count; i += 2) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "../new-data/%s", shared_data->filenames[i+1]);

        FILE *file = fopen(fullpath, "r");
        if (!file) {
            fprintf(stderr, "Could not open file %s\n", fullpath);
        }

        char line[100];
        if (fgets(line, sizeof(line), file) == NULL) {
            fprintf(stderr, "Empty file or error reading header\n");
            fclose(file);
            return 1;
        }

        float max_rating = -1;
        PlaceData max_rating_data;

        while (fgets(line, sizeof(line), file) != NULL) {
            char *name = strtok(line, ",");
            char *rating_str = strtok(NULL, ",");

            if (rating_str == NULL) {
                fprintf(stderr, "Invalid file format\n");
                fclose(file);
                return 1;
            }

            float rating = atof(rating_str);

            if (rating > max_rating) {
                max_rating = rating;
                strcpy(max_rating_data.name, name);
                max_rating_data.rating = rating;
            }
        }

        fclose(file);

        printf("Type: %s\n", shared_data->filenames[i]);
        printf("filenames: %s\n", shared_data->filenames[i+1]);
        printf("----------------------\n");
        printf("Name: %s\n", max_rating_data.name);
        printf("Rating: %.1f\n\n", max_rating_data.rating);
        
    }

    if (shmdt(shared_data) == -1) {
        fprintf(stderr, "Failed to detach shared memory segment\n");
        return 1;
    }

    return 0;
}
```

Terakhir, kita membuat program bernama **db.c** untuk memindahkan file yang berada di folder `new-data` ke folder `database` dan menuliskannya ke file log.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <time.h>

#define SHARED_MEMORY_KEY 1234
#define MAX_filenames_LENGTH 512
#define MAX_FILES 10

typedef struct {
    char filenames[MAX_FILES][MAX_filenames_LENGTH];
    int count;
} SharedData;

void write_db_log(const char *type, const char *filename) {
    time_t now;
    struct tm *tm_info;
    char timestamp[20];
    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", tm_info);

    FILE *logFile = fopen("/home/irfanqs/modul3/soal_1/microservices/database/db.log", "a");
    if (logFile != NULL) {
        fprintf(logFile, "[%s] [%s] [%s]\n", timestamp, type, filename);
        fclose(logFile);
    }
}

void move_valid_file(const char *filename) {
    char command[512];
    snprintf(command, sizeof(command), "mv /home/irfanqs/modul3/soal_1/new-data/%s /home/irfanqs/modul3/soal_1/microservices/database/", filename);
    if (system(command) != 0) {
        fprintf(stderr, "Error moving file: %s\n", filename);
    } else {
        printf("File moved successfully: %s\n", filename);
    }
}

int main() {
    int shmid = shmget(SHARED_MEMORY_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *) -1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    for(int i = 1; i <= shared_data->count; i+=2) {
        if (strstr(shared_data->filenames[i], "trashcan")) {
            move_valid_file(shared_data->filenames[i]);
            write_db_log("Trash Can", shared_data->filenames[i]);
        } else if  (strstr(shared_data->filenames[i], "parkinglot")) {
            move_valid_file(shared_data->filenames[i]);
            write_db_log("Parking Lot", shared_data->filenames[i]);
        }
    }

    if (shmdt(shared_data) == -1) {
        perror("Error detaching shared memory");
        exit(EXIT_FAILURE);
    }
}
```

- Pertama, kita mendefinisikan struct untuk menyimpan data dari shared memory.
```c
typedef struct {
    char filenames[MAX_FILES][MAX_filenames_LENGTH];
    int count;
} SharedData;
```

- Kemudian dalam fungsi `write_db_log()`, program akan menuliskan setiap progress yang dilakukan ke dalam file `db.log`.

```c
void write_db_log(const char *type, const char *filename) {
    time_t now;
    struct tm *tm_info;
    char timestamp[20];
    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", tm_info);

    FILE *logFile = fopen("/home/irfanqs/modul3/soal_1/microservices/database/db.log", "a");
    if (logFile != NULL) {
        fprintf(logFile, "[%s] [%s] [%s]\n", timestamp, type, filename);
        fclose(logFile);
    }
}
```

- Lalu, dalam fungsi `move_valid_file()`, kita akan memindah sebuah file dari folder `new-data` ke `database` dengan `mv`.

```c
void move_valid_file(const char *filename) {
    char command[512];
    snprintf(command, sizeof(command), "mv /home/irfanqs/modul3/soal_1/new-data/%s /home/irfanqs/modul3/soal_1/microservices/database/", filename);
    if (system(command) != 0) {
        fprintf(stderr, "Error moving file: %s\n", filename);
    } else {
        printf("File moved successfully: %s\n", filename);
    }
}
```
- Selanjutnya dalam fungsi main, program mengambil data dari shared memory dan data tersebut digunakan untuk mengecek apakah file tersebut benar file yang valid atau tidak. Jika valid, program akan memanggil `move_valid_file()` dan `write_db_log()` untuk memindahkan file dan menuliskannya ke log.

```c
int main() {
    int shmid = shmget(SHARED_MEMORY_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    SharedData *shared_data = (SharedData *)shmat(shmid, NULL, 0);
    if (shared_data == (void *) -1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    for(int i = 1; i <= shared_data->count; i+=2) {
        if (strstr(shared_data->filenames[i], "trashcan")) {
            move_valid_file(shared_data->filenames[i]);
            write_db_log("Trash Can", shared_data->filenames[i]);
        } else if  (strstr(shared_data->filenames[i], "parkinglot")) {
            move_valid_file(shared_data->filenames[i]);
            write_db_log("Parking Lot", shared_data->filenames[i]);
        }
    }

    if (shmdt(shared_data) == -1) {
        perror("Error detaching shared memory");
        exit(EXIT_FAILURE);
    }
}
```

### Berikut merupakan cara kerja dari setiap program <br>
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/c9abe76e-4776-4aa7-8ba9-860ab545019d)
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/7db4c119-3f5c-432d-9fc6-0687a25c98bd)
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/26d670dd-aed4-4780-8a67-604a8fc9eb00)
<br> Berikut merupakan isi dari folder database dan juga isi dari file db.log <br>
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/1dfefce7-0b0a-4be5-a808-1f6a60d1dd06)

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
Shall Leglerg🥶 dan Carloss Signs 😎 adalah seorang pembalap F1 untuk tim Ferrari 🥵. Mobil F1 memiliki banyak pengaturan, seperti penghematan ERS, Fuel, Tire Wear dan lainnya. Pada minggu ini ada race di sirkuit Silverstone. Malangnya, seluruh tim Ferrari diracun oleh Super Max Max pada hari sabtu sehingga seluruh kru tim Ferrari tidak bisa membantu Shall Leglerg🥶 dan Carloss Signs 😎 dalam race. Namun, kru Ferrari telah menyiapkan program yang bisa membantu mereka dalam menyelesaikan race secara optimal. Program yang dibuat bisa mengatur pengaturan - pengaturan dalam mobil F1 yang digunakan dalam balapan. Programnya berketentuan sebagai berikut:

- Pada program actions.c, program akan berisi function function yang bisa di call oleh paddock.c
- Action berisikan sebagai berikut:
   - Gap [Jarak dengan driver di depan (float)]: Jika Jarak antara Driver dengan Musuh di depan adalah < 3.5s maka return Gogogo, jika jarak > 3.5s dan 10s return Push, dan jika jarak > 10s maka return Stay out of trouble.
   - Fuel [Sisa Bensin% (string/int/float)]: Jika bensin lebih dari 80% maka return Push Push Push, jika bensin di antara 50% dan 80% maka return You can go, dan jika bensin kurang dari 50% return Conserve Fuel.
   - Tire [Sisa Ban (int)]: Jika pemakaian ban lebih dari 80 maka return Go Push Go Push, jika pemakaian ban diantara 50 dan 80 return Good Tire Wear, jika pemakaian di antara 30 dan 50 return Conserve Your Tire, dan jika pemakaian ban kurang dari 30 maka return Box Box Box.
   - Tire Change [Tipe ban saat ini (string)]: Jika tipe ban adalah Soft return Mediums Ready, dan jika tipe ban Medium return Box for Softs.

		Contoh:
		[Driver] : [Fuel] [55%]
		[Paddock]: [You can go]

- Pada paddock.c program berjalan secara daemon di background, bisa terhubung dengan driver.c melalui socket RPC.
- Program paddock.c dapat call function yang berada di dalam actions.c.
- Program paddock.c tidak keluar/terminate saat terjadi error dan akan log semua percakapan antara paddock.c dan driver.c di dalam file race.log
  
Format log:
[Source] [DD/MM/YY hh:mm:ss]: [Command] [Additional-info]

ex :
[Driver] [07/04/2024 08:34:50]: [Fuel] [55%]
[Paddock] [07/04/2024 08:34:51]: [Fuel] [You can go]

- Program driver.c bisa terhubung dengan paddock.c dan bisa mengirimkan pesan dan menerima pesan serta menampilan pesan tersebut dari paddock.c sesuai dengan perintah atau function call yang diberikan.
- Jika bisa digunakan antar device/os (non local) akan diberi nilai tambahan.
- untuk mengaktifkan RPC call dari driver.c, bisa digunakan in-program CLI atau Argv (bebas) yang penting bisa send command seperti poin B dan menampilkan balasan dari paddock.c
</details>

### Penjelasan
Pertama, kita akan membuat file **actions.c** yang berfungsi untuk menyimpan segala fungsi yang dibutuhkan.

```c
#include <stdio.h>
#include <string.h>
#include "actions.h"

void gap(float distance, char *output) {
    if (distance < 3.5) sprintf(output, "Gogogo");
    else if (distance >= 3.5 && distance < 10) sprintf(output, "Push");
    else if (distance > 10) sprintf(output, "Stay out of trouble");
    else sprintf(output, "Invalid distance!");
}

void fuel(int fuel_percent, char *output) {
    if (fuel_percent > 80) sprintf(output, "Push Push Push");
    else if (fuel_percent >= 50 && fuel_percent <= 80) sprintf(output, "You can go");
    else if (fuel_percent < 50) sprintf(output, "Conserve Fuel");
}

void tire(int tire_usage, char *output) {
    if (tire_usage > 80) sprintf(output, "Go Push Go Push");
    else if (tire_usage >= 50 && tire_usage <= 80) sprintf(output, "Good Tire Wear");
    else if (tire_usage >= 30 && tire_usage < 50) sprintf(output, "Conserve Your Tire");
    else sprintf(output, "Box Box Box");
}

void tire_change(char* current_tire, char *output) {
    if (strcmp(current_tire, "Soft") == 0) sprintf(output, "Mediums Ready");
    else if (strcmp(current_tire, "Medium") == 0) sprintf(output, "Box for Softs");
    else sprintf(output, "Unknown tire type");
}
```

- Pertama, kita membuat fungsi `gap` yang menerima input berupa nilai distance dengan tipe data float. Lalu kita membuat fungsi if statement yang sesuai dengan soal.
```c
void gap(float distance, char *output) {
    if (distance < 3.5) sprintf(output, "Gogogo");
    else if (distance >= 3.5 && distance < 10) sprintf(output, "Push");
    else if (distance > 10) sprintf(output, "Stay out of trouble");
    else sprintf(output, "Invalid distance!");
}
```
- Selanjutnya, kita membuat fungsi `fuel` yang menerima input berupa nilai fuel_percent dengan tipe data int. Lalu kita membuat fungsi if statement yang sesuai dengan soal.
```c
void fuel(int fuel_percent, char *output) {
    if (fuel_percent > 80) sprintf(output, "Push Push Push");
    else if (fuel_percent >= 50 && fuel_percent <= 80) sprintf(output, "You can go");
    else if (fuel_percent < 50) sprintf(output, "Conserve Fuel");
}
```
- Selanjutnya, kita  membuat fungsi `tire` yang menerima input berupa nilai tire_usage dengan tipe data int. Lalu kita membuat fungsi if statement yang sesuai dengan soal.
```c
void tire(int tire_usage, char *output) {
    if (tire_usage > 80) sprintf(output, "Go Push Go Push");
    else if (tire_usage >= 50 && tire_usage <= 80) sprintf(output, "Good Tire Wear");
    else if (tire_usage >= 30 && tire_usage < 50) sprintf(output, "Conserve Your Tire");
    else sprintf(output, "Box Box Box");
}
```
- Terakhir, kita membuat fungsi  `tire_change` yang menerima input berupa nilai current_tire dengan tipe data string. Lalu kita membuat fungsi if statement yang sesuai dengan soal.
```c
void tire_change(char* current_tire, char *output) {
    if (strcmp(current_tire, "Soft") == 0) sprintf(output, "Mediums Ready");
    else if (strcmp(current_tire, "Medium") == 0) sprintf(output, "Box for Softs");
    else sprintf(output, "Unknown tire type");
}
```
- Agar kita dapat menjalankan fungsi ini di program lain, butuh yang namanya header file. Untuk itu kita perlu membuat file yang bernama **actions.h**.
```c
#ifndef ACTIONS_H
#define ACTIONS_H

void gap(float distance, char *output);
void fuel(int fuel_percent, char *output);
void tire(int tire_usage, char *output);
void tire_change(char* current_tire, char *output);

#endif

```
Selanjutnya, kita membuat file **padddock.c** yang berfungsi sebagai server.
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include "actions.h"

#define PORT 8080

void signal_handler(int signum) {
    if (signum == SIGINT) {
        // Lakukan apa pun yang diperlukan saat menerima sinyal SIGINT (misalnya, tutup socket dan keluar dari program)
        exit(EXIT_SUCCESS);
    }
}

void write_to_log_float(const char *client_info, const char *server_info, const char *input, float quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%.1f]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log(const char *client_info, const char *server_info, const char *input, int quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%d]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log_string(const char *client_info, const char *server_info, const char *input, const char *quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%s]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log_percent(const char *client_info, const char *server_info, const char *input, int quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%d%%]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log_error(const char *client_info, const char *server_info, const char *input, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s]\n", client_info, time_string, input);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

int main() {
    pid_t pid, sid;

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Atur signal handler untuk menangani SIGINT (Ctrl+C)
    signal(SIGINT, signal_handler);

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    // Tutup file descriptor standar
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Buka kembali stdin, stdout, dan stderr ke /dev/null
    int stdin_fd = open("/dev/null", O_RDWR);
    if (stdin_fd == -1) {
        perror("Failed to reopen stdin");
        exit(EXIT_FAILURE);
    }
    int stdout_fd = dup(stdin_fd);
    int stderr_fd = dup(stdin_fd);

    // Buka socket untuk server
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // START CODE
    float distance;
    int fuel_percent, tire_usage;
    char current_tire[20];
    char input[20];
    char output[100];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("172.23.143.99");
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Menerima data dari client (input)
        if (read(new_socket, input, sizeof(input)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }

        char output[100];
        if (strcmp(input, "Gap") == 0 || strcmp(input, "gap") == 0) {
            // Membaca string distance dari client
            char distance_str[20];
            if (read(new_socket, distance_str, sizeof(distance_str)) < 0) {
                perror("Read failed");
                exit(EXIT_FAILURE);
            }

            // Menambahkan karakter null pada akhir string
            distance_str[sizeof(distance_str) - 1] = '\0';

            // Mengonversi string distance ke float
            float distance;
            sscanf(distance_str, "%f", &distance);

            gap(distance, output);
            write_to_log_float("Driver", "Paddock", "Gap", distance, output);
            // Mengirimkan hasil ke client
            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }

        else if (strcmp(input, "Fuel") == 0 || strcmp(input, "fuel") == 0) {
            if (read(new_socket, &fuel_percent, sizeof(fuel_percent)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
            }

            fuel(fuel_percent, output);
            write_to_log_percent("Driver", "Paddock", "Fuel", fuel_percent, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        } 

        else if (strcmp(input, "Tire") == 0 || strcmp(input, "tire") == 0) {
            if (read(new_socket, &tire_usage, sizeof(tire_usage)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
            }

            tire(tire_usage, output);
            write_to_log("Driver", "Paddock", "Tire", tire_usage, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(input, "TireChange") == 0) {
            if (read(new_socket, current_tire, sizeof(current_tire)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
            }

            tire_change(current_tire, output);
            write_to_log_string("Driver", "Paddock", "Tire Change", current_tire, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        } else {
            sprintf(output, "Invalid input!");
            write_to_log_error("Driver", "Paddock", input, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
```
- Pertama, kita membuat fungsi `write_to_log()` untuk menuliskan segala proses interaksi antara client dan server (perbedaan dari tiap fungsi penulisan log adalah pada perbedaan tipe data pada value yang dimasukkan)
```c
void write_to_log_float(const char *client_info, const char *server_info, const char *input, float quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%.1f]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log(const char *client_info, const char *server_info, const char *input, int quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%d]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log_string(const char *client_info, const char *server_info, const char *input, const char *quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%s]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log_percent(const char *client_info, const char *server_info, const char *input, int quantity, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s] [%d%%]\n", client_info, time_string, input, quantity);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}

void write_to_log_error(const char *client_info, const char *server_info, const char *input, const char *output) {
    time_t current_time;
    char time_string[25];
    struct tm *timeinfo;

    time(&current_time);
    timeinfo = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%d/%m/%Y %H:%M:%S", timeinfo);

    FILE *fp = fopen("/home/irfanqs/modul3/soal_3/server/race.log", "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] [%s]: [%s]\n", client_info, time_string, input);
        fprintf(fp, "[%s] [%s]: [%s]\n", server_info, time_string, output);
        fclose(fp);
    } else {
        perror("Failed to open log file");
    }
}
```
- Masuk ke dalam fungsi main, kita akan membuat program berjalan dalam mode daemon. Lalu, kita membuka socket agar program server (paddock.c) dan client (client.c) dapat berinteraksi melalui socket tersebut. Setelah itu kita dapat menginisiasi variable-variable yang dibutuhkan beserta tipe datanya

```c
 pid_t pid, sid;

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Atur signal handler untuk menangani SIGINT (Ctrl+C)
    signal(SIGINT, signal_handler);

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    // Tutup file descriptor standar
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Buka kembali stdin, stdout, dan stderr ke /dev/null
    int stdin_fd = open("/dev/null", O_RDWR);
    if (stdin_fd == -1) {
        perror("Failed to reopen stdin");
        exit(EXIT_FAILURE);
    }
    int stdout_fd = dup(stdin_fd);
    int stderr_fd = dup(stdin_fd);

    // Buka socket untuk server
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // START CODE
    float distance;
    int fuel_percent, tire_usage;
    char current_tire[20];
    char input[20];
    char output[100];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("172.23.143.99");
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
```

- Masuk ke dalam while loop, program akan mendengarkan input yang dimasukkan oleh user pada program client. Input tersebut akan dipilah menggunakan if statement. Setiap input akan menjalankan fungsi yang diberikan sesuai dengan if statementnya. Setelah fungsi dijalankan, program akan mengeluarkan output yang nantinya akan dikirimkan ke client. Program **paddock.c** akan berhenti bila user membunuh program ini menggunakan `pkill`

```c
 while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Menerima data dari client (input)
        if (read(new_socket, input, sizeof(input)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }

        char output[100];
        if (strcmp(input, "Gap") == 0 || strcmp(input, "gap") == 0) {
            // Membaca string distance dari client
            char distance_str[20];
            if (read(new_socket, distance_str, sizeof(distance_str)) < 0) {
                perror("Read failed");
                exit(EXIT_FAILURE);
            }

            // Menambahkan karakter null pada akhir string
            distance_str[sizeof(distance_str) - 1] = '\0';

            // Mengonversi string distance ke float
            float distance;
            sscanf(distance_str, "%f", &distance);

            gap(distance, output);
            write_to_log_float("Driver", "Paddock", "Gap", distance, output);
            // Mengirimkan hasil ke client
            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }

        else if (strcmp(input, "Fuel") == 0 || strcmp(input, "fuel") == 0) {
            if (read(new_socket, &fuel_percent, sizeof(fuel_percent)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
            }

            fuel(fuel_percent, output);
            write_to_log_percent("Driver", "Paddock", "Fuel", fuel_percent, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        } 

        else if (strcmp(input, "Tire") == 0 || strcmp(input, "tire") == 0) {
            if (read(new_socket, &tire_usage, sizeof(tire_usage)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
            }

            tire(tire_usage, output);
            write_to_log("Driver", "Paddock", "Tire", tire_usage, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(input, "TireChange") == 0) {
            if (read(new_socket, current_tire, sizeof(current_tire)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
            }

            tire_change(current_tire, output);
            write_to_log_string("Driver", "Paddock", "Tire Change", current_tire, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        } else {
            sprintf(output, "Invalid input!");
            write_to_log_error("Driver", "Paddock", input, output);

            if (write(new_socket, output, sizeof(output)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        close(new_socket);
    }

    close(server_fd);
    return 0;
```

Selanjutnya, kita akan membuat file **driver.c** yang berfungsi sebagai client.
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../server/actions.h"
#include <string.h>

#define PORT 8080
#define IP "127.0.0.1"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    float distance;
    int fuel_percent, tire_usage;
    char current_tire[20];
    char fuel_string[10];
    char input[20];
    char output[100];

    while (1) {
        // Buka socket untuk server
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }

        // Coba melakukan koneksi ke server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        // Meminta input dari pengguna
        printf("Command: ");
        scanf("%s", input);

        if (strcmp(input, "Gap") == 0 || strcmp(input, "gap") == 0) {
            printf("Info: ");
            scanf("%f", &distance);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            // Mengonversi float ke string
            char distance_str[20];
            sprintf(distance_str, "%f", distance);

            // Mengirimkan string distance ke server
            if (write(sock, distance_str, sizeof(distance_str)) < 0) {
                perror("Write failed");
                exit(EXIT_FAILURE);
            }

        }
        else if (strcmp(input, "Fuel") == 0 || strcmp(input, "fuel") == 0) {
            printf("Info: ");
            scanf("%s", fuel_string);
            sscanf(fuel_string, "%d%%", &fuel_percent);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            if (write(sock, &fuel_percent, sizeof(fuel_percent)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(input, "Tire") == 0 || strcmp(input, "tire") == 0) {
            printf("Info: ");
            scanf("%d", &tire_usage);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            if (write(sock, &tire_usage, sizeof(tire_usage)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(input, "TireChange") == 0) {
            printf("Info: ");
            scanf("%s", current_tire);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            if (write(sock, current_tire, sizeof(current_tire)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else {
            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        // Menerima hasil dari server
        if (read(sock, output, sizeof(output)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }

        printf("[Paddock]: %s\n", output);

        close(sock);
    }

    return 0;
}
```
- Pertama, kita tentukan dulu port dan ip yang akan digunakan untuk menghubungkan dengan server di pc lokal maupun nonlokal
```c
#define PORT 8080
#define IP "127.0.0.1"
```

- Kemudian masuk ke dalam fungsi main, kita menginisiasi variable yang akan digunakan beserta tipe datanya
```c
int sock = 0;
struct sockaddr_in serv_addr;
float distance;
int fuel_percent, tire_usage;
char current_tire[20];
char fuel_string[10];
char input[20];
char output[100];
```
- Masuk ke dalam while loop, program akan mencoba menyambungkan antara server dan client melalui socket. Lalu program akan menerima input yang diinputkan oleh server. Inputan user akan dipilah sesuai dengan fungsi yang akan dijalankan nantinya. Input user juga akan dikirimkan kepada server dengan memanfaatkan socket yang telah disambungakan. Program akan menerima output dari server dan akan dimunculkan di terminal.
```c
   while (1) {
        // Buka socket untuk server
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }

        // Coba melakukan koneksi ke server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        // Meminta input dari pengguna
        printf("Command: ");
        scanf("%s", input);

        if (strcmp(input, "Gap") == 0 || strcmp(input, "gap") == 0) {
            printf("Info: ");
            scanf("%f", &distance);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            // Mengonversi float ke string
            char distance_str[20];
            sprintf(distance_str, "%f", distance);

            // Mengirimkan string distance ke server
            if (write(sock, distance_str, sizeof(distance_str)) < 0) {
                perror("Write failed");
                exit(EXIT_FAILURE);
            }

        }
        else if (strcmp(input, "Fuel") == 0 || strcmp(input, "fuel") == 0) {
            printf("Info: ");
            scanf("%s", fuel_string);
            sscanf(fuel_string, "%d%%", &fuel_percent);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            if (write(sock, &fuel_percent, sizeof(fuel_percent)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(input, "Tire") == 0 || strcmp(input, "tire") == 0) {
            printf("Info: ");
            scanf("%d", &tire_usage);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            if (write(sock, &tire_usage, sizeof(tire_usage)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else if (strcmp(input, "TireChange") == 0) {
            printf("Info: ");
            scanf("%s", current_tire);

            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
            if (write(sock, current_tire, sizeof(current_tire)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        else {
            if (write(sock, input, sizeof(input)) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
            }
        }
        // Menerima hasil dari server
        if (read(sock, output, sizeof(output)) < 0) {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }

        printf("[Paddock]: %s\n", output);

        close(sock);
    }
```

### Berikut merupakan demo dari file actions.c, driver.c, dan paddock.c

Client dan server terhubung dengan RPC non local dengan memanfaatkan ip dari vm linux
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/cbb0758e-8943-4922-a4e4-d32060775106)

<br> Berikut merupakan isi dari file race.log (start dari nomor 31) <br>
![image](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/130438307/6e1b6773-c1ac-41b2-bf32-d7fc432ed9f2)

### Kendala
Tidak ada kendala pada soal ini

### Revisi
Tidak ada revisi pada soal ini

## Soal 4
<details><summary>Klik untuk melihat soal</summary>Lewis Hamilton 🏎 seorang wibu akut dan sering melewatkan beberapa episode yang karena sibuk menjadi asisten. Maka dari itu dia membuat list anime yang sedang ongoing (biar tidak lupa) dan yang completed (anime lama tapi pengen ditonton aja). Tapi setelah Lewis pikir-pikir malah kepikiran untuk membuat list anime. Jadi dia membuat file (harap diunduh) dan ingin menggunakan socket yang baru saja dipelajarinya untuk melakukan CRUD pada list animenya. 
Client dan server terhubung melalui socket. 
client.c di dalam folder client dan server.c di dalam folder server
Client berfungsi sebagai pengirim pesan dan dapat menerima pesan dari server.
Server berfungsi sebagai penerima pesan dari client dan hanya menampilkan pesan perintah client saja.  
Server digunakan untuk membaca myanimelist.csv. Dimana terjadi pengiriman data antara client ke server dan server ke client.
- Menampilkan seluruh judul
- Menampilkan judul berdasarkan genre
- Menampilkan judul berdasarkan hari
- Menampilkan status berdasarkan berdasarkan judul
- Menambahkan anime ke dalam file myanimelist.csv
- Melakukan edit anime berdasarkan judul
- Melakukan delete berdasarkan judul
Selain command yang diberikan akan menampilkan tulisan “Invalid Command”
Karena Lewis juga ingin track anime yang ditambah, diubah, dan dihapus. Maka dia membuat server dapat mencatat anime yang dihapus dalam sebuah log yang diberi nama change.log.
Format: [date] [type] [massage]
Type: ADD, EDIT, DEL
Ex:
[29/03/24] [ADD] Kanokari ditambahkan.
[29/03/24] [EDIT] Kamis,Comedy,Kanokari,completed diubah menjadi Jumat,Action,Naruto,completed.
[29/03/24] [DEL] Naruto berhasil dihapus.

Koneksi antara client dan server tidak akan terputus jika ada kesalahan input dari client, cuma terputus jika user mengirim pesan “exit”. Program exit dilakukan pada sisi client.
Hasil akhir:
soal_4/
    ├── change.log
    ├── client/
    │   └── client.c
    ├── myanimelist.csv
    └── server/
        └── server.c


</details>

### Penjelasan
### 1. buat **server.c**
berikut isi dan penjelasan dari file *server.c*

- berikut header filesnya:
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
```

- `maks_client` mendifinisikan jumlah maksimum client yang dapat di tangani, sedangkan size_buffer merupakan ukuran buffer untuk membaca data.
```c
#define maks_clients 10
#define size_buffer 1024
```
- Fungsi send_response digunakan untuk mengirim tanggapan ke klien. Ini membungkus pesan dalam format yang benar dan mengirimkannya ke socket klien.
```c
void send_response(int client_socket, const char *message) {
    char response[size_buffer];     // Buffer untuk menyimpan pesan yang akan dikirim
    snprintf(response, size_buffer, "%s\n", message); // Memformat pesan dengan newline
    send(client_socket, response, strlen(response), 0); // Mengirim pesan yang diformat melalui socket
}
```
- Fungsi `log_change` digunakan untuk mencatat perubahan dalam file log, dengan memformat pesan log dengan tanggal dan waktu, jenis perubahan, dan detailnya, lalu menuliskannya ke file log.
```c
void log_change(FILE *log_file, const char *type, const char *detail) {
    time_t now;                     // untuk menyimpan waktu saat ini
    struct tm *tm_info;             // untuk menyimpan waktu lokal
    char timestamp[20];             // string untuk menyimpan tanggal yang diformat
    time(&now);                    
    tm_info = localtime(&now);      // mengonversi ke waktu lokal
    strftime(timestamp, sizeof(timestamp), "%d/%m/%y", tm_info); // memformat tanggal menjadi DD/MM/YY
    char log_entry[size_buffer];    // Buffer untuk menyimpan entri log
    sprintf(log_entry, "[%s] [%s] %s\n", timestamp, type, detail); // memformat entri log
    log_entry[strcspn(log_entry, "\n")] = ' '; // menghapus karakter newline
    fputs(log_entry, log_file);     // menulis entri log ke file log.
    fflush(log_file);               // membersihkan stream file.
}
```
- Fungsi `handle_client` dipanggil untuk menangani koneksi dari client. dia ngrbaca input dari client, memprosesnya, dan memberikan tanggapan sesuai.
```c
void handle_client(int client_socket, FILE *anime_file, FILE *log_file) {
    char buffer[size_buffer];       // buffer untuk menyimpan data yang dibaca dari socket
    int read_bytes;                 // variabel untuk menyimpan jumlah byte yang dibaca
```
- Loop untuk membaca data dari socket klien dan memasukkannya ke dalam buffer. Jika pesan dari klien terakhirnya adalah newline, bakal dihapus karena tidak diperlukan dalam pemrosesan
```c
while ((read_bytes = read(client_socket, buffer, size_buffer - 1)) > 0) {
        buffer[read_bytes] = '\0'; 

        printf("Received input from client: %s\n", buffer); // mencetak input dari client
        if (buffer[read_bytes - 1] == '\n') // memeriksa jika karakter terakhir adalah newline
            buffer[read_bytes - 1] = '\0';  // menghapus newline
```

- Buffer ada dua bagian: command dan argumen. Ini dilakukan dengan menggunakan fungsi strtok untuk memisahkan string berdasarkan spasi dan newline
```c
        char *command = strtok(buffer, " "); // memisahkan perintah dari buffer
        char *argument = strtok(NULL, "\n"); // memisahkan argumen dari buffer
```

- Jika tidak ada command yang diberikan oleh client, server akan mengirim tanggapan "Invalid Command" dan melanjutkan loop.
```c
if (command == NULL) {              // jika perintah .
            send_response(client_socket, "Invalid Command\n"); // mengirimkan respons 'Invalid Command'.
            continue;                       // melanjutkan ke iterasi berikutnya.
        }
```
- Jika perintah yang diterima adalah "exit", server akan mengirim pesan "Connection closed" ke client dan menutup koneksi
```c
        if (strcmp(command, "exit") == 0) {
            printf("Exit command received. Closing connection.\n");
            send_response(client_socket, "Connection closed.\n");
            break;       // Keluar dari loop untuk menutup koneksi.
        }
```
- Jika perintah yang diterima adalah "tampilkan", server akan membaca file anime dan mengirimkan daftar judul anime ke client
```c
        if (strcmp(command, "tampilkan") == 0) {
            char response[size_buffer] = ""; 
            char line[size_buffer];         
            int count = 1;                 
            rewind(anime_file);              // mengembalikan pointer file ke awal file.
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *token = strtok(line, ","); // memisahkan baris dengan koma
                token = strtok(NULL, ",");       // lewati token pertama
                token = strtok(NULL, ",");       // dapatkan token ketiga (judul)
                token[strcspn(token, "\n")] = '\0'; // menghapus karakter newline
                char formatted_line[size_buffer];
                sprintf(formatted_line, "%2d. %s\n", count++, token); // memformat baris
                strcat(response, formatted_line); // menambahkan baris yang diformat ke respons
            }
            send_response(client_socket, response); // Mengirim respons ke client
```
- Jika perintah yang diterima adalah "hari", server akan mencari anime yang ditayangkan pada hari yang diinginkan dan mengirimkan daftar anime tersebut ke client.
```c
        } else if (strcmp(command, "hari") == 0) {
            if (argument == NULL) {                 // Jika argumen NULL.
                send_response(client_socket, "Invalid Command\n"); // maka akan mengirimkan respons 'Invalid Command'.
                continue;                           // melanjutkan ke iterasi berikutnya
            }
            char response[size_buffer] = "";        // Buffer untuk menyimpan respons
            char line[size_buffer];                 // Buffer untuk menyimpan baris yang dibaca dari file.
            int count = 1;                          // Penghitung untuk daftar anime
            rewind(anime_file);                     // Mengembalikan pointer file
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *token = strtok(line, ",");    // Memisahkan baris dengan koma
                if (strcmp(token, argument) == 0) { // Membandingkan hari dengan argumen
                    token = strtok(NULL, ",");      // Lewati token kedua
                    token = strtok(NULL, ",");      // Dapatkan token ketiga (judul)
                    token[strcspn(token, "\n")] = '\0'; // Menghapus karakter newline
                    char formatted_line[size_buffer];
                    sprintf(formatted_line, "%2d. %s\n", count++, token); 
                    strcat(response, formatted_line); // Menambahkan baris yang diformat ke respons
                }
            }
            if (strlen(response) == 0) {            // Jika respons kosong
                send_response(client_socket, "No anime found for the specified day\n"); // Mengirimkan respons 'No anime found for the specified day'.
            } else {
                send_response(client_socket, response); // Mengirim respons ke client.
            }
```
- Jika perintah yang diterima adalah "genre", server akan mencari anime yang memiliki genre yang diinginkan dan mengirimkan daftar anime tersebut ke klien.
```c
 } else if (strcmp(command, "genre") == 0) {
            if (argument == NULL) {                 // Jika argumen null.
                send_response(client_socket, "Invalid Command\n"); // Mengirimkan respons 'Invalid Command'
                continue;                           
            }
            char response[size_buffer] = "";       
            char line[size_buffer];                 
            int count = 1;                          // Penghitung untuk daftar anime
            rewind(anime_file);                     
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *token = strtok(line, ",");  
                token = strtok(NULL, ",");          // Dapatkan token kedua (genre)
                if (strcmp(token, argument) == 0) { // Membandingkan genre dengan argumen
                    token = strtok(NULL, ",");      // Dapatkan token ketiga (judul)
                    token[strcspn(token, "\n")] = '\0'; // Menghapus karakter newline
                    char formatted_line[size_buffer];
                    sprintf(formatted_line, "%2d. %s\n", count++, token); // Memformat baris
                    strcat(response, formatted_line); // Menambahkan baris yang diformat ke respons
                }
            }
            if (strlen(response) == 0) {            // Jika respons kosong.
                send_response(client_socket, "No anime found for the specified genre\n"); // Mengirimkan respons 'No anime found for the specified genre'
            } else {
                send_response(client_socket, response); // Mengirim respons ke klien
            }
```
- Jika perintah yang diterima adalah "status", server akan mencari status dari anime yang dimaksud dan mengirimkan status tersebut ke client
```c
   } else if (strcmp(command, "status") == 0) {
            if (argument == NULL) {                 // Jika argumen null
                send_response(client_socket, "Invalid Command\n"); // Mengirimkan respons 'Invalid Command'
                continue;                           // Melanjutkan ke iterasi berikutnya.
            }
            char response[size_buffer] = "";        // Buffer untuk menyimpan respons
            char line[size_buffer];                 // Buffer untuk menyimpan baris yang dibaca dari file
            rewind(anime_file);                     // Mengembalikan pointer file ke awal file
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *day = strtok(line, ",");      // Memisahkan baris dengan koma (hari)
                char *genre = strtok(NULL, ",");    // Dapatkan token kedua (genre)
                char *title = strtok(NULL, ",");    // Dapatkan token ketiga (judul)
                char *status = strtok(NULL, ",");   // Dapatkan token keempat (status)
                
                while (strtok(NULL, ",") != NULL) {
                    strcat(title, strtok(NULL, ",")); // Menambahkan token berikutnya ke judul jika ada koma di dalamnya
                }

                char *clean_title = strtok(title, "\n");
                for (int i = 0; clean_title[i]; i++) {
                    clean_title[i] = tolower(clean_title[i]); // Mengubah judul menjadi huruf kecil untuk perbandingan
                }
                for (int i = 0; argument[i]; i++) {
                    argument[i] = tolower(argument[i]); // Mengubah argumen menjadi huruf kecil untuk perbandingan
                }
                if (strcasecmp(clean_title, argument) == 0) { // Perbandingan tidak peka huruf besar/kecil antara judul dan argumen
                    sprintf(response, "%s\n", status); // Memformat status
                    break; // Keluar dari loop karena status ditemukan
                }
            }
            if (strlen(response) == 0) {            // Jika respons kosong.
                send_response(client_socket, "Anime not foundn\n"); // Mengirimkan respons 'Anime not found'.
            } else {
                send_response(client_socket, response); // Mengirim respons ke klien.
            }
```
- Jika perintah yang diterima adalah "add", server akan menambahkan anime baru ke dalam file anime dan mencatat perubahan tersebut ke dalam file log.
```c
  } else if (strcmp(command, "add") == 0) {
            if (argument == NULL) {                 // Jika argumen null
                send_response(client_socket, "Invalid Command\n"); // Mengirimkan respons 'Invalid Command'
                continue;                         
            }
            char *day = strtok(argument, ",");     
            char *genre = strtok(NULL, ",");     
            char *title = strtok(NULL, ",");       
            char *status = strtok(NULL, ","); 

            fprintf(anime_file, "\n%s,%s,%s,%s", day, genre, title, status); // Menulis anime baru ke file.
            fflush(anime_file);                   // Membersihkan stream file
            char log_entry[size_buffer];
            snprintf(log_entry, size_buffer, "ADD: %s, %s, %s, %s", day, genre, title, status); // Memformat entri log.
            log_change(log_file, "ADD", log_entry); // Mencatat penambahan

            send_response(client_socket, "Anime successfully added.\n"); // Mengirimkan respons 'Anime successfully added.'.
```
- Jika perintah yang diterima adalah "edit", server akan mengedit informasi anime yang sudah ada di file anime dan mencatat perubahan ke dalam file log.
```c
   } else if (strcmp(command, "edit") == 0) {
            if (argument == NULL) {                 // Jika argumen null
                send_response(client_socket, "Invalid Command\n"); // Mengirimkan respons 'Invalid Command'
                continue;                           // Melanjutkan ke iterasi berikutnya.
            }
//Dapatkan token
            char *anime_to_edit = strtok(argument, ","); 
            char *new_day = strtok(NULL, ",");         
            char *new_genre = strtok(NULL, ",");      
            char *new_title = strtok(NULL, ",");        
            char *new_status = strtok(NULL, ",");

            FILE *temp_file = fopen("/home/winds/soal_4/temp.csv", "w"); // Membuka file sementara untuk pengeditan
            if (temp_file == NULL) {
                perror("Failed to create temporary file "); // Mencetak kesalahan ketika pembukaan file gagal.
                send_response(client_socket, "Failed to edit anime.\n"); // Mengirimkan respons 'Failed to edit anime'.
                continue; // Melanjutkan ke iterasi berikutnya.
            }
            rewind(anime_file);                        // Mengembalikan pointer file ke awal file
            char line[size_buffer];                    // Buffer untuk menyimpan baris yang dibaca dari file
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *day = strtok(line, ",");         // Memisahkan baris dengan koma (hari)
                char *genre = strtok(NULL, ",");       // Dapatkan token kedua (genre)
                char *title = strtok(NULL, ",");       // Dapatkan token ketiga (judul)
                char *status = strtok(NULL, ",");      // Dapatkan token keempat (status)

                while (strtok(NULL, ",") != NULL) {
                    strcat(title, strtok(NULL, ","));  // Menambahkan token berikutnya ke judul jika ada koma di dalamnya
                }
                char *clean_title = strtok(title, "\n"); // Menghapus karakter newline dari judul
                if (strcasecmp(clean_title, anime_to_edit) == 0) { // Perbandingan tidak peka huruf besar/kecil antara judul dan argumen
                    fprintf(temp_file, "%s,%s,%s,%s\n", new_day, new_genre, new_title, new_status); // Menulis detail baru ke file sementara
                } else {
                    fprintf(temp_file, "%s,%s,%s,%s", day, genre, title, status); // Menulis detail lama ke file sementara.
                }
            }

            fclose(anime_file);                        // Menutup file anime.
            fclose(temp_file);                         // Menutup file sementara.
            if (rename("/home/winds/soal_4/temp.csv", "/home/winds/soal_4/myanimelist.csv") != 0) {
                perror("Failed to rename file");   // Mencetak kesalahan jika penggantian nama file gagal.
                send_response(client_socket, "Failed to edit anime\n");
                continue;
            }

            char log_entry[size_buffer];
            snprintf(log_entry, size_buffer, "EDIT: %s, %s, %s, %s, %s", anime_to_edit, new_day, new_genre, new_title, new_status); // Memformat entri log
            log_change(log_file, "EDIT", log_entry); // Mencatat pengeditan
            send_response(client_socket, "Anime successfully edited.\n"); // Mengirimkan respons 'Anime successfully edited'
            anime_file = fopen("/home/winds/soal_4/myanimelist.csv", "r+"); // Membuka kembali file anime
            if (anime_file == NULL) {
                perror("Failed to reopen anime list file"); // Mencetak kesalahan jika pembukaan kembali file gagal
                return; 
            }
```
- Jika perintah yang diterima adalah "delete", server akan menghapus anime yang dari file anime dan mencatat perubahan tersebut ke dalam file log.
```c
 } else if (strcmp(command, "delete") == 0) {
            if (argument == NULL) {                 // Jika argumen null
                send_response(client_socket, "Invalid Command\n"); // Mengirimkan respons 'Invalid Command'.
                continue;                           
            }
            FILE *temp_file = fopen("/home/winds/soal_4/temp.csv", "w"); // Membuka file sementara untuk penghapusan.
            if (temp_file == NULL) {
                perror("Gagal membuat file sementara");
                send_response(client_socket, "Gagal menghapus anime.\n"); 
                continue; 
            }
            rewind(anime_file);                        // mengembalikan pointer file ke awal file
            char line[size_buffer];                    // buffer untuk menyimpan baris yang dibaca dari file
            int deleted = 0;                           
            while (fgets(line, size_buffer, anime_file) != NULL) {
                char *day = strtok(line, ",");       
                char *genre = strtok(NULL, ",");      
                char *title = strtok(NULL, ",");       
                char *status = strtok(NULL, ",");     
               
                while (strtok(NULL, ",") != NULL) {
                    strcat(title, strtok(NULL, ","));  // Menambahkan token berikutnya ke judul jika ada koma di dalamnya.
                }
    
                char *clean_title = strtok(title, "\n"); // Menghapus karakter newline dari judul.
                if (strcasecmp(clean_title, argument) == 0) { // Perbandingan tidak peka huruf besar/kecil antara judul dan argumen.
                    deleted = 1;                        // Menetapkan penanda penghapusan
                    continue;                           
                }

                fprintf(temp_file, "%s,%s,%s,%s", day, genre, title, status); // Menulis baris ke file sementara
            

            fclose(temp_file);                         // Menutup temp_file sementara
            fclose(anime_file);                        // Menutup file anime
            if (remove("/home/winds/soal_4/myanimelist.csv") != 0) {
                perror("Gagal menghapus file");       // Mencetak kesalahan jika penghapusan file gagal.
                send_response(client_socket, "Gagal menghapus anime.\n"); // Mengirimkan respons 'Gagal menghapus anime'
                continue;
            }
            if (rename("/home/winds/soal_4/temp.csv", "/home/winds/soal_4/myanimelist.csv") != 0) {
                perror("Gagal mengganti nama file"); 
                send_response(client_socket, "Gagal menghapus anime.\n"); // Mengirimkan respons 'Gagal menghapus anime'
                continue; 
            }
            if (deleted) {                           
                char log_entry[size_buffer];
                snprintf(log_entry, size_buffer, "DELETE: %s", argument); // Memformat entri log
                log_change(log_file, "DELETE", log_entry); // Mencatat penghapusan
                send_response(client_socket, "Anime berhasil dihapus.\n"); // Mengirimkan respons 'Anime berhasil dihapus'
            } else {
                send_response(client_socket, "Anime tidak ditemukan.\n"); .
            }
            anime_file = fopen("/home/winds/soal_4/myanimelist.csv", "r+"); // Membuka kembali file anime
            if (anime_file == NULL) {
                perror("Gagal membuka kembali file daftar anime"); // Mencetak kesalahan jika pembukaan kembali file gagal
                return;
            }
        } else {
            send_response(client_socket, "Invalid Command\n"); // Mengirimkan respons 'Invalid Command' untuk perintah yang tidak dikenal.
        }

```
```c
  memset(buffer, 0, size_buffer); // Membersihkan buffer untuk pembacaan berikutnya
    }

    if (read_bytes == 0) {
        printf("Client closed the connection.\n"); // Mencetak pesan di server untuk penutupan koneksi oleh klien
    } else {
        perror("read failed"); // Mencetak kesalahan jika pembacaan gagal
    }

    close(client_socket); // Menutup socket klien
}
```
- Fungsi `main()` membuka file daftar anime dan file log, membuat dan mengikat socket server. Kemudian, dalam loop utama, server mendengarkan koneksi masuk dari client. Setiap kali koneksi diterima, fungsi handle_client() dipanggil untuk menangani permintaan client. Setelah menangani permintaan, server menutup soket client, file anime, file log, dan socket server sebelum kembali dan mengindikasikan kesuksesan dengan kode keluar 0.
```c
int main() {
    int server_socket, client_socket; // Variabel untuk socket server dan client
    int opt = 1;                      // Opsi untuk setsockopt
    struct sockaddr_in server_address, client_address; //  untuk menyimpan informasi alamat server dan klien.
    socklen_t client_len;             // Variabel untuk menyimpan ukuran alamat klien

    system("wget 'https://drive.google.com/uc?export=download&id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50' -O /home/winds/soal_4/myanimelist.csv"); // untuk mengunduh file daftar anime dari Google Drive

    FILE *anime_file = fopen("/home/winds/soal_4/myanimelist.csv", "r+"); // Membuka file daftar anime untuk dibaca dan ditulis.
    if (anime_file == NULL) {
        perror("Failed to open anime list file"); // Mencetak kesalahan jika pembukaan file gagal.
        return 1; 
    }

    FILE *log_file = fopen("/home/winds/soal_4/change.log", "a+"); // Membuka file log untuk penambahan
    if (log_file == NULL) {
        // jika file log tidak ada, membuat file log baru
        FILE *newlog_file = fopen("/home/winds/soal_4/change.log", "w"); // Membuat file log baru
        if (newlog_file == NULL) {
            perror("Failed to create log file"); // Mencetak kesalahan jika pembuatan file gagal
            fclose(anime_file); // Menutup file anime
            return 1;
        }
        fclose(newlog_file); // Menutup file log baru
        log_file = fopen("/home/winds/soal_4/change.log", "a+"); // Membuka file log untuk penambahan
        if (log_file == NULL) {
            perror("Failed to open log file"); 
            fclose(anime_file); // Menutup file anime
            return 1; 
        }
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0); // Membuat socket server.
    if (server_socket < 0) {
        perror("Failed to create socket");
        fclose(anime_file); 
        fclose(log_file); 
        return 1; 
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed"); // Mencetak kesalahan jika pengaturan opsi socket gagal.
        exit(EXIT_FAILURE); 
    }
    server_address.sin_family = AF_INET; // Menetapkan alamat family server ke IPv4
    server_address.sin_addr.s_addr = INADDR_ANY; // Menetapkan alamat IP server untuk menerima koneksi masuk apa pun
    server_address.sin_port = htons(8080); // Menetapkan port server ke 8080

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket"); // Mencetak kesalahan jika pengikatan socket gagal
        close(server_socket); // Menutup socket server
        fclose(anime_file); 
        fclose(log_file); 
        return 1; 
    }

    if (listen(server_socket, maks_clients) < 0) {
        perror("Failed to listen on socket"); // Mencetak kesalahan jika mendengarkan di socket gagal.
        close(server_socket);
        fclose(anime_file); 
        fclose(log_file); 
        return 1; 
    }

    printf("Server started. Listening on port 8080...\n"); // Mencetak pesan bahwa server telah dimulai.

    while (1) {
        client_len = sizeof(client_address); // Menetapkan ukuran alamat klien
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len); // Menerima koneksi klien masuk
        if (client_socket < 0) {
            perror("Failed to accept client"); // Mencetak kesalahan jika penerimaan klien gagal
            continue; 
        }

        handle_client(client_socket, anime_file, log_file); // Menangani permintaan klien
        break; // Keluar dari loop 
    }

    fclose(anime_file); // Menutup file anime
    fclose(log_file); // Menutup file log
    close(server_socket); // Menutup socket server

    return 0; 
}
```
untuk menjalankan file ini kita perlu command `gcc server.c -o server; ./server`

### 2. Membuat **Client.c**
Program ini merupakan client yang terhubung ke server, mengirimkan perintah yang dimasukkan oleh pengguna, dan menampilkan respons dari server.
```c
#include <stdio.h>          
#include <stdlib.h>         
#include <string.h>        
#include <unistd.h>         
#include <sys/socket.h>     
#include <arpa/inet.h>      

#define size_buffer 1024    //ukuran buffer untuk data yang akan dikirim dan diterima

int main() {                
    int client_socket;      // Deklarasi variabel untuk socket klien
    struct sockaddr_in server_address;  // Deklarasi struktur untuk menyimpan alamat server
    char buffer[size_buffer];           // Deklarasi buffer untuk menyimpan data yang akan dikirim
    char response[2048];                // Deklarasi buffer untuk menyimpan respons dari server

    //Membuat Socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (client_socket < 0) {                         // Memeriksa apakah pembuatan socket gagal
        perror("Error creating socket");              
        return 1;                                     
    }

    server_address.sin_family = AF_INET;  // Menetapkan tipe alamat IP yang digunakan
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");  // Mengatur alamat IP server (localhost)
    server_address.sin_port = htons(8080);                     // Mengatur port server (8080)

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // Menghubungkan ke server
        perror("Error connecting to server");  // Menampilkan pesan error jika koneksi gagal
        return 1;                              
    }

    printf("Connected to server. Enter commands ('exit' to quit):\n");  // Memberitahu bahwa koneksi berhasil

    while (1) {  // Loop utama untuk mengirim dan menerima data

        printf("\nYou> ");   // Muntuk memasukkan perintah
        fgets(buffer, size_buffer, stdin);  // Membaca input dari pengguna
        buffer[strcspn(buffer, "\n")] = '\0';  // Menghilangkan karakter newline di akhir input

        send(client_socket, buffer, strlen(buffer), 0);  // Mengirim data ke server

        if (strcmp(buffer, "exit") == 0) {  // Memeriksa jika pengguna ingin keluar
            break;  // Keluar dari loop
        }

        int bytes_received = recv(client_socket, response, sizeof(response) - 1, 0);  // Menerima respons dari server
        if (bytes_received < 0) {  // Jika ada error saat menerima data
            perror("recv failed");  // Menampilkan pesan error
            break;  // Keluar dari loop
        } else if (bytes_received == 0) {  // Jika server menutup koneksi
            printf("Server closed the connection.\n");  
            break;  // Keluar dari loop
        }

        response[bytes_received] = '\0';  // Menambahkan null terminator di akhir string

        printf("Server:\n");  // Memberitahu bahwa ini adalah respons dari server
        char *token = strtok(response, "\n"); 
        while (token != NULL) {
            printf("%s\n", token);  // Menampilkan setiap bagian respons
            token = strtok(NULL, "\n");  
        }
        printf("\n");
        if (strstr(response, "Connection closed.") != NULL) {  // Jika respons menyatakan bahwa koneksi ditutup
            break;  // Keluar dari loop
        }
    }
    close(client_socket);  // Menutup soket
    return 0; 
}
```
untuk menjalankan file ini kita perlu command `gcc client.c -o client; ./client`

### Dokumentasi
*pada terminal server*
![Screenshot from 2024-05-11 20-56-54](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/79549192/99643f23-f45c-4919-bf70-14aa4d4d2417)

*pada terminal client*
![Screenshot from 2024-05-11 20-57-35](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/79549192/8dacd540-ecf5-44f1-8ec0-37d26af9e6b6)
![Screenshot from 2024-05-11 20-58-07](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/79549192/652bc96d-de7a-48e3-986f-b52eb38d9e5d)
![Screenshot from 2024-05-11 20-58-23](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/79549192/5f8577dc-655a-4417-97de-808b076cc972)
![Screenshot from 2024-05-11 20-58-27](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/79549192/84eb7ad6-ada3-45aa-99f8-5abc68878f2c)

*file change.log*
![Screenshot from 2024-05-11 21-18-19](https://github.com/irfanqs/Sisop-3-2024-MH-IT27/assets/79549192/74e049c6-e268-4e1c-b8eb-854c098652b7)


### Kendala
Tidak ada kendala pada nomor ini

### Revisi
Tidak ada Revisi pada nomor ini
