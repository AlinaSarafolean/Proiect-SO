#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

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

void convertToGray(const char *inputFile, const char *outputFile) {
    int input = open(inputFile, O_RDONLY);
    if (input == -1) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    Bmp img;
    if (read(input, &img.signature, 2) != 2) {
        perror("Error reading BMP header");
        close(input);
        exit(EXIT_FAILURE);
    }

    int output = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output == -1) {
        perror("Error opening output file");
        close(input);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < img.height; ++i) {
        for (int j = 0; j < img.width; ++j) {
            uint8_t pixel[3];
            if (read(input, pixel, 3) != 3) {
                perror("Error reading pixel values");
                close(input);
                close(output);
                exit(EXIT_FAILURE);
            }

            uint8_t grayValue = (uint8_t)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);

            if (write(output, &grayValue, 1) != 1) {
                perror("Error writing gray pixel value");
                close(input);
                close(output);
                exit(EXIT_FAILURE);
            }
        }
    }

    close(input);
    close(output);
}

void imagine(const char *inputFile, const char *outputDir) {
    char outputFileName[BUFFER];
    snprintf(outputFileName, BUFFER, "%s/%s_statistica.txt", outputDir, inputFile);

    char grayOutputFileName[BUFFER];
    snprintf(grayOutputFileName, BUFFER, "%s/%s_gray.bmp", outputDir, inputFile);

    pid_t pid = fork();
    if (pid == -1) {
        perror("Error forking process");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        struct stat fileInfo;
        if (stat(inputFile, &fileInfo) == -1) {
            perror("Error getting file information");
            exit(EXIT_FAILURE);
        }

        char outputDirFullPath[BUFFER];
        snprintf(outputDirFullPath, BUFFER, "%s", outputDir);

        // Adaugare gestionare eroare la deschiderea sau crearea directorului de ieșire
        if (mkdir(outputDirFullPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1 && errno != EEXIST) {
            perror("Error creating output directory");
            exit(EXIT_FAILURE);
        }

        FILE *outputFile = fopen(outputFileName, "w");
        if (!outputFile) {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }

        Bmp img;
        afisare(inputFile, &img, &fileInfo);

        fclose(outputFile);

        if (img.signature == 0x4D42) {
            convertToGray(inputFile, grayOutputFileName);
        }

        exit(EXIT_SUCCESS);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Process with PID %d exited with code %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Process with PID %d terminated by signal %d\n", pid, WTERMSIG(status));
        }
    }
}

void director(const char *inputDir, const char *outputDir) {
    DIR *dir = opendir(inputDir);
    if (!dir) {
        perror("Error opening input directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char inputPath[BUFFER];
        char outputPath[BUFFER];

        snprintf(inputPath, BUFFER, "%s/%s", inputDir, entry->d_name);
        snprintf(outputPath, BUFFER, "%s/%s_statistica.txt", outputDir, entry->d_name);

        imagine(inputPath, outputPath);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_directory> <output_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    director(argv[1], argv[2]);

    return 0;
}


