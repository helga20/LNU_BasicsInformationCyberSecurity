#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    char bl[512];
}
BUF;

int main(int argc, char *argv[])
{
    // ���������� �� �������� ������� ��� ����������
    if (argc != 2)
    {
        printf("Usage: ./recover image\n");
        return 1;
    }
    
    // �������� ��� �������
    char *f = argv[1];
    // ��������� ����
    FILE *infile = fopen(f, "rb");
    
    // ���� �� ������� ������� ����, ��������� ��������
    if (infile == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", f);
        return 2;
    }
    
    FILE *outfile;
    BUF bf;
    
    int a = 0;
    // ��'� ����������
    char st[] = "000.jpg";
    // ������ card.raw �� ���� ������� �� 512 ����
    while (fread(&bf, 512, 1, infile))
    {
        // ���� ������ �� ������� ����� ����� ����� JPEG
        if (bf.bl[0] == (char) 0xff && bf.bl[1] == (char) 0xd8 && bf.bl[2] == (char) 0xff)
        {
     // ���� ���������� �� �����, ��������� ��������� ����������
            if (a)
            {
                fclose(outfile);
            }
            // ������� ��� �����
            st[1] = a / 10 + '0';
            st[2] = a++ % 10 + '0';            
            // ��������� �� ��������� ����� ����
            outfile = fopen(st, "wb");
        }
        // ���������� ��� � card.raw � ������� ����������
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
