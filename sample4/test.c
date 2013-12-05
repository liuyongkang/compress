/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年12月05日 14时12分52秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Nick (LiuYongkang), liuyongkanglinux@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <limits.h>

#define LENGTH 16

uint8_t model[1 << LENGTH][2] = {{0}};


uint16_t predict(uint16_t context, uint16_t high, uint16_t low)
{
	int m0, m1;

	m0 = model[context][0] + 1;
	m1 = model[context][1] + 1;

	return low + (high - low) * m0 / (m0 + m1); 
}

void update(uint16_t *context, int b)
{
	if (model[*context][b] == 255) {
		model[*context][0] /= 2;
		model[*context][1] /= 2;
	} else {
		model[*context][b]++;
	}
	*context = *context * 2 + b;
}


void compress(char *in, char *out)
{
	FILE *in_file = fopen(in, "rb");
	FILE *out_file = fopen(out, "wb");
	uint16_t high = 65535;
	uint16_t low = 0;
	uint16_t context = 0;
	uint32_t len = 0;
	int cur;
	int i;

	fwrite(&len, sizeof(len), 1, out_file); 	// 给写文件大小留下四个字节

	while ((cur = fgetc(in_file)) != EOF) {
		len++;
		for (i = 0; i < 8; i++) {
			if (1 & (cur >> i)) {
				low = predict(context, high, low) + 1;
				update(&context, 1);
			} else {
				high = predict(context, high, low);
				update(&context, 0);
			}

			if (high == low) {
				fputc(high / 256, out_file);
				fputc(high % 256, out_file);
				high = 65535;
				low = 0;
			}
		}
	}

//	fputc((uint8_t)(high >> 24), out_file);
//	fputc((uint8_t)(high >> 16), out_file);
	fputc((uint8_t)(high >> 8), out_file);
	fputc((uint8_t)(high), out_file);

	fseek(out_file, 0, SEEK_SET);
	fwrite(&len, sizeof(len), 1, out_file);

	fclose(in_file);
	fclose(out_file);
}

void decompress(char *in, char *out)
{
	FILE *in_file = fopen(in, "rb");
	FILE *out_file = fopen(out, "wb");
	uint16_t high = 0;
	uint16_t mid;
	uint16_t val = 0;
	uint16_t low = 0;
	uint16_t context = 0;
	uint32_t len;
	int cur;
	int outc;
	int pos = 0;
	
	fread(&len, sizeof(len), 1, in_file);
	
	while (1) {
		while (high == low) {
			val = (val << 8) | fgetc(in_file);
			val = (val << 8) | fgetc(in_file);
			low = 0;
			high = 65535;
		}
		
		mid = predict(context, high, low);

		if (val > mid) {
			low = mid + 1;
			outc |= (1 << (pos++));
			update(&context, 1);
		} else {
			high = mid;
			outc &= ~(1 << (pos++));
			update(&context, 0);
		}


		if (pos == 8) {
			fputc(outc, out_file);
			if (!(--len)) {
				break;
			}
			pos = 0;
		}
	}

	fclose(in_file);
	fclose(out_file);
}



int main(int argc, char **argv)
{
	clock_t s, e;

	if (argc == 4) {
		s = clock();
		if (strcmp(argv[1], "e") == 0) {
			compress(argv[2], argv[3]);
			e = clock();
			printf("%fs\n", ((double)(e - s)) / CLOCKS_PER_SEC);
		} else if (strcmp(argv[1], "d") == 0) {
			decompress(argv[2], argv[3]);
			e = clock();
			printf("%fs\n", ((double)(e - s)) / CLOCKS_PER_SEC);
		} else {
			printf("usage: e/d infile outfile\n");
		}
	} else {
		printf("usage: e/d infile outfile\n");
	}

	return 0;
}

