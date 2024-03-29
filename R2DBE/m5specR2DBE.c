/***************************************************************************
 *   Copyright (C) 2008-2011 by Walter Brisken & Chris Phillips            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id: m5spec.c 5956 2014-02-17 04:08:42Z ChrisPhillips $
// $HeadURL: https://svn.atnf.csiro.au/difx/libraries/mark5access/trunk/mark5access/mark5_stream.c $
// $LastChangedRevision: 5956 $
// $Author: ChrisPhillips $
// $LastChangedDate: 2014-02-16 23:08:42 -0500 (Sun, 16 Feb 2014) $
//
//============================================================================

// Change this to configure detection, if possible
#define USEGETOPT 1

#include "config.h"
#include <stdio.h>
#include <complex.h>
#include <stdlib.h>
#include <string.h>
#include <fftw3.h>
#include <math.h>
#include <signal.h>
#include "../mark5access/mark5_stream.h"

#if USEGETOPT
#include <getopt.h>
#endif

const char program[] = "m5spec";
const char author[]  = "Walter Brisken, Chris Phillips";
const char version[] = "1.3.1";
const char verdate[] = "20120508";

int die = 0;

typedef enum {VLBA=1, DBBC, NOPOL} polmodetype;

typedef void (*sighandler_t)(int);

sighandler_t oldsiginthand;

void siginthand(int j)
{
	printf("\nBeing killed.  Partial results will be saved.\n\n");
	die = 1;

	signal(SIGINT, oldsiginthand);
}

static void usage(const char *pgm)
{
	printf("\n");

	printf("%s ver. %s   %s  %s\n\n", program, version, author, verdate);
	printf("A Mark5 spectrometer.  Adapted for R2DBE 1-IF 2048 MHz data (m5spec fails on frequency axis).\n\n");
	printf("Usage : %s <infile> <dataformat (ignored)> <nchan> <nint> <outfile> [<offset>]\n\n", program);
	printf("  <infile> is the name of the input file\n\n");
	printf("  <dataformat> should be of the form: "
		"<FORMAT>-<Mbps>-<nchan>-<nbit>, e.g.:\n");
	printf("    VDIF_8192-8192-1-2 (here the 1st 8192 is payload size in bytes)\n\n");
	printf("  <nchan> is the number of channels to make per IF\n\n");
	printf("  <nint> is the number of FFT frames to spectrometize\n\n");
	printf("  <outfile> is the name of the output file\n\n");
	printf("  <offset> is number of bytes into file to start decoding\n\n");
	printf("\n\n\n");
	printf("The following options are supported\n\n");
	printf("    -help      This list\n\n");
	printf("The folllowing options are supported\n\n");
}

int harvestRealData(struct mark5_stream *ms, double **spec, fftw_complex **zdata, fftw_complex **zx, int nchan, int nint, int chunk, long long *total, long long *unpacked, polmodetype polmode)
{
	fftw_plan *plan;
	double **data;
	int j;

	plan = (fftw_plan *)malloc(ms->nchan*sizeof(fftw_plan));
	data = (double **)malloc(ms->nchan*sizeof(double *));
	for(j = 0; j < ms->nchan; ++j)
	{
		data[j] = (double *)malloc((chunk+2)*sizeof(double));
		plan[j] = fftw_plan_dft_r2c_1d(nchan*2, data[j], zdata[j], FFTW_MEASURE);
	}
	for(j = 0; j < nint; ++j)
	{
		int status;
		int i;

		if(die)
		{
			break;
		}

		status = mark5_stream_decode_double(ms, chunk, data);
		if(status < 0)
		{
			break;
		}
		else
		{
			*total += chunk;
			*unpacked += status;
		}

		if(ms->consecutivefails > 5)
		{
			break;
		}

		for(i = 0; i < ms->nchan; ++i)
		{
			/* FFT */
			fftw_execute(plan[i]);
		}

		for(i = 0; i < ms->nchan; ++i)
		{
			int c;

			for(c = 0; c < nchan; ++c)
			{
				double re, im;
				
				re = creal(zdata[i][c]);
				im = cimag(zdata[i][c]);
				spec[i][c] += re*re + im*im;
			}
		}

		if (polmode==VLBA) 
		{
		  for(i = 0; i < ms->nchan/2; ++i)
		  {
			int c;

			for(c = 0; c < nchan; ++c)
			{
				zx[i][c] += zdata[2*i][c]*~zdata[2*i+1][c];
			}
		  }
		} 
		else if (polmode==DBBC) 
		{
		  for(i = 0; i < ms->nchan/2; ++i)
		  {
			int c;

			for(c = 0; c < nchan; ++c)
			{
				zx[i][c] += zdata[i][c]*~zdata[i+ms->nchan/2][c];
			}
		  }
		}
	}
	for(j = 0; j < ms->nchan; ++j)
	{
		fftw_destroy_plan(plan[j]);
		free(data[j]);
	}
	free(plan);
	free(data);

	return 0;
}


int spec(const char *filename, const char *formatname, int nchan, int nint, const char *outfile, long long offset, polmodetype polmode, int doublesideband)
{
	struct mark5_stream *ms;
	double **spec;
	fftw_complex **zdata, **zx;
	int i, c;
	int chunk;
	long long total, unpacked;
	FILE *out;
	double f, sum, chanbw;
	double x, y;
	int docomplex;

	total = unpacked = 0;

	ms = new_mark5_stream_absorb(
		new_mark5_stream_file(filename, offset),
		new_mark5_format_generic_from_string(formatname) );

	if(!ms)
	{
		printf("Error: problem opening %s\n", filename);

		return EXIT_FAILURE;
	}

	mark5_stream_print(ms);

	docomplex = 0;
	chunk = 2*nchan;

	out = fopen(outfile, "w");
	if(!out)
	{
		fprintf(stderr, "Error: cannot open %s for write\n", outfile);
		delete_mark5_stream(ms);

		return EXIT_FAILURE;
	}

	spec = (double **)malloc(ms->nchan*sizeof(double *));
	zdata = (fftw_complex **)malloc(ms->nchan*sizeof(fftw_complex *));
	zx = (fftw_complex **)malloc((ms->nchan/2)*sizeof(fftw_complex *));
	for(i = 0; i < ms->nchan; ++i)
	{
		spec[i] = (double *)calloc(nchan, sizeof(double));
		zdata[i] = (fftw_complex *)malloc((nchan+2)*sizeof(fftw_complex));
	}
	for(i = 0; i < ms->nchan/2; ++i)
	{
		zx[i] = (fftw_complex *)calloc(nchan, sizeof(fftw_complex));
	}

	harvestRealData(ms, spec, zdata, zx, nchan, nint, chunk, &total, &unpacked, polmode);

	fprintf(stderr, "%Ld / %Ld samples unpacked\n", unpacked, total);

	/* normalize across all ifs/channels */
	sum = 0.0;
	for(c = 0; c < nchan; ++c)
	{
		for(i = 0; i < ms->nchan; ++i)
		{
			sum += spec[i][c];
		}
	}

	f = ms->nchan*nchan/sum;

	// R2DBE: 2048 MHz bandwidth (too wide for mark5access, numerical errors)
	chanbw = 2*2048e6/(2e6*nchan);

	for(c = 0; c < nchan; ++c)
	{
		fprintf(out, "%f ", (double)c*chanbw);
		for(i = 0; i < ms->nchan; ++i)
		{
			fprintf(out, " %f", f*spec[i][c]);
		}
		if (polmode!=NOPOL)
		{
		        for(i = 0; i < ms->nchan/2; ++i)
			{
			        x = creal(zx[i][c])*f;
				y = cimag(zx[i][c])*f;
				fprintf(out, "  %f %f", sqrt(x*x+y*y), atan2(y, x));
			}
		}
		fprintf(out, "\n");
	}

	fclose(out);

	for(i = 0; i < ms->nchan; ++i)
	{
		free(zdata[i]);
		free(spec[i]);
	}
	for(i = 0; i < ms->nchan/2; ++i)
	{
		free(zx[i]);
	}
	free(zx);
	free(zdata);
	free(spec);
	delete_mark5_stream(ms);

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	long long offset = 0;
	int nchan, nint;
	int retval;
	polmodetype polmode = VLBA;
	int doublesideband = 0;

#if USEGETOPT
	int opt;
	struct option options[] = {
	  {"help", 0, 0, 'h'},
	  {0, 0, 0, 0}
	};

	while ((opt = getopt_long_only(argc, argv, "h", options, NULL)) != EOF)
	  switch (opt) {
	  case 'h': // help
	    usage(argv[0]);
	    return EXIT_SUCCESS;
	    break;
	  }

#else
	int optind=1;
#endif
	// R2DBE:
	polmode = NOPOL;
	doublesideband = 0;

	oldsiginthand = signal(SIGINT, siginthand);

	if (argc-optind == 1)
	{
		struct mark5_format *mf;
		int bufferlen = 1<<11;
		char *buffer;
		FILE *in;
		int r;

		if(strcmp(argv[1], "-h") == 0 ||
		   strcmp(argv[1], "--help") == 0)
		{
			usage(argv[0]);

			return EXIT_SUCCESS;
		}

		in = fopen(argv[1], "r");
		if(!in)
		{
			fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
			
			return EXIT_FAILURE;
		}

		buffer = malloc(bufferlen);

		r = fread(buffer, bufferlen, 1, in);
		if(r < 1)
		{
			fprintf(stderr, "Error, buffer read failed.\n");

			fclose(in);
			free(buffer);

			return EXIT_FAILURE;
		}
		else
		{
			mf = new_mark5_format_from_stream(new_mark5_stream_memory(buffer, bufferlen/2));

			print_mark5_format(mf);
			delete_mark5_format(mf);

			mf = new_mark5_format_from_stream(new_mark5_stream_memory(buffer, bufferlen/2));

			print_mark5_format(mf);
			delete_mark5_format(mf);
		}

		fclose(in);
		free(buffer);

		return EXIT_SUCCESS;
	}

	else if(argc-optind < 5)
	{
		usage(argv[0]);

		return EXIT_FAILURE;
	}

	nchan = atol(argv[optind+2]);
	nint  = atol(argv[optind+3]);
	if(nint <= 0)
	{
		nint = 2000000000L;
	}

	if(argc-optind > 5)
	{
		offset=atoll(argv[optind+5]);
	}

	retval = spec(argv[optind], "VDIF_8192-8192-1-2", nchan, nint, argv[optind+4], offset,
		      polmode, doublesideband);

	return retval;
}

