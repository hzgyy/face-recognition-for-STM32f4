#include "syn6288.h"
#include "./BSP/USART2/usart2.h"
#include "string.h"
#include "delay.h"


//Music:ѡ�񱳾����֡�0:�ޱ������֣�1~15��ѡ�񱳾�����
void SYN_FrameInfo(uint8_t Music, uint8_t *HZdata)
{
  /****************��Ҫ���͵��ı�**********************************/
  unsigned  char  Frame_Info[50];
  unsigned  char  HZ_Length;
  unsigned  char  ecc  = 0;  			//����У���ֽ�
  unsigned  int i = 0;
  HZ_Length = strlen((char*)HZdata); 			//��Ҫ�����ı��ĳ���

  /*****************֡�̶�������Ϣ**************************************/
  Frame_Info[0] = 0xFD ; 			//����֡ͷFD
  Frame_Info[1] = 0x00 ; 			//�������������ȵĸ��ֽ�
  Frame_Info[2] = HZ_Length + 3; 		//�������������ȵĵ��ֽ�
  Frame_Info[3] = 0x01 ; 			//���������֣��ϳɲ�������
  Frame_Info[4] = 0x01 | Music << 4 ; //����������������������趨

  /*******************У�������***************************************/
  for(i = 0; i < 5; i++)   				//���η��͹���õ�5��֡ͷ�ֽ�
  {
    ecc = ecc ^ (Frame_Info[i]);		//�Է��͵��ֽڽ������У��
  }

  for(i = 0; i < HZ_Length; i++)   		//���η��ʹ��ϳɵ��ı�����
  {
    ecc = ecc ^ (HZdata[i]); 				//�Է��͵��ֽڽ������У��
  }
  /*******************����֡��Ϣ***************************************/
  memcpy(&Frame_Info[5], HZdata, HZ_Length);
  Frame_Info[5 + HZ_Length] = ecc;
  USART2_SendString(Frame_Info, 5 + HZ_Length + 1);
}


/***********************************************************
* ��    �ƣ� YS_SYN_Set(u8 *Info_data)
* ��    �ܣ� ������	�������
* ��ڲ����� *Info_data:�̶���������Ϣ����
* ���ڲ�����
* ˵    �����������������ã�ֹͣ�ϳɡ���ͣ�ϳɵ����� ��Ĭ�ϲ�����9600bps��
* ���÷�����ͨ�������Ѿ�������������������á�
**********************************************************/
void YS_SYN_Set(uint8_t *Info_data)
{
  uint8_t Com_Len;
  Com_Len = strlen((char*)Info_data);
  USART2_SendString(Info_data, Com_Len);
}


void open_say(void)
{
	SYN_FrameInfo(2, "[v7][m1][t5] soundb ����԰��ӭ��");
}
void error_say(void)
{
	SYN_FrameInfo(2, "[v7][m1][t5] sounda ʶ��ʧ��");
}
void alarm_say(void)
{
	SYN_FrameInfo(2, "[v7][m1][t5] soundn β��ͨ�б���");
}