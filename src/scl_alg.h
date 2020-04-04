
#ifndef SCL_ALG_H_
#define SCL_ALG_H_
/*测试改写这里*/
#define L 2
#define N 1024
#define M 10
#define CRCBYTE 3
#define N_gauss N //需要产生的高斯白噪声序列的点的个数


//#define DXRE 4//优化第一次LLR
/*测试改写这里*/

#ifndef SCL_ALG_C_
extern const uint32_t CRC24_Gx;
extern const int16_t difNodTab[10];//同列对应的fg间距1,2,4,8,16,32,64,128
extern const int16_t sameNodTab[10];//同列相邻同类节点地址间距2,4,8,16,32,64,128,256
extern const int16_t deCoCode[N];//记录译码结果
extern const int16_t frz[N];  
#endif

#endif // !SCL_ALG_H_