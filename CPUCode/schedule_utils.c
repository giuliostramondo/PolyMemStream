#include "schedule_utils.h"



int64_t *compress_schedule_toROM(Schedule *s, int schedule_len,int lanes,
                                                    int schedule_rom_size){
    int64_t *rom_data = (int64_t*) malloc(sizeof(int64_t)*schedule_rom_size);
    for(int i=0;i<schedule_rom_size;i++)
        rom_data[i]=0;
    for(int i =0;i<schedule_len;i++){
        int current_entry=0;
        int mask_tmp =0;
        int *curr_mask = s[i].mask;
        for(int mask_bit =0;mask_bit < lanes ; mask_bit ++ ){
            mask_tmp |= (unsigned int)(curr_mask[mask_bit])<<mask_bit;
        }
        current_entry = mask_tmp;
        current_entry |= s[i].j<<8;
        current_entry |= s[i].i<<17;
        current_entry |= s[i].shape <<25;
        rom_data[i]=current_entry; 

        printf("acc_Type %d, i %d, j %d, mask %d\n",s[i].shape,s[i].i,s[i].j,mask_tmp);
        printf("compressed : %d\n",current_entry);
    } 
    return rom_data;
}

int getFileLenght(char *scheduleFile){

    if( access( scheduleFile, F_OK ) == -1 ) {
        return -1; 
     }
    FILE *fp=fopen(scheduleFile,"r");
    //Count line of FILE
    int lines=0;
    char ch;
        printf("%x",fp);
    fflush(stdout);
    while(!feof(fp))
    {
      ch = fgetc(fp);
        printf("%c",ch);
    fflush(stdout);
      if(ch == '\n')
      {
        lines++;
      }
    }
    fclose(fp);
    return lines;
}

int check_digit(char d){
    if ( d=='0'|| d=='1'|| d=='2'||
         d=='3'|| d=='4'|| d=='5'||
         d=='6'|| d=='7'|| d=='8'|| d=='9')
        return 1;
    else 
        return 0;
}

Schedule *parseSchedule(char *scheduleFile){
    int lines = getFileLenght(scheduleFile);
    if(lines == -1 ){
        return FILE_NOT_FOUND;
    }
    FILE *fp=fopen(scheduleFile,"r");
    int lane_count=0;
    int parenthesis_open=0;
    char ch = fgetc(fp);
    while(ch!=']')
    {   
      if(lane_count==0){
        if(ch == '[')
            parenthesis_open=1;
      }
      if(parenthesis_open==1){
       if(ch ==',')
          lane_count++;
       if(ch==']'){
            lane_count++;
            parenthesis_open=0;
       } 
      }
      
      ch = fgetc(fp);
    }
    lane_count++;
    printf("lines : %d\n",lines);
    printf("lanes : %d\n",lane_count);
    fclose(fp); 
    fp=fopen(scheduleFile,"r");
    Schedule *s=(Schedule*)malloc(sizeof(Schedule)*lines);
    char buffer[10];
    int buffer_len=0;
    int state =0;
    for(int i=0; i<lines;i++){
        //printf("line %d, of %d\n",i,lines);
        ch = fgetc(fp);
          while(ch != '\n'){

          if(state==0&&ch=='(')
              state=1;
          else if(state==1&&ch=='(')
              state=2;
          else if(state==2){
                while(ch!=','){
                    if(check_digit(ch)){
                        buffer[buffer_len++]=ch;
                        ch = fgetc(fp);
                    }else{
                        printf("Error at line %d, expected digit got %c (state %d)\n",i,ch,state);
                        return NULL;
                    }
                }
                buffer[buffer_len++]='\0';
                s[i].i=atoi(buffer);
                buffer_len=0;
                ch = fgetc(fp);//consume space
                if(ch == ' '){
                    state=3;
                }else{
                        printf("Error at line %d, expected space got %c (state %d)\n",i,ch,state);
                    return NULL;
                }

          }
          else if(state==3){
               while(ch!=','){
                    if(check_digit(ch)){
                        buffer[buffer_len++]=ch;
                        ch = fgetc(fp);
                    }else{
                        printf("Error at line %d, expected digit got %c (state %d)\n",i,ch,state);
                        return NULL;
                    }
                }
                buffer[buffer_len++]='\0';
                s[i].j=atoi(buffer);
                buffer_len=0;
                ch = fgetc(fp);//consume space
                if(ch == ' '){
                    state=4;
                }else{
                        printf("Error at line %d, expected space got %c (state %d)\n",i,ch,state);
                    return NULL;
                }   
          }else if(state==4){
                while(ch!=':'){
                    ch = fgetc(fp);//consume chars
                    if(feof(fp)||ch=='\n'){
                        printf("Error at line %d, expected : but never found it (state %d)\n",i,state);
                        return NULL;
                    }
                }
                ch = fgetc(fp);//consume space
                if(ch == ' '){
                    state=5;
                }else{
                        printf("Error at line %d, expected space got %c (state %d)\n",i,ch,state);
                    return NULL;
                }   
          }else if(state==5){
                while(ch!='>'){
                    if(check_digit(ch)){
                        buffer[buffer_len++]=ch;
                        ch = fgetc(fp);
                    }else{
                        printf("Error at line %d, expected digit got %c (state %d)\n",i,ch,state);
                        return NULL;
                    }
                }
                buffer[buffer_len++]='\0';
                s[i].shape=atoi(buffer);
                buffer_len=0;
                while(ch!='['){
                    ch = fgetc(fp);//consume chars
                    if(feof(fp)||ch=='\n'){
                        printf("Error at line %d, expected [ but never found it (state %d)\n",i,state);
                        return NULL;
                    }
                } 
                state=6;
          }else if(state==6){
              int *mask_=(int*)malloc(sizeof(int)*lane_count);
              int current_lane=0;
             while(ch!=']'){
                     if(check_digit(ch)){
                        buffer[buffer_len++]=ch;
                        ch = fgetc(fp);
                    }else{
                        printf("Error at line %d, expected digit got %c (state %d)\n",i,ch,state);
                        return NULL;
                    }
                    buffer[buffer_len++]='\0';
                    //printf("%s\n",buffer);
                    mask_[current_lane++]=atoi(buffer);
                    buffer_len=0;    
                    if(ch==','){
                        ch = fgetc(fp);//consume space
                        if(ch == ' '){
                            state=6;
                            ch = fgetc(fp);//next char
                        }else{
                                printf("Error at line %d, expected space got %c (state %d)\n",i,ch,state);
                            return NULL;
                        }   
                    }
                   // ch = fgetc(fp);//next char
             } 
                    s[i].mask=mask_; 
                   ch = fgetc(fp);//next char
                    if(ch == ')'){
                        state=6;
                        //ch = fgetc(fp);//next char
                    }else{
                            printf("Error at line %d, expected ) got %c (state %d)\n",i,ch,state);
                        return NULL;
                    }  
          }

          ch = fgetc(fp);
        }
        state=0;
    }
    fclose(fp);
    return s;
}
