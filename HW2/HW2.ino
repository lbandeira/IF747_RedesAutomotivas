#define SYNC_STATE
#define SEG1_STATE
#define SEG2_STATE
#define TIME_SEG1
#define TIME_SEG2

bool resync;
bool writing_point;
int TQ_count;
int phase_error;
int time_seg1;
int time_seg2;

int BLT_STATE;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  switch(BLT_STATE){
    case SYNC_STATE:
      writing_point = true;
      resync = true;
      TQ_count = 0;
      phase_error = 0;
      time_seg1 = TIME_SEG1;
      time_seg2 = TIME_SEG2;
      BLT_STATE = SEG1_STATE;
      break;
      
    case SEG1_STATE:
      writing_point = false;
      TQ_count++;
      
      if(TQ_count == TIME_SEG1){
        BLT_STATE = SEG2_STATE;
        phase_error = TIME_SEG2;
        TQ_error = 0;
      }
      else if (software_sync == true){ //software_sync
        if(phase_error <= SJW){
          TQ_count = TQ_count - phase_error;
          BLT_STATE = SEG1_STATE;
        }
        else{
          TQ_count = TQ_count - SJW; 
          BLT_STATE = SEG1_STATE; 
        }
      }
      else{
        BLT_STATE = SEG1_STATE;
      }

      phase_error++;
      break;
      
    case SEG2_STATE:
      writing_point = false;
      TQ_count++;
      
      if(TQ_count == TIME_SEG2){
        BLT_STATE = SYNC_STATE;
      }
      else if (software_sync == true){ //software_sync
        if(phase_error <= SJW){
          TQ_count = TQ_count + phase_error;
          BLT_STATE = SEG2_STATE;
        }
        else{
          TQ_count = TQ_count + SJW; 
          BLT_STATE = SEG2_STATE; 
        }
      }
      else{
        BLT_STATE = SEG2_STATE;
      }

      phase_error--;
      break;
    
  }





}
