/***********************************************

Author contact information: 
    Michael J. Black
    Department of Computer Science
    Brown University 
    P.O. Box 1910
    Providence, RI 02912 

    http://www.cs.brown.edu/people/black/
    email: black@cs.brown.edu

    401-863-7637 (voice) 
    401-863-7657 (fax) 

---------------------------------------------------------------------
Copyright 1993, Michael J. Black

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any non-commercial purpose is hereby granted without
fee, provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the author not be used
in advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

For commercial use and licensing information contact the author
at the address above
---------------------------------------------------------------------

File: gnc.c

Synopsis: Perform graduated non-convexity to recover
          optical flow for two images.

Status: This is EXPERIMENTAL code.  There is no guarantee.
        I'm not a C hacker, so if I am doing stupid things
	please enlighten me.

Details:

This is the main program (complied into a file called "gnc") for
estimating optical flow.  It takes the following required inputs from
standard input:

  gnc <first-frame> <second-frame> <max-pyr> <min-pyr> <in-path> <out-path>

where:
 
  * first-frame and second-frame are integer frame numbers (assuming
    that your image sequences is numbered).

  * max and min pyramid really could be a single number (max - min)
    which says how many layers you want in the pyramid.  So for
    a three level pyramid, you would have max = 4, min = 2, for
    example.  

  * in-path and out-path give the paths for the input and output files.
    Input files should be byte files with no header.  The frame number
    will be concatenated to the path to give <inpath><frame> as the
    input file name.

    The output files saved are <outpath>-*-<frame>, where * is
       "u": horizontal flow component
       "v": vertical flow
       "discont": spatial discontinuities

There are a number of optional command line arguments:

   -l1, -l2, -l3: These weight the data, spatial, and temporal
                  terms respectively.  Float.  
		  Usually l1=l2=l3=1.0.

   -S1, -S2, -S3: These are the INTIAL scale parameters for the
                  robust estimator (for the data, spatial, and 
		  temporal terms respectively).  These should be
		  chosen large enough to ensure that the objective
		  function is convex.
	       
   -s1, -s2, -s3: These are the FINAL scale parameters for the
                  robust estimator (for the data, spatial, and 
		  temporal terms respectively).  These should be
		  chosen to reflect what is considered an outlier.

   -f:  A float that indicates the rate that S2, S2, and S3 are 
        decreased each frame.  So S1 at time t is S1 * "f" at t-1.

   -nx, -ny:  These are the x and y dimensions of the images (integer).

   -F: If 1, then the images are prefiltered with a Laplacian, otherwise
       they are not.

   -w: The "overrelaxation" parameter (0 < w < 2).  Typically
       w = 1.995.

   -iters:  The number of iterations of relaxation per frame.
            Usually between 1 and 20 for most sequences.  If you
	    have just a two image sequence this will probably be
	    much higher.

   -end: If your input files don't look like <in-path><frame>
         but have a tail, then this is the value of end.  So, if
	 end is specified the input path is <in-path><frame><end>.

Example:

  gnc 0 1 4 4 
      /usr/people/black/data/pepsi/pepsi- 
      foo- 
      -l1 1.0 -l2 1.0 
      -S1 10.0 -S2 1.0 
      -s1 1.0 -s2 0.05 
      -nx 64 -ny 64 
      -F 0 -w 1.995 
      -iters 50

************************************************/


# include       <stdio.h>
# include       <math.h>
# include       <string.h>

# include "robust.h"
#include "calflow.h"


//-l1 1.0 -l2 1.0 
//-S1 10.0 -S2 1.0 
//-s1 1.0 -s2 0.05 
//-nx 64 -ny 64 
//-F 0 -w 1.995 
//-iters 50




 void calFlow(float* prev,float* curr,float* u,float* v,int from,int to,int max_level,int min_level,int nx,int ny)
{
  float lambda1=10.0, lambda2=1.0, lambda3=0.0, l3=0.05;
  float s1=10.0, s2=1.0, s3=1.0;
  float s1_end=10.0, s2_end=0.05, s3_end=1.0;
  //float factor=0.75;
  float factor=0.5;
  float s1_rate, s2_rate, s3_rate;
  float omega=1.995;
  char *inpath, *outpath, *flag, *end;
  int index, im, by=1;
  int iters = 40, filter=0, skip=15;
  int stages = 10, st;
  float s1_factor, s2_factor;

  unsigned char *uOut, *vOut;
  float *dx, *dy, *dt;//, *u, *v;
  float *u_scale, *v_scale, *du, *dv;
  float *prev_u, *prev_v, *prev_du, *prev_dv;
  float *lap1, *lap2, *err;
  float *discont, *outliers, *sigma1, *sigma2, 
  *sigma3, *temp;
  float maxAmp;
  int fdOutu, fdOutv, fdOutdiscont, fdOuttemp;
  int sizeInput, imageSize, sizeDeriv, sizeOutput;
  int start, n, i, j, i0, ix, iy, ixf, iyf, itmp, icount;
  char fnCurr[MAXLEN];
  char fnOutu[MAXLEN], fnOutv[MAXLEN], fnOutdiscont[MAXLEN], fnOuttemp[MAXLEN];
  float image_abs_max(), *load_image(); 
  double atof(); 
  char *itoa();
  void reverse();

  fprintf(stderr, "allocating memory\n");
  sizeDeriv = nx * ny * sizeof( float );
  sizeOutput = nx * ny * sizeof( char );

  if ((err = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "err");
  if ((discont = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "discont");
  if ((outliers = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "outliers");
  if ((sigma1 = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "sigma1");
  if ((sigma2 = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "sigma2");
  if ((sigma3 = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "sigma3");
  //if ((prev = (float *) malloc((size_t) sizeDeriv)) == NULL)
  //    error(" Unable to allocate memory for ", "prev");
  //if ((curr = (float *) malloc((size_t) sizeDeriv)) == NULL)
  //    error(" Unable to allocate memory for ", "curr");
 /* if ((u = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "u");
  if ((v = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "v");*/
  if ((du = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "du");
  if ((dv = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "dv");
  if ((prev_u = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "prev_u");
  if ((prev_v = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "prev_v");
  if ((prev_du = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "prev_du");
  if ((prev_dv = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "prev_dv");
  if ((u_scale = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "u_scale");
  if ((v_scale = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "v_scale");
  if ((dx = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "dx");
  if ((dy = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "dy");
  if ((dt = (float *) malloc((size_t) sizeDeriv)) == NULL)
      error(" Unable to allocate memory for ", "dt");

  /* initialize u, v */
  /* this should be fixed to allow prev_u and prev_v to be
     entered from a file
     */
  fprintf(stderr, "initializing u,v,du,dv\n");
  for(i=0;i<ny;i++){
    for(j=0;j<nx;j++){
      index = (i * nx) + j;
      //prev[index] = 0.0;
      //curr[index] = 0.0;
      //u[index] = 0.0;
      //v[index] = 0.0;
      du[index] = 0.0;
      dv[index] = 0.0;
      prev_u[index] = 0.0;
      prev_v[index] = 0.0;
      prev_du[index] = 0.0;
      prev_dv[index] = 0.0;
      outliers[index] = 0.0;
      sigma1[index] = s1;
      sigma2[index] = s2;
      sigma3[index] = s3;
    }
  }
  /* no need for the temporal constraint */
  l3 = 0.0;
  s1_factor = pow((s1_end/s1), (1.0/ (float)(stages -1)));
  s2_factor = pow((s2_end/s2), (1.0/ (float)(stages -1)));
  for(st=0; st<stages; st++) {
    fprintf(stderr, "Stage: %d\n  s1: %f\n  s2: %f\n\n",
	    st, s1, s2);
    /* update flow */
    //printf("calculate:%d",st);
    pyramid_sor(prev, curr, max_level, min_level, iters, omega, u, v, 
		prev_u, prev_v, du, dv, lambda1, lambda2,
		lambda3, sigma1, sigma2, sigma3, nx, ny, dx, dy, dt, err,
		u_scale, v_scale, filter);
    s1 = max((s1 * s1_factor), s1_end);
    s2 = max((s2 * s2_factor), s2_end);
    for(i=0;i<ny;i++){
      for(j=0;j<nx;j++){
	index = (i*nx) +j;
	sigma1[index] = s1;
	sigma2[index] = s2;
      }
    }
    copy_image(prev_u, u, nx, ny);
    copy_image(prev_v, v, nx, ny);
  }
  free(err);
  free(discont);
  free(outliers);
  free(sigma1);
  free(sigma2);
  free(sigma3);
  free(du);
  free(dv);
  free(prev_u);
  free(prev_v);
  free(prev_du);
  free(prev_dv);
  free(u_scale);
  free(v_scale);
  free(dx);
  free(dy);
  free(dt);
}

