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

#define LIMIT 16

void compress(char *in, char *out)
{
	uint8_t model[1 << LIMIT][2] = {{0}};
	uint32_t low = 0, high = UINT32_MAX;
	uint32_t status = 0;
	FILE *in_file = fopen(in, "rb");
	FILE *out_file = fopen(out, "wb");
	char cur;
//	char in_buffer[4096];
//	char out_buffer[4096];
	int i;
	int temp;
	uint32_t len = 0;
	double rate;

//	setbuf(in_file, in_buffer);
//	setbuf(out_file, out_buffer);

	fwrite(&len, sizeof(len), 1, out_file);
	while ((cur = fgetc(in_file)) != EOF) {
		++len;
	
		for (i = 0; i != 8; ++i) {
			if (1 & (cur >> i)) {
				temp = status % (1 << LIMIT);
				rate = (model[temp][0] + 1) * 1.0 / (model[temp][0] + 1 + model[temp][1] + 1);
				low += (high - low) * rate + 1;
				if (!(++model[temp][1])) {
					model[temp][1] = 128;
					model[temp][0] /= 2;
				}
			} else {
				temp = status % (1 << LIMIT);
				rate = (model[temp][0] + 1) * 1.0 / (model[temp][0] + 1 + model[temp][1] + 1);
				high = low + (high - low) * rate;
				if (!(++model[temp][0])) {
					model[temp][0] = 1 << 7;
					model[temp][1] /= 2;
				}
			}
			status = (status << 1) + (1 & (cur >> i));
			
			while ((high / (1 << 24)) == (low / (1 << 24))) {
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
//	char in_buffer[4096];
//	char out_buffer[4096];
	int pos = 0;
	char out_char = 0;
	int temp;
	uint32_t len;
	unsigned char in_char;
	double rate;

//	setbuf(in_file, in_buffer);
//	setbuf(out_file, out_buffer);
	
	fread(&len, sizeof(uint32_t), 1, in_file);
	
	while (!feof(out_file)) {
		while (high / (1 << 24) == low / (1 << 24) && !feof(out_file)) {
			in_char = fgetc(in_file);
			cur = (cur << 8) | (uint32_t)in_char;
			low <<= 8;
			high = (high << 8) + 255;
		}

		temp = status % (1 << LIMIT);
		rate = (model[temp][0] + 1) * 1.0 / (model[temp][0] + 1 + model[temp][1] + 1);
		mid = low + (high - low) * rate;
		if (cur > mid) {
			low = mid + 1;
			if (!(++model[temp][1])) {
				model[temp][1] = 128;
				model[temp][0] /= 2;
			}
			status = (status << 1) + 1;
			out_char |= 1 << (pos++);
		} else {
			high = mid;
			if (!(++model[temp][0])) {
				model[temp][0] = 128;
				model[temp][1] /= 2;
			}
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
	if (argc == 4) {
		if (strcmp(argv[1], "e") == 0) {
			compress(argv[2], argv[3]);
		} else if (strcmp(argv[1], "d") == 0) {
			decompress(argv[2], argv[3]);
		}
	} else {
		printf("usage: e/d infile outfile\n");
	}

	return 0;
}

