#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


typedef struct{
  char signature[2];
  int f_size;
  int reserved;
  int offset; 
  int size;
  int width;
  int height;
  short planes;
  short bit_count;
  int compression;
  int img_size;
  int x_pixels;
  int y_pixels;
  int colors;
  int imp_colors;
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

int main(int argc, char *argv[]){
  if(argc != 2){
    fprintf(stderr, "Usage %s <fisier_intrare>\n", argv[0]);
    return 1;
  }

  const char *fisier_intrare = argv[1];
  char fisier_iesire[] = "statistica2.txt";

  int intrare = open(fisier_intrare, O_RDONLY);
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
    return 1;
  }

  if(read(intrare, &img.f_size, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    return 1;
  }

  if(read(intrare, &img.reserved, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    return 1;
  }

  if(read(intrare, &img.offset, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    return 1;
  }
  
  if(read(intrare, &img.size, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    return 1;
  }
  
  if(read(intrare, &img.height, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    return 1;
  }
  
  if(read(intrare, &img.width, 4) != 4){
    perror("Eroare la citirea header-ului BMP");
    close(intrare);
    close(iesire);
    return 1;
  }

  if(img.signature[0] != 'B' || img.signature[1] != 'M'){
    fprintf(stderr, "Fisierul nu este de tip BMP\n");
    close(intrare);
    close(iesire);
    exit(-1);
  }

  struct stat fileInfo;
  if(fstat(intrare, &fileInfo) == -1){
    perror("Eroare la obtinerea informatiilor despre fisierul de intrare");
    close(intrare);
    close(iesire);
    return 1;
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
	 fisier_intrare, img.height, img.width, fileInfo.st_size, fileInfo.st_uid, modTimp, fileInfo. st_nlink, permUSR, permGRP, permOTH);

printf("%ld", sizeof(img));
 
  if(write(iesire, outputBuffer, strlen(outputBuffer)) == -1){
    perror("Eroare la scrierea in fisierul de lesire");
  }
  
    close(intrare);
    close(iesire);
    return 0;
}
