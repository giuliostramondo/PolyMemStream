#include "tests.h"

#define VERBOSE 1

#define STREAM_ARRAY_SIZE	32

#define    LOAD 0
#define    OFFLOAD 1
#define    COPY 2
#define    SCALE 3
#define    ADD 4
#define    TRIAD 5

int M=512;
int N=512;

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


int test_read_write_DRAM(){
    int STREAM_SIZE = 768;//Must be a multiple of 48 elements ( stream requirement from DRAM ) 
    int sizeBytes = STREAM_SIZE * sizeof(double);
    double *input = malloc(sizeBytes);
    double *output = malloc(sizeBytes);
    for(int i=0;i<STREAM_SIZE;i++){
        input[i]= i;
        output[i]=0;
    }

    //Load bitstream on fpga
    max_file_t* StreamMaxFile =  PRFStream_init();
    max_engine_t* StreamDFE=max_load(StreamMaxFile,"*");

    //Configuring write action
    PRFStream_writeToLMEM_actions_t write_DRAM_action;
    write_DRAM_action.param_size=STREAM_SIZE;
    write_DRAM_action.param_start_address=0;
    write_DRAM_action.instream_fromcpu_toLMEM =input;
    
    //Write to LMEM 
    PRFStream_writeToLMEM_run(StreamDFE,&write_DRAM_action);

    //Configuring read action
    PRFStream_readFromLMEM_actions_t read_DRAM_action;
    read_DRAM_action.param_size=STREAM_SIZE;
    read_DRAM_action.param_start_address=0;
    read_DRAM_action.outstream_tocpu_fromLMEM=output;
    PRFStream_readFromLMEM_run(StreamDFE,&read_DRAM_action);
    int error=0;
    //test if output is equal to input
    for(int i=0;i<STREAM_SIZE;i++){
        if(input[i] != output[i]){
           error=1; 
        }  
    }
    
    max_unload(StreamDFE);
    return error;
}

//Helper function for test_load_from_DRAM_to_PolyMem
double *generateCPUtoDRAMStream(int *stream_size,int vec_size,double * inA, double *inB, double* inC){
    int padding = (((vec_size*8/384)+1)*384)/8- vec_size;
    int size = (vec_size+padding)*3;
    *(stream_size)=size;
    double* stream = malloc(sizeof(double) * size);
    for(int i =0;i<size;i++){
        if(i%(vec_size + padding ) < vec_size){
            if(i/(vec_size+padding) == 0)
                stream[i]=inA[i];
            else if(i/(vec_size+padding) == 1)
                stream[i] = inB[i];
            else if (i/(vec_size+padding) == 2)
                stream[i] = inC[i];

        }
        else 
            stream[i]=0; 
        printf("STREAM GENERATOR index: %d value :%f\n",stream[i]);
    }
    return stream;
}
//Host load data to DRAM
//PolyMem load data from DRAM
//Run Copy Benchmark 
//Offload from PolyMem to Host 
int test_load_from_DRAM_to_PolyMem(){
    //Stream to DRAM Must be a multiple of 48 elements ( stream requirement from DRAM )
    int vec_size = 512*170;
    int sizeBytes = vec_size * sizeof(double);
    double *inA = malloc(sizeBytes);
    double *inB = malloc(sizeBytes);
    double *inC = malloc(sizeBytes);
    double *aOut = malloc(sizeBytes);
    double *bOut = malloc(sizeBytes);
    double *cOut = malloc(sizeBytes);
    //initialize A B C input vectors
    for(int i=0;i<vec_size;i++){
        inA[i]= i;
        inB[i]= 1;
        inC[i]=-1;
    }

    int stream_size;
    double* input = generateCPUtoDRAMStream(&stream_size,vec_size,inA,inB,inC);

    //Load bitstream on fpga
    max_file_t* StreamMaxFile =  PRFStream_init();
    max_engine_t* StreamDFE=max_load(StreamMaxFile,"*");

    //Configuring write action
    PRFStream_writeToLMEM_actions_t write_DRAM_action;
    write_DRAM_action.param_size=stream_size;
    write_DRAM_action.param_start_address=0;
    write_DRAM_action.instream_fromcpu_toLMEM =input;
    
    //Write to LMEM 
    PRFStream_writeToLMEM_run(StreamDFE,&write_DRAM_action);

    double *dummyOutputA= malloc(sizeof(double)*2);
    double *dummyOutputB= malloc(sizeof(double)*2);
    double *dummyOutputC= malloc(sizeof(double)*2);

    PRFStream_loadFromDRAM_actions_t load_DRAM_action;
    load_DRAM_action.outstream_tocpu_outA=dummyOutputA;
    load_DRAM_action.outstream_tocpu_outB=dummyOutputB;
    load_DRAM_action.outstream_tocpu_outC=dummyOutputC;
    load_DRAM_action.param_VEC_SIZE=512*170;

    //Load from DRAM to PolyMem
    PRFStream_loadFromDRAM_run(StreamDFE,&load_DRAM_action);

    //Offload from PolyMem to Host
    int64_t * scheduleROM = malloc((512*512/8)*sizeof(int64_t));
    PRFStream_actions_t prfStreamInput;
    prfStreamInput.param_VEC_SIZE=512*170;
    prfStreamInput.param_copy_repeats= 1;
    prfStreamInput.param_prfMode=OFFLOAD;
    //Do not use the schedule for loading/Offloading
    prfStreamInput.param_scheduleROMsize=0;
    prfStreamInput.instream_fromcpu_inA=dummyOutputA;
    prfStreamInput.instream_fromcpu_inB=dummyOutputB;
    prfStreamInput.instream_fromcpu_inC=dummyOutputC;
    prfStreamInput.outstream_tocpu_outA=aOut;
    prfStreamInput.outstream_tocpu_outB=bOut;
    prfStreamInput.outstream_tocpu_outC=cOut;
    prfStreamInput.inmem_PRFStreamKernel_ScheduleROM=scheduleROM;

    PRFStream_run(StreamDFE,&prfStreamInput);
    printf("Done Reading PolyMem"); 
    for (int i =0;i<512*170;i++){
        printf("LOAD FROM DRAM index : %d a: %f b: %f c:%f\n",i,aOut[i],bOut[i],cOut[i]);
    }
    max_unload(StreamDFE);
    return 0;
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

int test_loadEngineInterface_offload(){
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

    PRFStream_loadFromHost_actions_t load_action_inputs;
    load_action_inputs.instream_fromcpu_inA=a;
    load_action_inputs.instream_fromcpu_inB=b;
    load_action_inputs.instream_fromcpu_inC=c;
    load_action_inputs.param_VEC_SIZE=STREAM_ARRAY_SIZE;
    load_action_inputs.outstream_tocpu_outA=aOut;
    load_action_inputs.outstream_tocpu_outB=bOut;
    load_action_inputs.outstream_tocpu_outC=cOut;

    PRFStream_actions_t prfStreamInput;
    prfStreamInput.param_VEC_SIZE=STREAM_ARRAY_SIZE;
    prfStreamInput.param_copy_repeats= 1;
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

    //PRFStream_run(StreamDFE,&prfStreamInput);
    printf("Calling loadFromHost_run\n");
    PRFStream_loadFromHost_run(StreamDFE,&load_action_inputs);
    printf("Returned from  loadFromHost_run\n");

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
    int vec_size = 87040;
    int sizeBytes = vec_size * sizeof(double);
    double *a = malloc(sizeBytes);
    double *b = malloc(sizeBytes);
    double *c = malloc(sizeBytes);
    double *aOut = malloc(sizeBytes);
    double *bOut = malloc(sizeBytes);
    double *cOut = malloc(sizeBytes);
    double *cExpectedOut = malloc(sizeBytes);



    //char *scheduleFile= "europar_polymem_30percent_solution_RoCo.schedule_v2";
    char *scheduleFile= "./pattern_utils/schedules/offset_2/pattern_20percent_M_512_N_170_offset_2_RoCo.schedule";
    Schedule *s = parseSchedule(scheduleFile);

    int schedule_len = getFileLenght(scheduleFile);

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

    //Compute expected C output
     // initialize with -1  
    for(int i=0;i< vec_size; i++)
        cExpectedOut[i]=-1;
    // Only copy from A elements accessed by the schedule
    printf("START DEBUG sparse with schedule expected output\n");
    for(int i=0;i< schedule_len; i++)
        {
          int *mask = s[i].mask; 
           Address2d *access=AGU( s[i].i,s[i].j,2,4,s[i].shape);
           printf("schedule %d, i:%d, j:%d shape:%d  \n",i,s[i].i,s[i].j,s[i].shape);
           printf("mask: ");
           for(int j=0;j<8;j++)
               printf("%d ",mask[j]);
           printf("\n");
               printf("index to update : ");
            for( int j =0;j<2*4;j++){
                if(mask[j]==1){

                    int index = access[j].i*M+access[j].j;
                    printf("(id: %d value of a: %d) ",index,a[index]);
                    cExpectedOut[index]=a[index];
                    printf("Expected C : %f\n", cExpectedOut[index]);
                }
            }
                   printf("\n");
        }

    printf("END DEBUG sparse with schedule expected output\n");
    PRFStream_actions_t prfStreamInput;
    prfStreamInput.param_VEC_SIZE=vec_size;
    prfStreamInput.param_copy_repeats= 1;
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

    PRFStream_run(StreamDFE,&prfStreamInput);

    //prfStreamInput.param_scheduleROMsize=0;
    //prfStreamInput.param_prfMode=OFFLOAD;
    //PRFStream_run(StreamDFE,&prfStreamInput);
    //prfStreamInput.param_scheduleROMsize=0;
    //prfStreamInput.param_prfMode=OFFLOAD;
    //PRFStream_run(StreamDFE,&prfStreamInput);  

    prfStreamInput.param_copy_repeats= 1;
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
        if ( cExpectedOut[i] != cOut[i]){
            error=1;
            printf("COPY SPARSE with schedule ERROR DUMP id : %d -> %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
            printf("Expected C : %f\n", cExpectedOut[i]);
        }
        if(VERBOSE){
                printf("COPY SPARSE with schedule DUMP id : %d -> %f , %f , %f , %f , %f , %f \n",i,a[i] , aOut[i] , b[i] , bOut[i] , c[i] , cOut[i]);
            printf("Expected C : %f\n", cExpectedOut[i]);
        }
    }
    max_unload(StreamDFE);
    return error;

}
#define TESTNB 8
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

    stream_tests[6].function = test_loadEngineInterface_offload;
    stream_tests[6].name = "STREAM load Engine Interface";


    stream_tests[7].function = test_read_write_DRAM;
    stream_tests[7].name = "load offload from DRAM";
    
    stream_tests[7].function = test_load_from_DRAM_to_PolyMem;
    stream_tests[7].name = "Load from DRAM to PolyMem";
    //stream_tests[6].function = test_STREAM_sparse_with_schedule;
    //stream_tests[6].name = "STREAM sparse with schedule";

    
    error = run_testsuite(stream_tests, TESTNB);
    return error;
}
