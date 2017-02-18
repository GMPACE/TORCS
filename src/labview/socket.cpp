/*
 * socket.cpp
 *
 *  Created on: 2017. 2. 18.
 *      Author: NaYeon Kim
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys\types.h>
#include <sys\socket.h>
#include <arpa\inet.h>
#include <unistd.h>
#include <sstream>


#define PORT_NUM 6341
#define SERVER_IP "192.168.0.8"

using namespace std;

template <typename T>
string ToString(T val)
{
	stringstream stream;
	stream << val;
	return stream.str();

}





int main(void)
{


 char ReceiveData[1024];
 int strLen;




 int s = socket(AF_INET,SOCK_STREAM,0);//socket 생성
 if (s == -1)
 {
  cout << "Socket 오류입니다" << endl;
  exit(1);
 }

 struct sockaddr_in addr;
 addr.sin_family = AF_INET;
 addr.sin_port = htons(PORT_NUM);
 addr.sin_addr.s_addr = inet_addr(SERVER_IP);

 if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1)
 {
  cout << "서버 연결 오류" << endl;
  exit(1);
 }

 cout << "서버 연결 성공" << endl;



 /*************** TCP Setting ****************/

 int car_autosteer = 0;
 int car_autogear = 0;
 int car_speed = 10;
 int car_rpm = 110;
 int car_gear = 0;
 int car_steer = 0;
 int car_odometer = 0;



 int count = 0;
 string str2 = "";

 string test[19] = {"Empty", "Accel","Brake", "Steer","gear","UpperRampStatic","UpperRamp","Ramp","RampAuto","WiperMove","WiperTrigger","WiperOff","WiperAuto","WiperLow","WiperHigh","RightTurn","LeftTurn","Bright","LampTrigger" };



 while (1)
 {

  string str= "dataautosteer" + ToString(car_autosteer) + +"autogear" + ToString(car_autogear)
   + "speed" + ToString(car_speed)
   + "rpm" + ToString(car_rpm)
   + "gear" + ToString(car_gear)
   + "steer" + ToString(car_steer)
   + "odometer" + ToString(car_odometer)
   + "eom";
  const char* cstr = str.c_str();




  send(s,cstr,60, 0);//메세지 보내기


  strLen = recv(s, ReceiveData, sizeof(ReceiveData) - 1, 0);//메세지 받기



  ReceiveData[strLen] = 0;




  char *ptr = strtok(ReceiveData, ".");//각각 값으로 쪼개기

  while (ptr != NULL)
  {
   str2 += test[count];
   str2 += " : ";
   str2 += ptr;
   str2 += " ";
   ptr = strtok(NULL, ".");
   count++;
  }


  cout << str2 << '\n';
  //printf("Msg : %s \n", ReceiveData);

  sleep(100);



 }



 close(s);

 return 0;
}




