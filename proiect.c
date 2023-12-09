#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#define BUFFER 1024

typedef struct {
    uint16_t signature;
    uint32_t f_size;
    uint32_t reserved;
    uint32_t offset;
    uint32_t size;
    int32_t height;
    int32_t width;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t img_size;
    int32_t x_pixels;
    int32_t y_pixels;
    uint32_t colors;
    uint32_t imp_colors;
} Bmp;

char permUSR[4];
char permGRP[4];
char permOTH[4];

void permisiune(mode_t mode) {
    snprintf(permUSR, 4, "%c%c%c", (mode & S_IRUSR) ? 'R' : '-', (mode & S_IWUSR) ? 'W' : '-', (mode & S_IXUSR) ? 'X' : '-');
    snprintf(permGRP, 4, "%c%c%c", (mode & S_IRGRP) ? 'R' : '-', (mode & S_IWGRP) ? 'W' : '-', (mode & S_IXGRP) ? 'X' : '-');
    snprintf(permOTH, 4, "%c%c%c", (mode & S_IROTH) ? 'R' : '-', (mode & S_IWOTH) ? 'W' : '-', (mode & S_IXOTH) ? 'X' : '-');
}


void imagine(const char *fis, int tipFisier) {
    char fisier_iesire[] = "statistica.txt";
    int iesire = open(fisier_iesire, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (iesire == -1) {
        perror("Eroare la deschiderea fisierului de iesire");
        exit(-1);
    }

    struct stat fileInfo;
    if (stat(fis, &fileInfo) == -1) {
        perror("Eroare la obtinerea informatiilor despre fisier");
        close(iesire);
        exit(-1);
    }

    char modTimp[20];
    struct tm *timeInfo = localtime(&fileInfo.st_mtime);
    strftime(modTimp, sizeof(modTimp), "%d.%m.%Y", timeInfo);

    char outputBuffer[512];
    permisiune(fileInfo.st_mode);

    if (tipFisier == 1) { // BMP
        Bmp img;
        int intrare = open(fis, O_RDONLY);
        if (intrare == -1) {
            perror("Eroare la deschiderea fisierului de intrare");
            close(iesire);
            exit(-1);
        }

        if (read(intrare, &img.signature, sizeof(img.signature)) != sizeof(img.signature) ||
            read(intrare, &img.f_size, sizeof(img.f_size)) != sizeof(img.f_size) ||
            read(intrare, &img.reserved, sizeof(img.reserved)) != sizeof(img.reserved) ||
            read(intrare, &img.offset, sizeof(img.offset)) != sizeof(img.offset) ||
            read(intrare, &img.size, sizeof(img.size)) != sizeof(img.size) ||
            read(intrare, &img.height, sizeof(img.height)) != sizeof(img.height) ||
            read(intrare, &img.width, sizeof(img.width)) != sizeof(img.width)) {
            perror("Eroare la citirea header-ului BMP");
            close(intrare);
            close(iesire);
            exit(-1);
        }

        close(intrare);

        snprintf(outputBuffer, sizeof(outputBuffer),
                 "nume fisier: %s\n"
                 "inaltime: %d\n"
                 "lungime: %d\n"
                 "dimensiune: %lu\n"
                 "identificatorul utilizatorului: %d\n"
                 "timpul ultimei modificari: %s\n"
                 "contorul de legaturi: %lu\n"
                 "drepturi de acces user: %s\n"
                 "drepturi de acces grup: %s\n"
                 "drepturi de acces altii: %s\n"
		 "\n",
                 fis, img.height, img.width, fileInfo.st_size, fileInfo.st_uid, modTimp, fileInfo.st_nlink, permUSR,
                 permGRP, permOTH);
    } else if (tipFisier == 2) { // Director
        snprintf(outputBuffer, sizeof(outputBuffer),
                 "nume director: %s\n"
                 "identificatorul utilizatorului: %d\n"
                 "drepturi de acces user: %s\n"
                 "drepturi de acces grup: %s\n"
                 "drepturi de acces altii: %s\n"
		 "\n",
                 fis, fileInfo.st_uid, permUSR, permGRP, permOTH);
    } else if (tipFisier == 3) { // Legatura simbolica
        snprintf(outputBuffer, sizeof(outputBuffer),
                 "nume legatura simbolica: %s\n"
                 "dimensiune legatura: %lu\n"
                 "dimensiune fisier: %lu\n"
                 "drepturi de acces user: %s\n"
                 "drepturi de acces grup: %s\n"
                 "drepturi de acces altii: %s\n"
		 "\n",
                 fis, fileInfo.st_size, fileInfo.st_size, permUSR, permGRP, permOTH);
    } else { // Fisier obisnuit
        snprintf(outputBuffer, sizeof(outputBuffer),
                 "nume fisier: %s\n"
                 "dimensiune: %lu\n"
                 "identificatorul utilizatorului: %d\n"
                 "timpul ultimei modificari: %s\n"
                 "contorul de legaturi: %lu\n"
                 "drepturi de acces user: %s\n"
                 "drepturi de acces grup: %s\n"
                 "drepturi de acces altii: %s\n"
		 "\n",
                 fis, fileInfo.st_size, fileInfo.st_uid, modTimp, fileInfo.st_nlink, permUSR, permGRP, permOTH);
    }

    if (write(iesire, outputBuffer, strlen(outputBuffer)) == -1) {
        perror("Eroare la scrierea in fisierul de iesire");
    }

    close(iesire);
}

void director(const char *nume) {
    DIR *dir = opendir(nume);
    if (!dir) {
        perror("Error opening directory");
        return;
    }

    struct stat fileInfo;
    if (stat(nume, &fileInfo) == -1) {
        perror("Eroare la obtinerea informatiilor despre director");
        closedir(dir);
        exit(-1);
    }

    imagine(nume, 2); // Director

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char path[BUFFER];
        snprintf(path, BUFFER, "%s/%s", nume, entry->d_name);

        if (entry->d_type == DT_REG) {
            if (strstr(entry->d_name, ".bmp") != NULL) {
                imagine(path, 1); // BMP
            } else {
                imagine(path, 0); // Fisier obisnuit
            }
        } else if (entry->d_type == DT_DIR) {
            director(path);
        } else if (entry->d_type == DT_LNK) {
            imagine(path, 3); // Legatura simbolica
        }
    }

    closedir(dir);
 } 

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <director_intrare>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    director(argv[1]);

    return 0;
}
