/**********************************************************
  $Header$
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include "struct.h"
#include "cid_counters.h"

extern char query_buf[];
extern int query_len;

void bufputc(char byte)
{
    query_buf[query_len] = byte;
    query_len++;
    query_buf[query_len] = '\0';
}

void bufncat(char *bytes, int n)
{
    memcpy(&query_buf[query_len], bytes, n);
    query_len += n;
    query_buf[query_len] = '\0';
}

void print_eol()
{
    bufputc('\n');
}

void print_dat(char *string)
{
    bufncat(string, strlen(string));
}

void print_sub(char *string)
{
    static char head[5];
    static char tail[15];
    int  len;

    strsplit(string, head, tail);
    if (program.longform) {
        bufncat(head, strlen(head));
	bufncat(tail, strlen(tail));
    } else {
	len = strlen(head);
	bufncat(head, strlen(head));
	while (len < 4) {
	    len++;
	    bufputc(' ');
	}
    }
    bufputc(' ');
}

void print_hdr(char *string)
{
    if (program.header) print_sub(string);
}
