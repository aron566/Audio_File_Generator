# Audio_File_Generator

通用PCM数据转wav文件生成器

![image-20210929014939246](README.image/image-20210929014939246.png)

## 协议模式下

![image-20211117211950376](README.Image/image-20211117211950376.png)

需使能数据校验

```
/*
           /-----------/-----------/-----------/-----------/-----------/
          /   帧头    / DATA_SIZE /    DATA   /   CRC16L  /   CRC16H  /
         /-----------/-----------/-----------/-----------/-----------/
        /   0x0566  /    >=2    /   ....    /   1Byte   /   1Byte   /
       /-----------/-----------/-----------/-----------/-----------/
      /   2Bytes  /   2Bytes  / 2<= nBytes/   1Byte   /   1Byte   /
     /-----------/-----------/-----------/-----------/-----------/
    /--------------校验区域--------------/
ps:小端模式（高地址高位数据，不包括帧头） 可以直接memcpy
*/
```

16K采样率的音频16bit位宽，传输为16000点/s == 32000Bytes/s，分成64Bytes每包发出，需要500包，每包加上协议字段6个字节，就是增加500*6=3000Bytes，传输1s时长的16K数据需要32000 + 3000 = 35000Bytes，以1.5Mbps可以：150000 / 35000 ~= 4通道数据

## 原始数据发送模块

[demo工程](https://github.com/aron566/Audio_File_Generator_MCU_Code)

依赖两个文件：`Audio_Debug.c`，`CircularQueue.c`，源码位于：MUC_Code目录下

## 使用方式

```c
static uint32_t SEND_DATA_FUNC_PORT(uint8_t *, uint32_t len)
{
  /*调用串口发送*/
}
static bool GET_IDEL_STATE_PORT(void)
{
  /*返回串口空闲状态*/
  return true;
}

/*定义发送区大小*/
static int16_t Send_Buf[128*4];/**< 4通道数据*/

int main(void)
{
  /*初始化硬件,时钟，串口，音频接收等*/
  ...
    
  /*初始化模块*/
  Audio_Debug_Init((uint16_t *)Send_Buf, SEND_DATA_FUNC_PORT, GET_IDEL_STATE_PORT);
  
  for(;;)
  {
    /*接收到音频数据*/
    if(Audio_Data_Is_Received == true)
    {
      /*调用算法处理音频*/
      Result_Audio_Data_Ptr = Algorithm_Func(Source_Audio_Data);
      
      /*数据给调试模块输出*/
      Audio_Debug_Put_Data(Source_Audio_Data, Result_Audio_Data_Ptr, 0);
    }
    /*依据音频周期，调用模块*/
    if(Can_Send_Flag == true)
    {
    	Audio_Debug_Start();
    }
  }
}
```

## 数据生成工具使用

- 连接串口
- 修改待保存音频文件名
- 设置传输的通道数，即原始数据的格式
- 设置文件大小，默认为0，则限制为65535大小，鼠标悬浮获得提示
- 启动录制即可

