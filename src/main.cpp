#include <Arduino.h>
#include <SPI.h>
// DID YOU UPLOAD THE FILESYSTEM IMAGE YET?
#include <SPIFFS.h>
#include <stream.hpp>
#include <zip.hpp>
using namespace io; // for streams
using namespace zip; // for zips

// if we try to declare all this
// on the stack we run into problems
// apparently the File class is pretty
// heavy:
char path[1024];
File f;
File f2;
archive arch;
archive_entry entry;
void setup()
{
  Serial.begin(115200);
  Serial.println();

  SPIFFS.begin(false);
  f=SPIFFS.open("/frankenstein.epub","rb");
  if(!f) {
    Serial.println("File not found");
    while(true);
  }
  file_stream fs(f);
  if(zip_result::success!=archive::open(&fs,&arch)) {
    Serial.println("Zip load failed.");
    while(true);
  }
  Serial.print("Number of files ");
  Serial.println(arch.entries_size());
  
  arch.entry(11,&entry);
  
  if(SPIFFS.exists("/tmp.htm")) {
    SPIFFS.remove("/tmp.htm");
  }

  f2 = SPIFFS.open("/tmp.htm","wb");
  
  file_stream fs2(f2);
  Serial.print("extracting ");
  entry.copy_path(path,1024);
  Serial.print(path);
  Serial.println("...");
  zip_result rr=entry.extract(&fs2);
  if(zip_result::success!=rr) {
    Serial.print("extraction failed ");
    Serial.println((int)rr);
    while(true);
  }
  Serial.println("extraction complete");
  f.close();
  f2.close();
  f=SPIFFS.open("/tmp.htm","rb");
  if(!f) {
    Serial.println("Temp file not found");
    while(true);
  }
  while(true) {
    int i = f.read();
    if(0>i) break;
    Serial.write(i);
  }
  f.close();
  
}

void loop()
{

}