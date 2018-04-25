# include <unistd.h>
# include <float.h>
# include <limits.h>
# include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "Maxfiles.h"
#include "MaxSLiCInterface.h"

#include "schedule_utils.h"
#include "tests.h"
#include "prf.h"

#define    LOAD 0
#define    OFFLOAD 1
#define    COPY 2
#define    SCALE 3
#define    ADD 4
#define    TRIAD 5

//Declarations taken from stream benchmark
//TODO set below to 87040
#ifndef STREAM_ARRAY_SIZE
#   define STREAM_ARRAY_SIZE	512
#endif

#ifndef OFFSET
#   define OFFSET	0
#endif

#ifdef NTIMES
#if NTIMES<=1
#   define NTIMES	2
#endif
#endif
#ifndef NTIMES
#   define NTIMES	2
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

#define M 512
#define N 512
#define p 2 
#define q 4

#define TEST 1

double *generateExpectedCOut_Copy_with_schedule(Schedule *s, int schedule_len, double*a,int vectorsize){

    int sizeBytes = vectorsize * sizeof(double);
    double *cExpectedOut = malloc(sizeBytes);

    for(int i=0;i< vectorsize; i++)
        cExpectedOut[i]=-1;

    for(int i=0;i< schedule_len; i++)
        {
          int *mask = s[i].mask; 
           Address2d *access=AGU( s[i].i,s[i].j,2,4,s[i].shape);
            for( int j =0;j<2*4;j++){
                if(mask[j]==1){

                    int index = access[j].i*M+access[j].j;
                    cExpectedOut[index]=a[index];
                }
            }
        }

    return cExpectedOut;

}

double *generateExpectedBOut_Scale_with_schedule(Schedule *s, int schedule_len, double *b, double *c, int vectorsize){

    int sizeBytes = vectorsize * sizeof(double);
    double *bExpectedOut = malloc(sizeBytes);

    for(int i=0;i< vectorsize; i++)
        bExpectedOut[i]=b[i];

    for(int i=0;i< schedule_len; i++)
        {
          int *mask = s[i].mask; 
           Address2d *access=AGU( s[i].i,s[i].j,2,4,s[i].shape);
            for( int j =0;j<2*4;j++){
                if(mask[j]==1){

                    int index = access[j].i*M+access[j].j;
                    bExpectedOut[index]=3*c[index];
                }
            }
        }

    return bExpectedOut;

}
//if ( aOut[i] + bOut[i] != cOut[i]){

double *generateExpectedCOut_ADD_with_schedule(Schedule *s, int schedule_len, double *a,double *b, double *c,int vectorsize){
     int sizeBytes = vectorsize * sizeof(double);
    double *cExpectedOut = malloc(sizeBytes);

    for(int i=0;i< vectorsize; i++)
        cExpectedOut[i]=c[i];

    for(int i=0;i< schedule_len; i++)
        {
          int *mask = s[i].mask; 
           Address2d *access=AGU( s[i].i,s[i].j,2,4,s[i].shape);
            for( int j =0;j<2*4;j++){
                if(mask[j]==1){

                    int index = access[j].i*M+access[j].j;
                    cExpectedOut[index]=a[index]+b[index];
                }
            }
        }

    return cExpectedOut;
}

//if ( aOut[i] != bOut[i] + 3* cOut[i]){
double *generateExpectedAOut_TRIAD_with_schedule(Schedule *s, int schedule_len, double *a,double *b, double *c,int vectorsize){
     int sizeBytes = vectorsize * sizeof(double);
    double *aExpectedOut = malloc(sizeBytes);

    for(int i=0;i< vectorsize; i++)
        aExpectedOut[i]=a[i];

    for(int i=0;i< schedule_len; i++)
        {
          int *mask = s[i].mask; 
           Address2d *access=AGU( s[i].i,s[i].j,2,4,s[i].shape);
            for( int j =0;j<2*4;j++){
                if(mask[j]==1){

                    int index = access[j].i*M+access[j].j;
                    aExpectedOut[index]=b[index]+3*c[index];
                }
            }
        }

    return aExpectedOut;
}


int getTotalNumberOfElementsAccessedbySchedule(Schedule *s, int  schedule_len){
    int accessed_elements=0;
    for(int i=0;i<schedule_len;i++){
        int *mask = s[i].mask; 
        for( int j =0;j<2*4;j++){
            accessed_elements+=mask[j];
        }
    }
    return accessed_elements;
}


int main(int argc, char *argv[])
{
        if(TEST)
            test_STREAM();



        int size = STREAM_ARRAY_SIZE;
        int num_copy=1;
        int schedule_len = 0;
        Schedule *s = NULL;
        int accessed_elements=0; 
        char * scheduleFile="None";
        if(argc>1){ //First input is vectorsize second input is how many time the FPGA executes the copy
            size = atoi(argv[1]);
            for(int i=0;i<6;i++){
                if(i!=2 && i!=3)
                    bytes[i]=3 * sizeof(STREAM_TYPE) * size;//Load and offload use vectors a,b and c so the stream size is 3*vectorsize
                else
                    
                    bytes[i]=2 * sizeof(STREAM_TYPE) * size;//Copy's uses only a and c vectors so the stream size is 2*vectorsize
                }
            if(argc>2){
                num_copy=atoi(argv[2]);
            }
            if(argc > 3 ){//Third input is the location of the path
                scheduleFile = argv[3];
                schedule_len = getFileLenght(scheduleFile);
                s = parseSchedule(scheduleFile);
                if (s==FILE_NOT_FOUND) {
                    printf("FILE_NOT_FOUND");
                    return 1;
                }
                //Recomputing total bytes processed
                accessed_elements=getTotalNumberOfElementsAccessedbySchedule(s, schedule_len);
                for(int i=0;i<6;i++){
                    if(i==0 || i == 1){
                        bytes[i]=3 * sizeof(STREAM_TYPE) * size;//Load and offload use vectors a,b and c so the stream size is 3*vectorsize
                    }else{
                    if(i==4 || i==5)
                        bytes[i]=3 * sizeof(STREAM_TYPE) * accessed_elements;//ADD, and Triad use vectors a,b and c so the stream size is 3*accessed_elements
                    else
                        bytes[i]=2 * sizeof(STREAM_TYPE) * accessed_elements;//Copy and Scale  uses only a and c vectors so the stream size is 2*vectorsize
                    }
                }
            }
       }

        int scheduleROMsize=(M*N/p*q);
        int *scheduleROM=NULL;
        if (s != FILE_NOT_FOUND)
            scheduleROM = compress_schedule_toROM(s,schedule_len,p*q,scheduleROMsize);
        else
            scheduleROM = malloc(scheduleROMsize*sizeof(int));
        FILE *fp;
        if( access( "stream_output.csv", F_OK ) != -1 ) {
            fp  =fopen("stream_output.csv","a");//if file exists append results to existing csv (do not write header)
        }
        else{
            fp  =fopen("stream_output.csv","w");//if file doesnt exists create it and write header
            if(fp==NULL){
                printf("Problems opening the output file stream_output.csv\n");
            }else{
                fprintf(fp,"Schedule File,Seq accessed elements, schedule length,parallel accessed elements,efficiency,Vector size(64bits elements),Load Bytes(B),Load AVG(s),Load Min(s),Load Max(s),Offload Bytes(B),Offload AVG(s),Offload Min(s),Offload Max(s),Copy Bytes(B),Copy AVG(s),Copy Min(s),Copy Max(s),Scale Bytes(B),Scale AVG(s),Scale Min(s),Scale Max(s),Add Bytes(B),Add AVG(s),Add Min(s),Add Max(s),Triad Bytes(B),Triad AVG(s),Triad Min(s),Triad Max(s),Copy Best Rate (MB/s),Scale Best Rate (MB/s),Add Best Rate (MB/s),Triad Best Rate (MB/s)\n");


            }    
        }
        

        // Int 64
        //int sizeBytes = size * sizeof(int64_t);
       // int64_t *a = malloc(sizeBytes);
        //int64_t *b = malloc(sizeBytes);
        //int64_t *c = malloc(sizeBytes);
        //int64_t *aOut = malloc(sizeBytes);
        //int64_t *bOut = malloc(sizeBytes);
        //int64_t *cOut = malloc(sizeBytes);
        //double 
        int sizeBytes = size * sizeof(STREAM_TYPE);
        STREAM_TYPE *a = malloc(sizeBytes);
        STREAM_TYPE *b = malloc(sizeBytes);
        STREAM_TYPE *c = malloc(sizeBytes);
        STREAM_TYPE *aOut = malloc(sizeBytes);
        STREAM_TYPE *bOut = malloc(sizeBytes);
        STREAM_TYPE *cOut = malloc(sizeBytes);

        double		t, times[6][NTIMES];

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
        //Do not use the schedule for loading/Offloading
        prfStreamInput.param_scheduleROMsize=0;
        prfStreamInput.instream_fromcpu_inA=a;
        prfStreamInput.instream_fromcpu_inB=b;
        prfStreamInput.instream_fromcpu_inC=c;
        prfStreamInput.outstream_tocpu_outA=aOut;
        prfStreamInput.outstream_tocpu_outB=bOut;
        prfStreamInput.outstream_tocpu_outC=cOut;
        prfStreamInput.inmem_PRFStreamKernel_ScheduleROM=scheduleROM;
        
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
                        printf("Load/Offload error %d , %d , %d , %d , %d , %d \n",a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                    }

            if(error){
                printf("Errors were found during loading/offloading\n");
                times[1][k] = -1;//Setting time to -1 to signal that an error happened
            }
            //Use the schedule (if provided) to run the STREAM kernel
            prfStreamInput.param_scheduleROMsize=schedule_len;
            printf("Copy benchmark ... \n");
            prfStreamInput.param_prfMode=COPY;
            times[2][k] = mysecond();
            PRFStream_run(StreamDFE,&prfStreamInput);
            times[2][k] =( mysecond() - times[2][k]);
            //Check correctness of copy benchmark
            printf("Checking correctness: Offloading ... \n");
            prfStreamInput.param_scheduleROMsize=0;
            prfStreamInput.param_prfMode=OFFLOAD;
            PRFStream_run(StreamDFE,&prfStreamInput);

            //If there is no schedule check that whole A has been copied in C
            if(schedule_len==0){
                for(int i = 0; i < size; ++i)
                        if ( a[i] != cOut[i]){
                            error=1;
                            printf("Copy error id:%d ->  %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                        }

                if(error){
                    printf("Errors were found during copy benchmark\n");
                    times[2][k] = -1;//Setting time to -1 to signal that an error happened
                }
        
            }else{//Else generate expected C and compare against that.
            
                double *cExpectedOut=generateExpectedCOut_Copy_with_schedule(s,schedule_len,a,size);
               for(int i = 0; i < size; ++i)
                        if ( cOut[i] != cExpectedOut[i]){
                            error=1;
                            printf("Copy error id:%d ->  %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                            printf("Expected C : %f\n", cExpectedOut[i]);
                        }

                if(error){
                    printf("Errors were found during copy benchmark\n");
                    times[2][k] = -1;//Setting time to -1 to signal that an error happened
                }
     
               free(cExpectedOut); 
            }
            printf("Scale benchmark ... \n");
            prfStreamInput.param_scheduleROMsize=schedule_len;
            prfStreamInput.param_prfMode=SCALE;
            times[3][k] = mysecond();
            PRFStream_run(StreamDFE,&prfStreamInput);
            times[3][k] = mysecond() - times[3][k];
            //Check correctness of copy benchmark
            printf("Checking correctness: Offloading ... \n");
            prfStreamInput.param_scheduleROMsize=0;
            prfStreamInput.param_prfMode=OFFLOAD;
            PRFStream_run(StreamDFE,&prfStreamInput);

            if(schedule_len==0){
            for(int i = 0; i < size; ++i)
                    if ( bOut[i] != 3* cOut[i]){
                        error=1;
                        printf("Scale error index :%d  %d , %d , %d , %d , %d , %d \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                    }

            if(error){
                printf("Errors were found during scale benchmark\n");
                times[3][k] = -1;//Setting time to -1 to signal that an error happened
            }
            }else{
               double *bExpectedOut=generateExpectedBOut_Scale_with_schedule(s, schedule_len, bOut, cOut,size);
               for(int i = 0; i < size; ++i)
                        if ( bOut[i] != bExpectedOut[i]){
                            error=1;
                            printf("Copy error id:%d ->  %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                            printf("Expected B : %f\n", bExpectedOut[i]);
                        }

                if(error){
                    printf("Errors were found during scale benchmark\n");
                    times[3][k] = -1;//Setting time to -1 to signal that an error happened
                }
     
               free(bExpectedOut); 
            }

            printf("ADD benchmark ... \n");
            prfStreamInput.param_scheduleROMsize=schedule_len;
            prfStreamInput.param_prfMode=ADD;
            times[4][k] = mysecond();
            PRFStream_run(StreamDFE,&prfStreamInput);
            times[4][k] = mysecond() - times[4][k];
            //Check correctness of copy benchmark
            printf("Checking correctness: Offloading ... \n");
            prfStreamInput.param_scheduleROMsize=0;
            prfStreamInput.param_prfMode=OFFLOAD;
            PRFStream_run(StreamDFE,&prfStreamInput);

            if(schedule_len==0){
                for(int i = 0; i < size; ++i)
                        if ( aOut[i] + bOut[i] != cOut[i]){
                            error=1;
                            printf("ADD error index: %d ->%d , %d , %d , %d , %d , %d \n",a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                        }

                if(error){
                    printf("Errors were found during add benchmark\n");
                    times[4][k] = -1;//Setting time to -1 to signal that an error happened
                }
            }else{
               double *cExpectedOut=generateExpectedCOut_ADD_with_schedule(s,schedule_len,aOut,bOut,cOut,size);
               for(int i = 0; i < size; ++i)
                        if ( cOut[i] != cExpectedOut[i]){
                            error=1;
                            printf("Copy error id:%d ->  %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                            printf("Expected C : %f\n", cExpectedOut[i]);
                        }

                if(error){
                    printf("Errors were found during add benchmark\n");
                    times[4][k] = -1;//Setting time to -1 to signal that an error happened
                }
     
               free(cExpectedOut); 

            }
            printf("TRIAD benchmark ... \n");
            prfStreamInput.param_scheduleROMsize=schedule_len;
            prfStreamInput.param_prfMode=TRIAD;
            times[5][k] = mysecond();
            PRFStream_run(StreamDFE,&prfStreamInput);
            times[5][k] = mysecond() - times[5][k];
            //Check correctness of copy benchmark
            printf("Checking correctness: Offloading ... \n");
            prfStreamInput.param_scheduleROMsize=0;
            prfStreamInput.param_prfMode=OFFLOAD;
            PRFStream_run(StreamDFE,&prfStreamInput);

            if(schedule_len==0){
                for(int i = 0; i < size; ++i)
                        if ( aOut[i] != bOut[i] + 3* cOut[i]){
                            error=1;
                            printf("TRIAD error index:%d-> %d , %d , %d , %d , %d , %d \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                        }

                if(error){
                    printf("Errors were found during triad benchmark\n");
                    times[5][k] = -1;//Setting time to -1 to signal that an error happened
                }
            }else{
              double *aExpectedOut=generateExpectedAOut_TRIAD_with_schedule(s,schedule_len,aOut,bOut,cOut,size);
               for(int i = 0; i < size; ++i)
                        if ( aOut[i] != aExpectedOut[i]){
                            error=1;
                            printf("TRIAD  error id:%d ->  %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                            printf("Expected A : %f\n", aExpectedOut[i]);
                        }

                if(error){
                    printf("Errors were found during TRIAD benchmark\n");
                    times[5][k] = -1;//Setting time to -1 to signal that an error happened
                }
     
               free(aExpectedOut);      
            }
        }
        for (int k=1; k<NTIMES; k++) /* note -- skip first iteration */
        {
            for (int j=0; j<6; j++) {
            avgtime[j] = avgtime[j] + times[j][k];
            mintime[j] = MIN(mintime[j], times[j][k]);
            maxtime[j] = MAX(maxtime[j], times[j][k]);
            }
        }
        printf("Function    Best Rate MB/s  Avg time     Min time     Max time     Bytes\n");
        for (int j=0; j<6; j++) {
            avgtime[j] = avgtime[j]/(double)(NTIMES-1);
            if(j<2){
            printf("%s%12.1f  %11.6f  %11.6f  %11.6f  %f\n", label[j],
               1.0E-06 * bytes[j]/mintime[j],
               avgtime[j],
               mintime[j],
               maxtime[j],
                bytes[j]);
            }else{ //consider how many times the operation was performed
               printf("%s%12.1f  %11.6f  %11.6f  %11.6f  %f\n", label[j],
               1.0E-06* num_copy * bytes[j]/mintime[j],
               avgtime[j]/num_copy,
               mintime[j]/num_copy,
               maxtime[j]/num_copy,
                bytes[j]);
     
            }
        }
        
        printf(HLINE);
        float efficiency=0;
        if(schedule_len>0){
            efficiency=accessed_elements*100/((float)(schedule_len*8));
        
            printf("seq accessed_elements: %d, schedule_len: %d, par accessed_elements: %d, efficiency:%f%\n",accessed_elements,schedule_len,schedule_len*8,efficiency);
        }
        else{
            efficiency=100;
            printf("seq accessed_elements: %d, schedule_len: %d, par accessed_elements: %d, efficiency:%f%\n",size,schedule_len,size,efficiency);
        }
            
        printf("Done.\n");
        //Write output to csv
        if(schedule_len>0){
            if(fp!=NULL){
                fprintf(fp,"%s,%d,%d,%d,%f,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
                        scheduleFile,accessed_elements,schedule_len,schedule_len*8,efficiency,
                        size,bytes[0],avgtime[0],mintime[0],maxtime[0],
                        bytes[1],avgtime[1],mintime[1],maxtime[1],
                        bytes[2],avgtime[2]/num_copy,mintime[2]/num_copy,maxtime[2]/num_copy,
                        bytes[3],avgtime[3]/num_copy,mintime[3]/num_copy,maxtime[3]/num_copy,
                        bytes[4],avgtime[4]/num_copy,mintime[4]/num_copy,maxtime[4]/num_copy,
                        bytes[5],avgtime[5]/num_copy,mintime[5]/num_copy,maxtime[5]/num_copy,
                       1.0E-06* num_copy * bytes[2]/mintime[2],
                       1.0E-06* num_copy * bytes[3]/mintime[3],
                       1.0E-06* num_copy * bytes[4]/mintime[4],
                       1.0E-06* num_copy * bytes[5]/mintime[5]
                        );
                fclose(fp);
            }
        }else{
             if(fp!=NULL){
                fprintf(fp,"%s,%d,%d,%d,%f,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",scheduleFile,size,schedule_len,size,efficiency,size,bytes[0],avgtime[0],mintime[0],maxtime[0],bytes[1],avgtime[1],mintime[1],maxtime[1],
                        bytes[2],avgtime[2]/num_copy,mintime[2]/num_copy,maxtime[2]/num_copy,
                        bytes[3],avgtime[3]/num_copy,mintime[3]/num_copy,maxtime[3]/num_copy,
                        bytes[4],avgtime[4]/num_copy,mintime[4]/num_copy,maxtime[4]/num_copy,
                        bytes[5],avgtime[5]/num_copy,mintime[5]/num_copy,maxtime[5]/num_copy,
                       1.0E-06* num_copy * bytes[2]/mintime[2],
                       1.0E-06* num_copy * bytes[3]/mintime[3],
                       1.0E-06* num_copy * bytes[4]/mintime[4],
                       1.0E-06* num_copy * bytes[5]/mintime[5]
                        );
                fclose(fp);
                //fprintf(fp,"%s,%d,%d,%d,%f,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n","None",size,schedule_len,size,efficiency,size,bytes[0],avgtime[0],mintime[0],maxtime[0],bytes[1],avgtime[1],mintime[1],
                  //      maxtime[1],bytes[2],avgtime[2],mintime[2],maxtime[2]);
                //fprintf(fp,"%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",size,bytes[0],avgtime[0],mintime[0],maxtime[0],bytes[1],avgtime[1],mintime[1],maxtime[1],bytes[2]
                   // ,avgtime[2],mintime[2],maxtime[2]);
            }
        }
        return 0;
}

