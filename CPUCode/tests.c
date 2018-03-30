#include "tests.h"

#define VERBOSE 1

#define STREAM_ARRAY_SIZE	32

#define    LOAD 0
#define    OFFLOAD 1
#define    COPY 2
#define    SCALE 3
#define    ADD 4
#define    TRIAD 5

void failed(){
        printf("[ \x1B[31mFailed\e[0m ]\n");
}
void succeded(){
        printf("[ \x1B[32mSucceded\e[0m ]\n");
}

int run_testsuite(Testsuite* tests, int testnb){
 int i =0;
 int error = 0;
 int align = -82;

 for ( i = 0 ; i < testnb; i++){
    printf("%s %*s", " Testing",align,tests[i].name);
    if ((*(tests[i].function))()){
        failed();
        error = 1;
    }
    else
        succeded();
 }

 return error;
}


int test_load_offload(){
    int sizeBytes = STREAM_ARRAY_SIZE * sizeof(double);
    double *a = malloc(sizeBytes);
    double *b = malloc(sizeBytes);
    double *c = malloc(sizeBytes);
    double *aOut = malloc(sizeBytes);
    double *bOut = malloc(sizeBytes);
    double *cOut = malloc(sizeBytes);

    int64_t * scheduleROM = malloc((512*512/8)*sizeof(int64_t));

    // Generate input data
    for(int i = 0; i < STREAM_ARRAY_SIZE; ++i) {
            a[i] = i;
            b[i] = 1;
            c[i] = -1;
    }

    PRFStream_actions_t prfStreamInput;
    prfStreamInput.param_VEC_SIZE=STREAM_ARRAY_SIZE;
    prfStreamInput.param_copy_repeats= 1;
    prfStreamInput.param_prfMode=LOAD;
    //Do not use the schedule for loading/Offloading
    prfStreamInput.param_scheduleROMsize=0;
    prfStreamInput.instream_aStream=a;
    prfStreamInput.instream_bStream=b;
    prfStreamInput.instream_cStream=c;
    prfStreamInput.outstream_aOutStream=aOut;
    prfStreamInput.outstream_bOutStream=bOut;
    prfStreamInput.outstream_cOutStream=cOut;
    prfStreamInput.inmem_PRFStreamKernel_ScheduleROM=scheduleROM;
    //Load bitstream on fpga
    max_file_t* StreamMaxFile =  PRFStream_init();
    max_engine_t* StreamDFE=max_load(StreamMaxFile,"*");

    PRFStream_run(StreamDFE,&prfStreamInput);

    prfStreamInput.param_prfMode=OFFLOAD;
    PRFStream_run(StreamDFE,&prfStreamInput);
    int error=0;
    for(int i = 0; i < STREAM_ARRAY_SIZE; ++i)
        if ( a[i] != aOut[i] || b[i] != bOut[i] || c[i] != cOut[i]){
                    error=1;
                    if(VERBOSE){
                        printf("Load/Offload error id : %d -> %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                    }
                }

    max_unload(StreamDFE);
    return error;
}

int test_STREAM_kernel(int kernel, int (*check)(int a, int b, int c) ){
    
    int sizeBytes = STREAM_ARRAY_SIZE * sizeof(double);
    double *a = malloc(sizeBytes);
    double *b = malloc(sizeBytes);
    double *c = malloc(sizeBytes);
    double *aOut = malloc(sizeBytes);
    double *bOut = malloc(sizeBytes);
    double *cOut = malloc(sizeBytes);

    int64_t * scheduleROM = malloc(512*512/8*sizeof(int64_t));

    // Generate input data
    for(int i = 0; i < STREAM_ARRAY_SIZE; ++i) {
            a[i] = i;
            b[i] = 1;
            c[i] = -1;
    }

    PRFStream_actions_t prfStreamInput;
    prfStreamInput.param_VEC_SIZE=STREAM_ARRAY_SIZE;
    prfStreamInput.param_copy_repeats= 1;
    prfStreamInput.param_prfMode=LOAD;
    //Do not use the schedule for loading/Offloading
    prfStreamInput.param_scheduleROMsize=0;
    prfStreamInput.instream_aStream=a;
    prfStreamInput.instream_bStream=b;
    prfStreamInput.instream_cStream=c;
    prfStreamInput.outstream_aOutStream=aOut;
    prfStreamInput.outstream_bOutStream=bOut;
    prfStreamInput.outstream_cOutStream=cOut;
    prfStreamInput.inmem_PRFStreamKernel_ScheduleROM=scheduleROM;
    //Load bitstream on fpga
    max_file_t* StreamMaxFile =  PRFStream_init();
    max_engine_t* StreamDFE=max_load(StreamMaxFile,"*");

    PRFStream_run(StreamDFE,&prfStreamInput);

    prfStreamInput.param_prfMode=kernel;
    PRFStream_run(StreamDFE,&prfStreamInput);

    prfStreamInput.param_prfMode=OFFLOAD;
    PRFStream_run(StreamDFE,&prfStreamInput);

    int error=0;
    for(int i = 0; i < STREAM_ARRAY_SIZE; ++i)
        if ( check(aOut[i],bOut[i],cOut[i])){
            error=1;
                    if(VERBOSE){
                        printf("kernel %d, error id : %d -> %f , %f , %f , %f , %f , %f \n",kernel,i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
                    }
        }
    max_unload(StreamDFE);
    return error;
}

int check_STREAM_copy(int a, int b, int c){
    if(c!=a){
        return 1;
    }
    return 0;
}

int test_STREAM_copy(){
    return test_STREAM_kernel(COPY, check_STREAM_copy);
}

int check_STREAM_scale(int a, int b, int c){
    if (b != 3*c){
        return 1;
    }
    return 0;
}

int test_STREAM_scale(){
    return test_STREAM_kernel(SCALE, check_STREAM_scale);
}

int check_STREAM_add(int a, int b, int c){
    if (c != a+b){
        return 1;
    }
    return 0;
}

int test_STREAM_add(){
    return test_STREAM_kernel(ADD, check_STREAM_add);
}

int check_STREAM_triad(int a, int b, int c){
    if (a != b+3*c){
        return 1;
    }
    return 0;
}

int test_STREAM_triad(){
    return test_STREAM_kernel(TRIAD, check_STREAM_triad);
}

int test_STREAM_sparse(){
    int sizeBytes = STREAM_ARRAY_SIZE * sizeof(double);
    double *a = malloc(sizeBytes);
    double *b = malloc(sizeBytes);
    double *c = malloc(sizeBytes);
    double *aOut = malloc(sizeBytes);
    double *bOut = malloc(sizeBytes);
    double *cOut = malloc(sizeBytes);

    int64_t * scheduleROM = malloc(512*512/8*sizeof(int64_t));
    for(int i=0;i<512*512/8;i++)
        scheduleROM[i]=0;
    // Generate input data
    for(int i = 0; i < STREAM_ARRAY_SIZE; ++i) {
            a[i] = i;
            b[i] = 1;
            c[i] = -1;
    }

    PRFStream_actions_t prfStreamInput;
    prfStreamInput.param_VEC_SIZE=STREAM_ARRAY_SIZE;
    prfStreamInput.param_copy_repeats= 1;
    prfStreamInput.param_prfMode=LOAD;
    //Do not use the schedule for loading/Offloading
    prfStreamInput.param_scheduleROMsize=0;
    prfStreamInput.instream_aStream=a;
    prfStreamInput.instream_bStream=b;
    prfStreamInput.instream_cStream=c;
    prfStreamInput.outstream_aOutStream=aOut;
    prfStreamInput.outstream_bOutStream=bOut;
    prfStreamInput.outstream_cOutStream=cOut;
    prfStreamInput.inmem_PRFStreamKernel_ScheduleROM=scheduleROM;
    //Load bitstream on fpga
    max_file_t* StreamMaxFile =  PRFStream_init();
    max_engine_t* StreamDFE=max_load(StreamMaxFile,"*");

    PRFStream_run(StreamDFE,&prfStreamInput);
    scheduleROM[0]=0x02000155;
    prfStreamInput.param_scheduleROMsize=1;
    prfStreamInput.param_copy_repeats= 1;
    prfStreamInput.param_prfMode=COPY;
    PRFStream_run(StreamDFE,&prfStreamInput);
    
    
    prfStreamInput.param_scheduleROMsize=0;
    prfStreamInput.param_prfMode=OFFLOAD;
    PRFStream_run(StreamDFE,&prfStreamInput);

    int error=0;
    for(int i = 0; i < STREAM_ARRAY_SIZE; ++i){
        if( i == 1 || i == 3 || i == 5 || i== 7){
            if(cOut[i]!=aOut[i])
                error=1;
        }else{
            if (cOut[i]!=-1)
                error=1;
        }
        if(VERBOSE){
                printf("COPY SPARSE DUMP id : %d -> %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
        }
    }
    max_unload(StreamDFE);
    return error;
}


int test_STREAM_sparse_with_schedule(){

    int sizeBytes = 87040 * sizeof(double);
    double *a = malloc(sizeBytes);
    double *b = malloc(sizeBytes);
    double *c = malloc(sizeBytes);
    double *aOut = malloc(sizeBytes);
    double *bOut = malloc(sizeBytes);
    double *cOut = malloc(sizeBytes);


    char *scheduleFile= "europar_polymem_30percent_solution_RoCo.schedule";
    Schedule *s = parseSchedule(scheduleFile);
        printf("here");
    int schedule_len = getFileLenght(scheduleFile);
        printf("here2");

    if (s==FILE_NOT_FOUND) {
        printf("FILE_NOT_FOUND");
    }
    int64_t * scheduleROM = compress_schedule_toROM(s,schedule_len,8,512*512/8); 
    // Generate input data
    for(int i = 0; i < 87040; ++i) {
            a[i] = i;
            b[i] = 1;
            c[i] = -1;
    }

    PRFStream_actions_t prfStreamInput;
    prfStreamInput.param_VEC_SIZE=STREAM_ARRAY_SIZE;
    prfStreamInput.param_copy_repeats= 1;
    prfStreamInput.param_prfMode=LOAD;
    //Do not use the schedule for loading/Offloading
    prfStreamInput.param_scheduleROMsize=0;
    prfStreamInput.instream_aStream=a;
    prfStreamInput.instream_bStream=b;
    prfStreamInput.instream_cStream=c;
    prfStreamInput.outstream_aOutStream=aOut;
    prfStreamInput.outstream_bOutStream=bOut;
    prfStreamInput.outstream_cOutStream=cOut;
    prfStreamInput.inmem_PRFStreamKernel_ScheduleROM=scheduleROM;
    //Load bitstream on fpga
    max_file_t* StreamMaxFile =  PRFStream_init();
    max_engine_t* StreamDFE=max_load(StreamMaxFile,"*");

    PRFStream_run(StreamDFE,&prfStreamInput);
    
    printf("START COPY SPARSE with schedule");
    prfStreamInput.param_scheduleROMsize=schedule_len;
    prfStreamInput.param_prfMode=COPY;
    PRFStream_run(StreamDFE,&prfStreamInput);

    printf("END COPY SPARSE with schedule");
    prfStreamInput.param_scheduleROMsize=0;
    prfStreamInput.param_prfMode=OFFLOAD;
    PRFStream_run(StreamDFE,&prfStreamInput);

    int error=0;
    for(int i = 0; i < 87040; ++i){
        if( i == 1 || i == 3 || i == 5 || i== 7){
            if(cOut[i]!=aOut[i])
                error=1;
        }else{
            if (cOut[i]!=-1)
                error=1;
        }
        if(VERBOSE){
                printf("COPY SPARSE with schedule DUMP id : %d -> %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
        }
    }
    max_unload(StreamDFE);
    return error;

}
#define TESTNB 7
int test_STREAM(){
    int error = 0;
    Testsuite stream_tests[TESTNB];

    stream_tests[0].function = test_load_offload;
    stream_tests[0].name = "load_offload";

    stream_tests[1].function = test_STREAM_copy;
    stream_tests[1].name = "STREAM copy";

    stream_tests[2].function = test_STREAM_scale;
    stream_tests[2].name = "STREAM scale";

    stream_tests[3].function = test_STREAM_add;
    stream_tests[3].name = "STREAM add";

    stream_tests[4].function = test_STREAM_triad;
    stream_tests[4].name = "STREAM triad";

    stream_tests[5].function = test_STREAM_sparse;
    stream_tests[5].name = "STREAM sparse";

    stream_tests[6].function = test_STREAM_sparse_with_schedule;
    stream_tests[6].name = "STREAM sparse with schedule";

    error = run_testsuite(stream_tests, TESTNB);
    return error;
}
