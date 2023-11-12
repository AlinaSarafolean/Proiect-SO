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
}Bmp;


char permUSR[4];
char permGRP[4];
char permOTH[4];



void permisiune(mode_t mode){
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


void afisare(const char *fis, const Bmp *img) {
    printf("Processing BMP file: %s\n", fis);
    printf("Width: %d\n", img->width);
    printf("Height: %d\n", img->height);
    printf("File Size: %u\n", img->f_size);
   
}

void imagine(const char *fis) {

  char fisier_iesire[] = "statistica.txt";
  int intrare = open(fis, O_RDONLY);
  if(intrare == -1){
    perror("Eroare la deschiderea fisierului de intrare");
    exit(-1);
  }

  int iesire = open(fisier_iesire, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if(iesire == -1){
    perror("Eroare la deschiderea fisierului de iesire");
    exit(-1);
  }

  Bmp img;
  
  if(read(intrare, &img.signature, 2) != 2){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    exit(-1);
  }

  if(read(intrare, &img.f_size, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    exit(-1);
  }

  if(read(intrare, &img.reserved, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    exit(-1);
  }

  if(read(intrare, &img.offset, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    exit(-1);
  }
  
  if(read(intrare, &img.size, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    exit(-1);
  }
  
  if(read(intrare, &img.height, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    exit(-1);
  }
  
  if(read(intrare, &img.width, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    exit(-1);
  }

    if(img.signature != 0x4D42) {
        printf("Not a valid BMP file: %s\n", fis);
        close(intrare);
	close(iesire);
	  exit(-1) ;
    }

  struct stat fileInfo;
  if(fstat(intrare, &fileInfo) == -1){
    perror("Eroare la obtinerea informatiilor despre fisierul de intrare");
    close(intrare);
    close(iesire);
    exit(-1);
  }   

  char modTimp[20];
  struct tm *timeInfo = localtime(&fileInfo.st_mtime);
  strftime(modTimp, sizeof(modTimp), "%d.%m.%Y", timeInfo);


    char outputBuffer [512];
  permisiune(fileInfo.st_mode);
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
	 "drepturi de acces altii: %s\n",
	 fis, img.height, img.width, fileInfo.st_size, fileInfo.st_uid, modTimp, fileInfo.st_nlink, permUSR, permGRP, permOTH);

printf("%ld", sizeof(img));
 
  if(write(iesire, outputBuffer, strlen(outputBuffer)) == -1){
    perror("Eroare la scrierea in fisierul de lesire");
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
        fprintf(stderr, "Usage: %s <director_intrare>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    director(argv[1]);

    return 0;
}






 
