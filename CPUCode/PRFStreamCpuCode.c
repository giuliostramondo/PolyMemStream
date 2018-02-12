# include <unistd.h>
# include <float.h>
# include <limits.h>
# include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"

#define    LOAD 0
#define    OFFLOAD 1
#define    COPY 2
#define    SCALE 3
#define    ADD 4
#define    TRIAD 5

//Declarations taken from stream benchmark
#ifndef STREAM_ARRAY_SIZE
#   define STREAM_ARRAY_SIZE	87040
#endif

#ifndef OFFSET
#   define OFFSET	0
#endif

#ifdef NTIMES
#if NTIMES<=1
#   define NTIMES	10
#endif
#endif
#ifndef NTIMES
#   define NTIMES	10
#endif

# define HLINE "-------------------------------------------------------------\n"

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif


#define STREAM_TYPE double


static double	avgtime[6] = {0}, maxtime[6] = {0},
		mintime[6] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};

static char	*label[6] = {"Load:      ","Offload:   ","Copy:      ", "Scale:     ",
    "Add:       ", "Triad:     "};

static double	bytes[6] = {
    3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE
    };

double mysecond()
{
/* struct timeval { long        tv_sec;
            long        tv_usec;        };

struct timezone { int   tz_minuteswest;
             int        tz_dsttime;      };     */

        struct timeval tp;
        struct timezone tzp;
        int i;

        i = gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}


int main(int argc, char *argv[])
{
        int size = STREAM_ARRAY_SIZE;
        int num_copy=1;
        FILE *fp;
        if( access( "stream_output.csv", F_OK ) != -1 ) {
            fp  =fopen("stream_output.csv","a");//if file exists append results to existing csv (do not write header)
        }
        else{
            fp  =fopen("stream_output.csv","w");//if file doesnt exists create it and write header
            if(fp==NULL){
                printf("Problems opening the output file stream_output.csv\n");
            }else{
                fprintf(fp,"Vector size(64bits elements),Load Bytes(B),Load AVG(s),Load Min(s),Load Max(s),Offload Bytes(B),Offload AVG(s),Offload Min(s),Offload Max(s),Copy Bytes(B),Copy AVG(s),Copy Min(s),Copy Max(s)\n");
            }    
        }
        
        if(argc>1){ //First input is vectorsize second input is how many time the FPGA executes the copy
            size = atoi(argv[1]);
            for(int i=0;i<6;i++){
                if(i!=2)
                    bytes[i]=3 * sizeof(STREAM_TYPE) * size;//Load and offload use vectors a,b and c so the stream size is 3*vectorsize
                else
                    
                    bytes[i]=2 * sizeof(STREAM_TYPE) * size;//Copy's uses only a and c vectors so the stream size is 2*vectorsize
                }
            if(argc>2){
                num_copy=atoi(argv[2]);
            }
           }
        int sizeBytes = size * sizeof(int64_t);
        int64_t *a = malloc(sizeBytes);
        int64_t *b = malloc(sizeBytes);
        int64_t *c = malloc(sizeBytes);
        int64_t *aOut = malloc(sizeBytes);
        int64_t *bOut = malloc(sizeBytes);
        int64_t *cOut = malloc(sizeBytes);
        
        double		t, times[4][NTIMES];

        // Generate input data
        for(int i = 0; i < size; ++i) {
                a[i] = i;
                b[i] = 1;
                c[i] = -1;
        }

        //Initialize Hardware design Parameters
        PRFStream_actions_t prfStreamInput;
        prfStreamInput.param_VEC_SIZE=size;
        prfStreamInput.param_copy_repeats= num_copy;
        prfStreamInput.param_prfMode=LOAD;
        prfStreamInput.instream_aStream=a;
        prfStreamInput.instream_bStream=b;
        prfStreamInput.instream_cStream=c;
        prfStreamInput.outstream_aOutStream=aOut;
        prfStreamInput.outstream_bOutStream=bOut;
        prfStreamInput.outstream_cOutStream=cOut;
        
        //Load bitstream on fpga
        max_file_t* StreamMaxFile =  PRFStream_init();
        max_engine_t* StreamDFE=max_load(StreamMaxFile,"*");



        
        printf("Running on DFE.\n");

        for(int k=0;k<NTIMES; k++)
        {
            printf("Loading ... \n");
            prfStreamInput.param_prfMode=LOAD;
            times[0][k] = mysecond();
            PRFStream_run(StreamDFE,&prfStreamInput);
            times[0][k] = mysecond() - times[0][k];

            printf("Offloading ... \n");
            prfStreamInput.param_prfMode=OFFLOAD;
            times[1][k] = mysecond();
            PRFStream_run(StreamDFE,&prfStreamInput);
            times[1][k] = mysecond() - times[1][k];

            printf("Checking correctness load/offload ... \n");
            //Check correctness of load/offload
            int error=0;
            for(int i = 0; i < size; ++i)
                    if ( a[i] != aOut[i] || b[i] != bOut[i] || c[i] != cOut[i]){
                        error=1;
                        printf("error %d , %d , %d , %d , %d , %d \n",a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                    }

            if(error){
                printf("Errors were found during loading/offloading\n");
                times[1][k] = -1;//Setting time to -1 to signal that an error happened
            }
            printf("Copy benchmark ... \n");
            prfStreamInput.param_prfMode=COPY;
            times[2][k] = mysecond();
            PRFStream_run(StreamDFE,&prfStreamInput);
            times[2][k] =( mysecond() - times[2][k]);
            //Check correctness of copy benchmark
            printf("Checking correctness: Offloading ... \n");
            prfStreamInput.param_prfMode=OFFLOAD;
            PRFStream_run(StreamDFE,&prfStreamInput);

            for(int i = 0; i < size; ++i)
                    if ( a[i] != cOut[i]){
                        error=1;
                        printf("error %d , %d , %d , %d , %d , %d \n",a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                    }

            if(error){
                printf("Errors were found during copy benchmark\n");
                times[2][k] = -1;//Setting time to -1 to signal that an error happened
            }
        }

 
        for (int k=1; k<NTIMES; k++) /* note -- skip first iteration */
        {
            for (int j=0; j<3; j++) {
            avgtime[j] = avgtime[j] + times[j][k];
            mintime[j] = MIN(mintime[j], times[j][k]);
            maxtime[j] = MAX(maxtime[j], times[j][k]);
            }
        }
        printf("Function    Best Rate MB/s  Avg time     Min time     Max time     Bytes\n");
        for (int j=0; j<3; j++) {
            avgtime[j] = avgtime[j]/(double)(NTIMES-1);

            printf("%s%12.1f  %11.6f  %11.6f  %11.6f  %f\n", label[j],
               1.0E-06 * bytes[j]/mintime[j],
               avgtime[j],
               mintime[j],
               maxtime[j],
                bytes[j]);
        }
        
        printf(HLINE);
        printf("Done.\n");
        //Write output to csv
        if(fp!=NULL){
            fprintf(fp,"%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",size,bytes[0],avgtime[0],mintime[0],maxtime[0],bytes[1],avgtime[1],mintime[1],maxtime[1],bytes[2]
                ,avgtime[2],mintime[2],maxtime[2]);
            fclose(fp);
        }
        return 0;
}

