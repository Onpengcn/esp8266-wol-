#include <WiFiUdp.h>
//=====================Wol=================
char Wol_buf[102];//前6为ff,后面都为MAC地址 16*{0xAA,0xAA,0xAA,0xAA,0xAA,0xAA}
char Pc_mac[6] = {0x40,0x16,0x7e,0x26,0x77,0x30};
const char * Boardcast_addr = "192.168.1.255";
unsigned int Remote_port = 9;
unsigned int Local_port = 9;
WiFiUDP Udp;

void Wol_Init(char Wol_buf[102],char Pc_mac[6],unsigned int Local_port){
  int i,j;
  Udp.begin(Local_port);
  for(i=0;i<6;i++){
    Wol_buf[i]=0xff;
  }
  for(i=1;i<17;i++){
    for(j=0;j<6;j++){
      Wol_buf[i*6+j] = Pc_mac[j];
    }
  }
  #ifdef wol_debug
  Serial.println("Wol package is:") 
  for(i=0;i<102;i++){
    Serial.print("%x ",Wol_buf[i]);
    if(i%6 == 0)
      Serial.print("\n");
  }
  #endif
}

void Wol_Send_packet(char Wol_buf[102],const char * Boardcast_addr,unsigned int Remote_port){
  Udp.beginPacket(Boardcast_addr,Remote_port);
  Udp.write(Wol_buf);
  Udp.endPacket();
}
