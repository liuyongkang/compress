/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *        Version:  1.0
 *        Created:  2013年10月23日 16时18分37秒
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
#include <limits.h>
#include <stdint.h>
#include <time.h>

#define LIMIT 24

void compress(char *in, char *out)
{
	uint8_t model[1 << LIMIT][2] = {{0}};
	uint32_t low = 0, high = UINT32_MAX;
	uint32_t status = 0;
	FILE *in_file = fopen(in, "rb");
	FILE *out_file = fopen(out, "wb");
	int cur;
	int i;
	int temp;
	uint32_t len = 0;
	double rate;

	fwrite(&len, sizeof(len), 1, out_file); 	// 给写文件大小留下四个字节

	while ((cur = fgetc(in_file)) != EOF) {
		++len;
	
		for (i = 0; i != 8; ++i) {
			if (1 & (cur >> i)) { 		// 分离每一位
				temp = status % (1 << LIMIT);
				rate = (model[temp][0] * 64 + 1) * 1.0 / (model[temp][0] * 64 + 1 + model[temp][1] * 64 + 1);
				low += (high - low) * rate + 1;
				if (!(++model[temp][1])) { 	// 溢出
					model[temp][1] = 128;
					model[temp][0] /= 2;
				}
				status = (status << 1) + 1;
			} else {
				temp = status % (1 << LIMIT);
				rate = (model[temp][0] * 64 + 1) * 1.0 / (model[temp][0] * 64 + 1 + model[temp][1] * 64 + 1);
				high = low + (high - low) * rate;
				if (!(++model[temp][0])) { 	// 溢出
					model[temp][0] = 128;
					model[temp][1] /= 2;
				}
				status <<= 1;
			}
			
			while ((high / (1 << 24)) == (low / (1 << 24))) { 	// 首字节相同，进行放大
				fputc((uint8_t)(high >> 24), out_file);
				low <<= 8;
				high = (high << 8) + 255;
			}
		}
	}
		
	fputc((uint8_t)(high >> 24), out_file);
	fputc((uint8_t)(high >> 16), out_file);
	fputc((uint8_t)(high >> 8), out_file);
	fputc((uint8_t)(high), out_file);

	fseek(out_file, 0, SEEK_SET);
	fwrite(&len, sizeof(len), 1, out_file);

	fclose(in_file);
	fclose(out_file);
}

void decompress(char *in, char *out)
{
	uint8_t model[1 << LIMIT][2] = {{0}};
	uint32_t low = 0, mid, high = 0;
	uint32_t cur = 0;
	uint32_t status = 0;
	FILE *in_file = fopen(in, "rb");
	FILE *out_file = fopen(out, "wb");
	int pos = 0;
	char out_char = 0;
	int temp;
	uint32_t len;
	unsigned char in_char;
	double rate;

	fread(&len, sizeof(uint32_t), 1, in_file);
	
	while (1) {
		while (high / (1 << 24) == low / (1 << 24)) {
			in_char = fgetc(in_file);
			cur = (cur << 8) | (uint32_t)in_char;
			low <<= 8;
			high = (high << 8) + 255;
		}

		temp = status % (1 << LIMIT);
		rate = (model[temp][0] * 64 + 1) * 1.0 / (model[temp][0] * 64 + 1 + model[temp][1] * 64 + 1);
		mid = low + (high - low) * rate;
		if (cur > mid) {
			if (!(++model[temp][1])) {
				model[temp][1] = 128;
				model[temp][0] /= 2;
			} 
			low = mid + 1;
			status = (status << 1) + 1;
			out_char |= 1 << (pos++);
		} else {
			if (!(++model[temp][0])) {
				model[temp][0] = 128;
				model[temp][1] /= 2;
			} 
			high = mid;
			status <<= 1;
			out_char &= ~(1 << (pos++));
		}

		if (pos == 8) {
			fputc(out_char, out_file);
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
			printf("%f\n", ((double)(e - s)) / CLOCKS_PER_SEC);
		} else if (strcmp(argv[1], "d") == 0) {
			decompress(argv[2], argv[3]);
			e = clock();
			printf("%f\n", ((double)(e - s)) / CLOCKS_PER_SEC);
		} else {
			printf("usage: e/d infile outfile\n");
		}
	} else {
	}

	return 0;
}

