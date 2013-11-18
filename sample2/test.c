/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年11月16日 17时12分51秒
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

#define MEM 1 << 20

struct node {
	uint32_t count[2];
	struct node *next[2];
};

struct node nodes[256][256];
struct node *p;
struct node model[MEM];
int num;

void node_init()
{
	int i, j;

	for (i = 0; i != 256; ++i) {
		for (j = 0; j != 127; ++j) {
			nodes[i][j].count[0] = 0;
			nodes[i][j].count[1] = 0;
			nodes[i][j].next[0] = &nodes[i][j * 2 + 1];
			nodes[i][j].next[1] = &nodes[i][j * 2 + 2];
		}
		for (; j != 255; ++j) {
			nodes[i][j].count[0] = 0;
			nodes[i][j].count[1] = 0;
			nodes[i][j].next[0] = &nodes[j * 2 - 254][0];
			nodes[i][j].next[1] = &nodes[j * 2 - 253][0];
		}
	}
	
	p = &nodes[0][0];
	num = 0;

	printf("\n");
}

double node_predict()
{
	return (p->count[0] + 1) * 1.0 / (p->count[0] + p->count[1] + 2);
}

void node_update(int bit)
{
	struct node *new_node;
	double rate;

	if (p->count[bit] > 1 && p->next[bit]->count[0] + p->next[bit]->count[1] > p->count[bit] + 1) {
		
		new_node = &model[num++];
		rate = p->count[bit] * 1.0 / (p->next[bit]->count[0] + p->next[bit]->count[1]);
		new_node->count[0] = p->next[bit]->count[0] * rate;
		new_node->count[1] = p->next[bit]->count[1] * rate;
		new_node->next[0] = p->next[bit]->next[0];
		new_node->next[1] = p->next[bit]->next[1];

		p->next[bit]->count[0] -= new_node->count[0];
		p->next[bit]->count[1] -= new_node->count[1];
		p->next[bit] = new_node;
	}
	
	++p->count[bit];
	p = p->next[bit];

	if (num == MEM) {
		node_init();
	}
}

void compress(char *in, char *out)
{
	FILE *in_file = fopen(in, "rb");
	FILE *out_file = fopen(out, "wb");
	uint32_t low = 0, mid, high = UINT32_MAX;
	int in_char;
	int bit;
	int i;
	uint32_t len = 0;

	node_init();

	fwrite(&len, sizeof(len), 1, out_file);

	while ((in_char = fgetc(in_file)) != EOF ) {
		++len;

		for (i = 0; i != 8; ++i) {
			bit = (in_char << i) & 0x80;
			
			mid = low + (high - low) * node_predict();
			node_update(bit != 0);

			if (bit) {
				low = mid + 1;
			} else {
				high = mid;
			}

			while (high >> 24 == low >> 24) {
				fputc((uint8_t)(high >> 24), out_file);
				low <<= 8;
				high = (high << 8) | 255;
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
	FILE *in_file = fopen(in, "rb");
	FILE *out_file = fopen(out, "wb");
	uint32_t low = 0, mid, cur = 0, high = 0;
	int in_char;
	uint8_t out_char;
	int pos = 0;
	uint32_t len;

	node_init();

	fread(&len, sizeof(len), 1, in_file);

	while (1) {
		while (high >> 24 == low >> 24) {
			in_char = fgetc(in_file);
			cur = (cur << 8) | in_char;
			low <<= 8;
			high = (high << 8) | 255;
		}
		mid = low + (high - low) * node_predict();
		if (cur > mid) {
			node_update(1);
			low = mid + 1;
			out_char |= 1 << (7 - (pos++));
		} else {
			node_update(0);
			high = mid;
			out_char &= ~(1 << (7 - (pos++)));
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

int main(int argc, char *argv[])
{
	clock_t s, e;

	if (argc == 4) {
		s = clock();
		if (strcmp(argv[1], "e") == 0) {
			compress(argv[2], argv[3]);
			e = clock();
			printf("%fs\n", ((double)(e - s) / CLOCKS_PER_SEC));
		} else if (strcmp(argv[1], "d") == 0) {
			decompress(argv[2], argv[3]);
			e = clock();
			printf("%fs\n", ((double)(e - s) / CLOCKS_PER_SEC));
		} else {
			printf("usage: e/d infile outfile\n");
		}
	} else {
		printf("usage: e/d infile outfile");
	}

	return 0;
}

