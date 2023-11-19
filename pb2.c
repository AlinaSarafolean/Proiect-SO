
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
    int32_t width;
    int32_t height;
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
    snprintf(permUSR, 4, "%c%c%c",
             (mode & S_IRUSR) ? 'R' : '-',
             (mode & S_IWUSR) ? 'W' : '-',
             (mode & S_IXUSR) ? 'X' : '-');
    snprintf(permGRP, 4, "%c%c%c",
             (mode & S_IRGRP) ? 'R' : '-',
             (mode & S_IWGRP) ? 'W' : '-',
             (mode & S_IXGRP) ? 'X' : '-');
    snprintf(permOTH, 4, "%c%c%c",
             (mode & S_IROTH) ? 'R' : '-',
             (mode & S_IWOTH) ? 'W' : '-',
             (mode & S_IXOTH) ? 'X' : '-');
}

void afisare(const char *fis, const Bmp *img, struct stat *fileInfo) {
    printf("Processing: %s\n", fis);
    if (img != NULL) {
        printf("Width: %d\n", img->width);
        printf("Height: %d\n", img->height);
    }
    printf("File Size: %lu\n", fileInfo->st_size);
}

void imagine(const char *fis) {
    char fisier_iesire[] = "statistica.txt";
    int intrare = open(fis, O_RDONLY);
    if (intrare == -1) {
        perror("Error opening input file");
        exit(-1);
    }

    struct stat fileInfo;
    if (fstat(intrare, &fileInfo) == -1) {
        perror("Error getting input file information");
        close(intrare);
        exit(-1);
    }

    if (S_ISDIR(fileInfo.st_mode)) {
        // Este un director, nu trebuie să citim antetul BMP
        close(intrare);
        return;
    }

    int iesire = open(fisier_iesire, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (iesire == -1) {
        perror("Error opening output file");
        close(intrare);
        exit(-1);
    }

    Bmp img;

    if (read(intrare, &img.signature, 2) != 2) {
        perror("Error reading BMP header");
        close(intrare);
        close(iesire);
        exit(-1);
    }

    char modTimp[20];
    struct tm *timeInfo = localtime(&fileInfo.st_mtime);
    strftime(modTimp, sizeof(modTimp), "%d.%m.%Y", timeInfo);

    char outputBuffer[512];
    permisiune(fileInfo.st_mode);

    if (img.signature == 0x4D42) {
        
        snprintf(outputBuffer, sizeof(outputBuffer),
                 "File Name: %s\n"
                 "Height: %d\n"
                 "Width: %d\n"
                 "File Size: %lu\n"
                 "User ID: %d\n"
                 "Last Modification Time: %s\n"
                 "Link Count: %lu\n"
                 "User Permissions: %s\n"
                 "Group Permissions: %s\n"
                 "Others Permissions: %s\n",
                 fis, img.height, img.width, fileInfo.st_size, fileInfo.st_uid, modTimp, fileInfo.st_nlink, permUSR, permGRP, permOTH);
    } else if (S_ISREG(fileInfo.st_mode)) {
        
        snprintf(outputBuffer, sizeof(outputBuffer),
                 "File Name: %s\n"
                 "File Size: %lu\n"
                 "User ID: %d\n"
                 "Last Modification Time: %s\n"
                 "Link Count: %lu\n"
                 "User Permissions: %s\n"
                 "Group Permissions: %s\n"
                 "Others Permissions: %s\n",
                 fis, fileInfo.st_size, fileInfo.st_uid, modTimp, fileInfo.st_nlink, permUSR, permGRP, permOTH);
    } else if (S_ISLNK(fileInfo.st_mode)) {
        
        char target[BUFFER];
        ssize_t targetSize = readlink(fis, target, BUFFER - 1);
        if (targetSize != -1) {
            target[targetSize] = '\0';
            snprintf(outputBuffer, sizeof(outputBuffer),
                     "Link Name: %s\n"
                     "Link Size: %lu\n"
                     "Target File Size: %lu\n"
                     "User Permissions: RWX\n"
                     "Group Permissions: R--\n"
                     "Others Permissions: ---\n",
                     fis, fileInfo.st_size, targetSize, permUSR, permGRP, permOTH);
        }
    }

    if (write(iesire, outputBuffer, strlen(outputBuffer)) == -1) {
        perror("Error writing to output file");
    }

    close(intrare);
    close(iesire);
}

void director(const char *nume) {
    DIR *dir = opendir(nume);
    if (!dir) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char path[BUFFER];
        snprintf(path, BUFFER, "%s/%s", nume, entry->d_name);
        imagine(path);
        if (entry->d_type == DT_DIR)
            director(path);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    director(argv[1]);

    return 0;
}






 
