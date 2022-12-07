#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>

#define OK 0
#define ERROR 1
#define ENDIMG 2
#define PADDING 3

#define FILESIZE 0
#define NAMESIZE 1
#define NAME 2
#define FILE_W 3
#define END_PARSE 4


struct IMG{
	short signature;
	int fileSize;
	short res1;
	short res2;
	int ofset;
	int headerSize;
	int width;
	int height;
	short planes;
	short bitsPerPixel;
	int compression;
	int imSize;
	int sizeUnusedHeader;
	FILE *file;	
	char *unusedHeader;
	int row;
	int column;
};


struct FileToWrite{
	int fileSize;
	int nameSize;
	char *name;
	char *file;
	char state;
	int bit;
	int byte;
};


char getNextBit(char *str, int size, int *n, int *bit) {
	if (*bit > 7) {
		*n = *n + 1;
		*bit = 0;
	}
	if (size <= *n){
		return -1;
	}
	char result = ((str[*n] >> *bit) % 2) == 0 ? 0 : 1;
	*bit = *bit + 1;
	return result;
}

int bbbit = 0;
int bbbit2 = 0;

char FileToWrite_get_next_bit(struct FileToWrite *obj){
	char bit = 0;
	switch (obj->state){		
		case FILESIZE:
			if (obj->byte < 4){
				bit = getNextBit((char*)&obj->fileSize, 4, &obj->byte, &obj->bit);
			}
			if (bit == -1) {
				obj->state = NAMESIZE;
				obj->byte = 0;
			}
			else {
				return bit;
			}
		case NAMESIZE:
			if (obj->byte < 4){
				bit = getNextBit((char*)&obj->nameSize, 4, &obj->byte, &obj->bit);
			}
			if (bit == -1) {
				obj->state = NAME;
				obj->byte = 0;
			}
			else {
				return bit;
			}
		case NAME:
			if (obj->byte < obj->nameSize){
				bit = getNextBit(obj->name, obj->nameSize, &obj->byte, &obj->bit);
			}
			if (bit == -1) {
				obj->state = FILE_W;
				obj->byte = 0;
			}
			else {
				return bit;
			}
		case FILE_W:
			if (obj->byte < obj->fileSize - obj->nameSize - 8){
				bit = getNextBit(obj->file, obj->fileSize - obj->nameSize - 8, &obj->byte, &obj->bit);
			}
			if (bit == -1) {
				obj->state = END_PARSE;
				obj->byte = 0;
			}
			else {
				return bit;
			}			
		case END_PARSE:
			return 0;
	}
	return 0;
}


void restore_file(struct FileToWrite *obj, char nextBit){
	switch (obj->state){		
		case FILESIZE:
			if (obj->bit < 32){
				int tmp = nextBit;
				tmp = tmp << obj->bit;
				obj->fileSize |= tmp;
				obj->bit++;
			}
			if (obj->bit >= 32) {
				obj->bit = 0;
				obj->state = NAMESIZE;
			}
			break;
		case NAMESIZE:
			if (obj->bit < 32){
				int tmp = nextBit;
				tmp = tmp << obj->bit;
				obj->nameSize |= tmp;
				obj->bit++;
			}
			if (obj->bit >= 32) {
				obj->bit = 0;
				obj->state = NAME;
			}
			break;
		case NAME:
			if (obj->byte == 0 && obj->bit == 0) {
				obj->name = malloc(obj->nameSize + 1);
				memset(obj->name, 0, obj->nameSize + 1);
				obj->name[obj->nameSize] = '\0';
			}
			if (obj->bit < 8){
				obj->name[obj->byte] |= nextBit << obj->bit;
				obj->bit++;
			}
			if (obj->bit >= 8) {
				obj->bit = 0;
				obj->byte++;
			}
			if (obj->byte >= obj->nameSize) {
				obj->byte = 0;
				obj->state = FILE_W;
			}
			break;
		case FILE_W:
			if (obj->byte == 0 && obj->bit == 0) {				
				obj->file = malloc(obj->fileSize - obj->nameSize - 8 + 1);
				memset(obj->file, 0, obj->fileSize - obj->nameSize - 8 + 1);
				obj->file[obj->fileSize - obj->nameSize - 8] = '\0';
			}
			if (obj->bit < 8){
				obj->file[obj->byte] |= nextBit << obj->bit;
				obj->bit++;
			}
			if (obj->bit >= 8) {
				obj->bit = 0;
				obj->byte++;
			}
			if (obj->byte >= obj->fileSize - obj->nameSize - 8) {
				obj->byte = 0;
				obj->state = END_PARSE;
			}		
		case END_PARSE:
			break;
	}
}


int IMG_get_headers(struct IMG *img, char *name) {
	img->file = fopen(name, "rb");
	if (img->file == NULL) {
		return ERROR;
	}
	fread(&img->signature, 2, 1, img->file);
	fread(&img->fileSize, 4, 1, img->file);
	fread(&img->res1, 2, 1, img->file);
	fread(&img->res2, 2, 1, img->file);
	fread(&img->ofset, 4, 1, img->file);	
	fread(&img->headerSize, 4, 1, img->file);
	fread(&img->width, 4, 1, img->file);
	fread(&img->height, 4, 1, img->file);
	fread(&img->planes, 2, 1, img->file);
	fread(&img->bitsPerPixel, 2, 1, img->file);
	fread(&img->compression, 4, 1, img->file);
	fread(&img->imSize, 4, 1, img->file);
	
	img->sizeUnusedHeader = img->headerSize-24;
	img->unusedHeader = malloc(img->sizeUnusedHeader);
	
	for (int  i = 0; i < img->sizeUnusedHeader; i++) {
		fread(&img->unusedHeader[i], 1, 1, img->file);
	}
	img->row = 0;
	img->column = 0;
	return OK;
}


int IMG_write_headers(struct IMG *img, char *name) {
	img->file = fopen(name, "wb");
	if (img->file == NULL) {
		return ERROR;
	}
	fwrite(&img->signature, 2, 1, img->file);
	fwrite(&img->fileSize, 4, 1, img->file);
	fwrite(&img->res1, 2, 1, img->file);
	fwrite(&img->res2, 2, 1, img->file);
	fwrite(&img->ofset, 4, 1, img->file);	
	fwrite(&img->headerSize, 4, 1, img->file);
	fwrite(&img->width, 4, 1, img->file);
	fwrite(&img->height, 4, 1, img->file);
	fwrite(&img->planes, 2, 1, img->file);
	fwrite(&img->bitsPerPixel, 2, 1, img->file);
	fwrite(&img->compression, 4, 1, img->file);
	fwrite(&img->imSize, 4, 1, img->file);
	
	for (int  i = 0; i < img->sizeUnusedHeader; i++) {
		fwrite(&img->unusedHeader[i], 1, 1, img->file);
	}
	return OK;
}


int IMG_get_next_pixel(struct IMG *img, char *pixel){
	if (img->row >= img->height) {
		return ENDIMG;
	}
	if (img->column == img->width) {
		img->row++;
		img->column = 0;
		int padding = (img->width * 3) % 4;
		padding = padding ? 4 - padding : 0;
		
		for (int i = 0; i < padding; i++) {
			fread(&pixel[i], 1, 1, img->file);
		}
		return PADDING;
	}
	fread(pixel, 3, 1, img->file);
	img->column++;
	return OK;
}


int IMG_write_next_pixel(struct IMG *img, char *pixel) {
	if (img->row >= img->height) {
		return ENDIMG;
	}
	if (img->column == img->width) {
		img->row++;
		img->column = 0;
		int padding = (img->width * 3) % 4;
		padding = padding ? 4 - padding : 0;
		
		for (int i = 0; i < padding; i++) {
			fwrite(&pixel[i], 1, 1, img->file);
		}
		return PADDING;
	}
	fwrite(pixel, 3, 1, img->file);
	img->column++;
	return OK;
}


void setLowerBit(char *byte, char bit) {
	*byte = *byte>>1<<1;
	*byte |= bit;
}

int main(int argc, char *argv[]){
	if (argc < 2 || argc > 3){
		printf("Use 'steg *.bmp <file>' for hiding \nor 'steg *.bmp' for recover\n");
		return 0;
	}
	char sys_check = 1;
	sys_check &= sizeof(int) == 4;
	sys_check &= sizeof(short) == 2;
	if (!sys_check) {
		printf("Sorry for that shit, you need to fix hard code, enjoy :)\n");
		return 0;
	}
	
	if (argc == 3){
		if (strstr(argv[1], ".bmp") == NULL){
			printf("I can't put data in that file, use BMP format\n");
			return 0;
		}
		FILE *file_to_hide = fopen(argv[2], "rb");
		fseek(file_to_hide, 0, SEEK_END);
		int size_file_to_hide = ftello(file_to_hide);
		char *filedata = malloc(size_file_to_hide);
		fseek(file_to_hide, 0, SEEK_SET);
		fread(filedata, size_file_to_hide, 1, file_to_hide);
		fclose(file_to_hide);
		
		struct FileToWrite hidden_file;
		hidden_file.name = argv[2];
		hidden_file.file = filedata;
		hidden_file.fileSize = 8 + strlen(hidden_file.name) + size_file_to_hide;
		hidden_file.nameSize = strlen(hidden_file.name);
		hidden_file.state = FILESIZE;
		hidden_file.bit = 0;
		hidden_file.byte = 0;
		
		int name_len = strlen(argv[1]);
		char *name_out_bmp = malloc(name_len + 2);
		memcpy(&name_out_bmp[0], "out_", 4);
		memcpy(&name_out_bmp[4], argv[1], name_len);
		name_out_bmp[name_len+4] = '\0';
		
		struct IMG in_img, out_img;
		IMG_get_headers(&in_img, argv[1]);
		if (in_img.signature != 0x4D42){
			printf("I can't put data in that file, use BMP format\n");
			return 0;
		}
		if (in_img.bitsPerPixel != 24){
			printf("I can't put data in that file, use format 24 bits per pixel\n");
			return 0;
		}		
		if (hidden_file.fileSize * 8 > in_img.width * in_img.height * 3){
			printf("\
Too large file for hide, in that file i can hide maximum %i bytes \
but your file have %i bytes\n", in_img.width * in_img.height * 3 / 8, hidden_file.fileSize);
			return 0;
		}
		memcpy(&out_img, &in_img, sizeof(in_img));
		IMG_write_headers(&out_img, name_out_bmp);
		char pix[3] = {0};
		int iresult = 0;
		while ( iresult != ENDIMG){
			iresult = IMG_get_next_pixel(&in_img, &pix[0]);
			if (iresult == OK) {
				for (int i = 0; i < 3; i++) {
					if (hidden_file.state != END_PARSE) {
						setLowerBit(&pix[i], FileToWrite_get_next_bit(&hidden_file));
					}
				}
			}
			IMG_write_next_pixel(&out_img, &pix[0]);
		}
		
		fclose(in_img.file);
		fclose(out_img.file);
		return 0;
	}
	if (argc == 2){
		if (strstr(argv[1], ".bmp") == NULL){
			printf("I can't get data from that file, use BMP format\n");
			return 0;
		}
		struct IMG in_img;
		struct FileToWrite hidden_file;
		hidden_file.fileSize =0;
		hidden_file.name = NULL;
		hidden_file.nameSize = 0;
		hidden_file.file = NULL;
		hidden_file.state = FILESIZE;
		hidden_file.bit = 0;
		hidden_file.byte = 0;
		IMG_get_headers(&in_img, argv[1]);
		
		if (in_img.signature != 0x4D42){
			printf("I can't get data from that file, use BMP format\n");
			return 0;
		}
		if (in_img.bitsPerPixel != 24){
			printf("I can't get data from that file, use format 24 bits per pixel\n");
			return 0;
		}		
		if (hidden_file.fileSize * 8 > in_img.width * in_img.height * 3){
			printf("\
Too large file for hide, in that file i can hide maximum %i bytes \
but your file have %i bytes\n", in_img.width * in_img.height * 3 / 8, hidden_file.fileSize);
			return 0;
		}
		
		int iresult = 0;
		char pix[3] = {0};
		while (iresult != ENDIMG){		
			iresult = IMG_get_next_pixel(&in_img, &pix[0]);
			if (iresult == OK) {
				for (int i = 0; i < 3; i++) {
					if (hidden_file.state != END_PARSE) {
						int t1=0, t2=0;
						restore_file(&hidden_file, getNextBit(&pix[i], 1, &t1, &t2));
					}
				}
			}
		}
		FILE *out_file = fopen(hidden_file.name, "wb");
		fwrite(hidden_file.file, hidden_file.fileSize-hidden_file.nameSize-8, 1, out_file);
		fclose(out_file);
	}
	return 0;
}