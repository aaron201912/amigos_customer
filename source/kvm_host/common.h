#ifndef __COMMON__
#define __COMMON__

#ifdef __cplusplus
extern "C" {
#endif

#define Key_DataSize		8
#define Mouse_DataSize		4
#define PacketSize(u32BufSize)		(u32BufSize)+sizeof(int)
//#define DEBUG

#define Key_Mask 			0x1000
#define Mouse_Mask 			0x2000
#define Clear_Mask 			0x0FFF

#define Key_Type 			6
#define Mouse_Type 			2

typedef struct ST_INTELL_TransferInfo_s{
   unsigned int u32BufSize ;
   char *pVirAddr;  
}ST_INTELL_TransferInfo_t;

#ifdef __cplusplus
}
#endif

#endif //COMMON