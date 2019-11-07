// this example will play a track and then 
// every five seconds play another track
//
// it expects the sd card to contain these three mp3 files
// but doesn't care whats in them
//
// sd:/mp3/0001.mp3
// sd:/mp3/0002.mp3
// sd:/mp3/0003.mp3

#include <SoftwareSerial.h>
#include <DFMiniMp3.h>

#define NUM_BUTTONS 2
// implement a notification class,
// its member methods will get called 
//
class Mp3Notify
{
public:
  static void OnError(uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }

  static void OnPlayFinished(uint16_t globalTrack)
  {
    Serial.println();
    Serial.print("Play finished for #");
    Serial.println(globalTrack);   
  }

  static void OnCardOnline(uint16_t code)
  {
    Serial.println();
    Serial.print("Card online ");
    Serial.println(code);     
  }

  static void OnUsbOnline(uint16_t code)
  {
    Serial.println();
    Serial.print("USB Disk online ");
    Serial.println(code);     
  }

  static void OnCardInserted(uint16_t code)
  {
    Serial.println();
    Serial.print("Card inserted ");
    Serial.println(code); 
  }

  static void OnUsbInserted(uint16_t code)
  {
    Serial.println();
    Serial.print("USB Disk inserted ");
    Serial.println(code); 
  }

  static void OnCardRemoved(uint16_t code)
  {
    Serial.println();
    Serial.print("Card removed ");
    Serial.println(code);  
  }

  static void OnUsbRemoved(uint16_t code)
  {
    Serial.println();
    Serial.print("USB Disk removed ");
    Serial.println(code);  
  }
};

// instance a DFMiniMp3 object, 
// defined with the above notification class and the hardware serial class
//
//DFMiniMp3<HardwareSerial, Mp3Notify> mp3(Serial1);

// Some arduino boards only have one hardware serial port, so a software serial port is needed instead.
// comment out the above definition and uncomment these lines

SoftwareSerial secondarySerial(10, 11); // RX, TX
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);

int pc_switch = 13;
int current_mode = 0; //0 - Parent 1 - Child
int buttons[9] = {2,3,4,5,6,7,8,9,12};
int sounds[10] = {1,2,3,4,5,6,7,8,9,10};
int parent_buffer[100];
int parent_key_counter = 0;
int child_key_counter = 0;
/*
*1 Dog Bark 0001
*2 Cat Meow 0002
*3 Crow Caw 0003
*4 Cow Moo 0004
*5 Car Horn 0005
*6 Chicken Clucking 0006
*7 Bell Ringing 0007
*8 Fart Fart-Sound 0008 
*9 Emergency Siren 0009
*10 Try Again
*/
void setup() 
{

  for(int i=0;i<NUM_BUTTONS;i++)
  {
    pinMode(buttons[i],INPUT);
    digitalWrite(buttons[i],HIGH);  
  }

  Serial.begin(115200);
  Serial.println("initializing...");
  
  mp3.begin();

  uint16_t volume = mp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  mp3.setVolume(20);
  
  uint16_t count = mp3.getTotalTrackCount();
  Serial.print("files ");
  Serial.println(count);
  
  Serial.println("starting...");
}

void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    // calling mp3.loop() periodically allows for notifications 
    // to be handled without interrupts
    mp3.loop(); 
    delay(1);
  }
}

void loop() 
{
  if(digitalRead(pc_switch) == LOW)
  {
    while(digitalRead(pc_switch) == LOW);
    if (current_mode ==0)
      current_mode =1;
    else
      current_mode =0;   
    Serial.print("Current Mode: ");Serial.println(current_mode);
  }

  for(int i=0;i<NUM_BUTTONS;i++)
  {
    if(digitalRead(buttons[i]) == LOW)  //Active low button
    { 
      Serial.print("current i ");Serial.println(i);
      Serial.print("track: ");Serial.println(sounds[i]); 
      mp3.playMp3FolderTrack(sounds[i]);
      
      if(current_mode == 0) //Parent Mode 
      {
        parent_buffer[parent_key_counter] = buttons[i];  
        parent_key_counter+=1;  
        Serial.print("Parent key registered, counter = ");Serial.println(parent_key_counter);
      }
      
      else  //Child Mode
      {
        if(buttons[i] == parent_buffer[child_key_counter])
        {
          Serial.print("Correct, child key counter ");Serial.println(child_key_counter);
          child_key_counter+=1;  
        }
        else
        {
          Serial.println("ERROR");
//          mp3.playMp3FolderTrack(sounds[9]);
          child_key_counter = 0;  
        }

        
        if(child_key_counter==parent_key_counter)
        {
          child_key_counter = 0;
          parent_key_counter = 0;
          Serial.println("Successful");
          current_mode = 0;
        }     
      }
      while(digitalRead(buttons[i]) == LOW);
      delay(500);
    }
  }
}
