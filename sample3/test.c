/*
 * =====================================================================================
 *
 *       Filename:  test.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年12月04日 12时42分35秒
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
#include <time.h>
#include <string.h>

#define MAX_ALPHA_SIZE 258
#define MAX_CODE_LEN   23
#define True           1
#define False          0
#define BLOCK          65536
#define MAX_LEN        15
#define WEIGHTOF(zz0)  ((zz0) & 0xffffff00)
#define DEPTHOF(zz1)   ((zz1) & 0x000000ff)
#define MYMAX(zz2,zz3) ((zz2) > (zz3) ? (zz2) : (zz3))

#define ADDWEIGHTS(zw1,zw2)                                     \
	(WEIGHTOF(zw1)+WEIGHTOF(zw2)) |                         \
(1 + MYMAX(DEPTHOF(zw1),DEPTHOF(zw2)))

#define UPHEAP(z)                                               \
{                                                               \
	int zz, tmp;                                            \
	zz = z; tmp = heap[zz];                                 \
	while (weight[tmp] < weight[heap[zz >> 1]]) {           \
		heap[zz] = heap[zz >> 1];                       \
		zz >>= 1;                                       \
	}                                                       \
	heap[zz] = tmp;                                         \
}

#define DOWNHEAP(z)                                             \
{                                                               \
	int zz, yy, tmp;                                        \
	zz = z; tmp = heap[zz];                                 \
	while (True) {                                          \
		yy = zz << 1;                                   \
		if (yy > nHeap) break;                          \
		if (yy < nHeap &&                               \
			weight[heap[yy+1]] < weight[heap[yy]])  \
			yy++;                                   \
		if (weight[tmp] < weight[heap[yy]]) break;      \
		heap[zz] = heap[yy];                            \
		zz = yy;                                        \
	}                                                       \
	heap[zz] = tmp;                                         \
}



void makeCodeLengths (int *len, 
		      int *freq,
		      int alphaSize,
		      int maxLen )
{
	int nNodes, nHeap, n1, n2, i, j, k;
	int  tooLong;

	int heap   [ MAX_ALPHA_SIZE + 2 ];
	int weight [ MAX_ALPHA_SIZE * 2 ];
	int parent [ MAX_ALPHA_SIZE * 2 ]; 

	for (i = 0; i < alphaSize; i++)
		weight[i+1] = (freq[i] == 0 ? 1 : freq[i]) << 8;

	while (True) {

		nNodes = alphaSize;
		nHeap = 0;

		heap[0] = 0;
		weight[0] = 0;
		parent[0] = -2;

		for (i = 1; i <= alphaSize; i++) {
			parent[i] = -1;
			nHeap++;
			heap[nHeap] = i;
			UPHEAP(nHeap);
		}

		while (nHeap > 1) {
			n1 = heap[1]; heap[1] = heap[nHeap]; nHeap--; DOWNHEAP(1);
			n2 = heap[1]; heap[1] = heap[nHeap]; nHeap--; DOWNHEAP(1);
			nNodes++;
			parent[n1] = parent[n2] = nNodes;
			weight[nNodes] = ADDWEIGHTS(weight[n1], weight[n2]);
			parent[nNodes] = -1;
			nHeap++;
			heap[nHeap] = nNodes;
			UPHEAP(nHeap);
		}

		tooLong = False;
		for (i = 1; i <= alphaSize; i++) {
			j = 0;
			k = i;
			while (parent[k] >= 0) { k = parent[k]; j++; }
			len[i-1] = j;
			if (j > maxLen) tooLong = True;
		}

		if (! tooLong) break;

		for (i = 1; i <= alphaSize; i++) {
			j = weight[i] >> 8;
			j = 1 + (j / 2);
			weight[i] = j << 8;
		}
	}
}


void assignCodes (int *code,
		  int *length,
		  int minLen,
		  int maxLen,
		  int alphaSize )
{
	int n, vec, i;
	int t1;
	int t2;

	vec = 0;
	for (n = minLen; n <= maxLen; n++) {
		for (i = 0; i < alphaSize; i++)
			if (length[i] == n) { code[i] = vec; vec++; };
		vec <<= 1;
	}

	for (i = 0; i < 256; i++) {
		t1 = 0;
		t2 = length[i] - 1;
		while (t1 < t2) {
			code[i] ^= (1 & (code[i] >> t1)) << t2;
			code[i] ^= (1 & (code[i] >> t2)) << t1;
			code[i] ^= (1 & (code[i] >> t1)) << t2;
			t1++;
			t2--;
		}
	}
}


void createDecodeTables (int *limit,
			 int *base,
			 int *perm,
			 int *length,
			 int minLen,
			 int maxLen,
			 int alphaSize )
{
	int pp, i, j, vec;

	pp = 0;
	for (i = minLen; i <= maxLen; i++)
		for (j = 0; j < alphaSize; j++)
			if (length[j] == i) { perm[pp] = j; pp++; };

	for (i = 0; i < MAX_CODE_LEN; i++) base[i] = 0;
	for (i = 0; i < alphaSize; i++) base[length[i]+1]++;

	for (i = 1; i < MAX_CODE_LEN; i++) base[i] += base[i-1];

	for (i = 0; i < MAX_CODE_LEN; i++) limit[i] = 0;
	vec = 0;

	for (i = minLen; i <= maxLen; i++) {
		vec += (base[i+1] - base[i]);
		limit[i] = vec-1;
		vec <<= 1;
	}
	for (i = minLen + 1; i <= maxLen; i++)
		base[i] = ((limit[i-1] + 1) << 1) - base[i];
}


void makeDecode(int *length,
		int *code,
		int *decode,
		int size)
{
	int i;
	int c;

	for (c = 0; c < size; c++) {
		for (i = 0; i + code[c] < 65536; i += (1 << length[c])) {
			decode[i + code[c]] = c;
		}
	}
}

void compress()
{
	unsigned char in_buffer[BLOCK];
	unsigned char out_buffer[BLOCK];
	int in_len;
	int out_len;
	int freq[MAX_ALPHA_SIZE];
	int length[MAX_ALPHA_SIZE];
	int code[MAX_ALPHA_SIZE];
	int i;
	int min_len;
	int max_len;
	int cur_code;
	int cur_len;

	while ((in_len = fread(in_buffer, 1, sizeof(in_buffer), stdin)) != 0) {
		out_len = 0;
		memset(freq, 0, sizeof(freq));

		for (i = 0; i < in_len; i++) {
			freq[in_buffer[i]]++;
		}

		makeCodeLengths(length, freq, 256, MAX_LEN);

		min_len = 16;
		max_len = 1;
		for (i = 0; i < 256; ++i) {
			if (length[i] < min_len) {
				min_len = length[i];
			}
			if (length[i] > max_len) {
				max_len = length[i];
			}
		}
		
		assignCodes(code, length, min_len, max_len, 256);

		for (i = 0; i < 256; i += 2) {
			out_buffer[out_len++] = length[i] * 16 + length[i + 1];
		}

		cur_code = 0;
		cur_len = 0;
		for (i = 0; i < in_len; i++) {
			cur_code += code[in_buffer[i]] << cur_len;
			cur_len += length[in_buffer[i]];
			
			while (cur_len > 7) {
				out_buffer[out_len++] = cur_code % 256;
				cur_code /= 256;
				cur_len -= 8;
			}
		}

		if (cur_len) {
			out_buffer[out_len++] = cur_code;
		}

		fwrite(&out_len, sizeof(out_len), 1, stdout);
		fwrite(&in_len, sizeof(in_len), 1, stdout);
		fwrite(&out_buffer, 1, out_len, stdout);
	}
}


void decompress()
{
	unsigned char in_buffer[BLOCK];
	unsigned char out_buffer[BLOCK];
	int in_len;
	int out_len;
	int freq[MAX_ALPHA_SIZE];
	int length[MAX_ALPHA_SIZE];
	int code[MAX_ALPHA_SIZE];
	int decode[BLOCK];
	int i;
	int min_len;
	int max_len;
	int cur_code;
	int cur_len;
	int in_pos;
	int out_pos;

	while (1) {
		if (!fread(&in_len, sizeof(in_len), 1, stdin) || !fread(&out_len, sizeof(out_len), 1, stdin)) {
			break;
		}
		fread(in_buffer, 1, in_len, stdin);

		in_pos = 0;
		out_pos = 0;

		for (i = 0; i < 256; i += 2) {
			length[i] = in_buffer[in_pos] / 16;
			length[i + 1] = in_buffer[in_pos] % 16;
			in_pos++;
		}

		min_len = 16;
		max_len = 1;
		for (i = 0; i < 256; ++i) {
			if (length[i] < min_len) {
				min_len = length[i];
			}
			if (length[i] > max_len) {
				max_len = length[i];
			}
		}
		
		assignCodes(code, length, min_len, max_len, 256);

		makeDecode(length, code, decode, 256);

		cur_code = 0;
		cur_len = 0;
		while (out_pos != out_len) {
			while (in_pos < in_len && cur_len < MAX_LEN) {
				cur_code += in_buffer[in_pos++] << cur_len;
				cur_len += 8;
			}

			i = decode[cur_code % 65536];

			out_buffer[out_pos++] = i;
			cur_code >>= length[i];
			cur_len -= length[i];
		}
		fwrite(out_buffer, 1, out_pos, stdout);
	}
}

int main(int argc, char *argv[])
{
	clock_t s, e;

	if (argc == 4) {
		freopen(argv[2], "rb", stdin);
		freopen(argv[3], "wb", stdout);
		s = clock();
		if (strcmp(argv[1], "e") == 0) {
			compress();
			e = clock();
			fprintf(stderr, "%fs\n", ((double)(e - s)) / CLOCKS_PER_SEC);
		} else if (strcmp(argv[1], "d") == 0) {
			decompress();
			e = clock();
			fprintf(stderr, "%fs\n", ((double)(e - s)) / CLOCKS_PER_SEC);
		} else {
			fprintf(stderr, "usage: e/d infile outfile\n");
		}
	} else {
		fprintf(stderr, "usage: e/d infile outfile\n");
	}

	return 0;
}

