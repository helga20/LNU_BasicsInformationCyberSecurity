#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    char bl[512];
}
BUF;

int main(int argc, char *argv[])
{
    // перевіряємо чи передано відбиток для відновлення
    if (argc != 2)
    {
        printf("Usage: ./recover image\n");
        return 1;
    }
    
    // отримуємо імя відбитку
    char *f = argv[1];
    // відкриваємо файл
    FILE *infile = fopen(f, "rb");
    
    // якщо не вдалось відкрити файл, завершуємо програму
    if (infile == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", f);
        return 2;
    }
    
    FILE *outfile;
    BUF bf;
    
    int a = 0;
    // ім'я фотографії
    char st[] = "000.jpg";
    // читаємо card.raw до кінця блоками по 512 байт
    while (fread(&bf, 512, 1, infile))
    {
        // якщо бачимо на початку блоку підпис файлу JPEG
        if (bf.bl[0] == (char) 0xff && bf.bl[1] == (char) 0xd8 && bf.bl[2] == (char) 0xff)
        {
     // якщо фотографія не перша, завершуємо формувати фотографію
            if (a)
            {
                fclose(outfile);
            }
            // формуємо імя файлу
            st[1] = a / 10 + '0';
            st[2] = a++ % 10 + '0';            
            // створюємо та відкриваємо новий файл
            outfile = fopen(st, "wb");
        }
        // переписуємо дані з card.raw в поточну фотографію
        if (a)
        {
            fwrite(&bf, 512, 1, outfile);
        }
    }
    // close
    fclose(infile);
    fclose(outfile);
    return 0;
}
