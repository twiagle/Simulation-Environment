/*
用到的数据类型：
除了scl_deco.c中记录L，2L的计数变量为uint_8，其余正整数,以及bpsk用int16_t,后来写的用uint16_t
llr用float
*/
#define SCL_ALG_C_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "scl_alg.h"
#include "scl_deco.h"

const int16_t difNodTab[10] = { 1,2,4,8,16,32,64,128,256,512 };//同列对应的fg间距,每列一个
const int16_t sameNodTab[10] = { 2,4,8,16,32,64,128 ,256,512,1024 };//同列相邻同类节点地址间距，除了最后一列
const uint32_t CRC24_Gx = 0x1864CFB;
//const int16_t frz[N] = { 0,0,0,1,0,1,1,1};
//const int16_t frz[N] = { 0 ,0, 0 ,0 ,0 ,0 ,0 ,0, 0, 0 ,0 ,1, 0, 1, 1 ,1 ,0 ,0 ,0, 1 ,0 ,1 ,1 ,1 ,1, 1 ,1 ,1 ,1 ,1, 1 ,1 };
const int16_t frz[N] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,0,0,0,1,0,1,1,1,0,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,0,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,1,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};//信息位，冰冻比特位
uint8_t unCoCode[N / 2];//信息比特
int8_t mixCode[N];//混合固定信息、编码、bpsk公用，不需要存在整个程序，加完噪声可以去掉,若被调函数自己产生的数组需要传出，只能全局或malloc，要是主调函数产生就传给被调一个指针即可 
//int16_t deCoCode[N];//记录译码结果

//创建文本文件
//FILE *creatTextFile(uint8_t *fileName) {
//	FILE *fname;
//
//	if ((fname = fopen(fileName, "a+")) == NULL) {
//		printf("\nFail to creat file");
//		exit(1);/////////////////////////////////////////////十分不合适
//	}
//	return fname;
//}
//信噪比转换方差
double enToDx(double ebnoi) {
	return 1.0492 / (pow(10, ebnoi / 10.0));
}
//编码加CRC
uint32_t Gen_CRC_24(unsigned char * Data_in, uint32_t lenth)  {//此CRC只适合一个字节表示一个比特的polar码,lenth为随机数长度
	uint32_t R = 0;

	for (uint32_t i = 0; i < lenth + 24; i++){
		if (i<lenth)
			R = (R << 1) + Data_in[i];
		else
			R = (R << 1) + 0;
		if (R >= 0x01000000)
			R = (R ^ CRC24_Gx);
	}
	return R;//25--32 = 0
}
/*生成随机数填充信息位*/
void randMesBits() {
	for (uint32_t i = 0; i < N / 2 - CRCBYTE*8; i++){//前 N / 2 - 24 是随机数，后24是CRC
		unCoCode[i] = rand() % 2;//0-1
	}
}
void encoAddCRC() {
	uint32_t tmp;
	uint32_t i;

	tmp = Gen_CRC_24(unCoCode, N / 2 - CRCBYTE * 8);
	for (i = 0; i < CRCBYTE * 8; i++) {
		if (tmp & 0x800000) {
			unCoCode[N / 2 - CRCBYTE * 8 + i] = 1;
		}
		else
			unCoCode[N / 2 - CRCBYTE * 8 + i] = 0;

		tmp <<= 1;
	}
}
/*混合固定信息比特*/
void mixMesFrz() {
	for (int16_t i = 0,j = 0; i < N; i++) {
		if (*(frz + i)) {
			mixCode[i] = (int8_t)unCoCode[j++];//信息位,最高位0，强制转换还是0
		}
		else
			mixCode[i] = 0;//固定位给0,在多次编码时必须把全局变量的默认0赋值
	}
}
void encoding(int8_t *p){
	int16_t i, j, k;

	for (i = 0; i < M; i++)//M列数，每列是一个二重循环
	{
		for (j = 0; j < N; j += *(sameNodTab + i))//外循环变量代表  每列最小结构（一些连续异或）   首地址  每次  每列分别  加0,2,4,8,  首地址为行号终止
		{
			for (k = 0; k < *(difNodTab + i); k++)//内循环代表   最小结构中'连续异或'   次数1,2,4,8
			{
				*(p + j + k) = (*(p + j + k) + *(p + j + k + *(difNodTab + i))) % 2;//相加取余数即01异或
			}
		}
	}
}
void bpsk(int8_t *p){
	for (int16_t i = 0; i < N; i++)
	{
		if (*(p + i) == 1)
			*(p + i) = -1;
		else if (*(p + i) == 0)
			(*(p + i))++;
	}
}
//part of wgn
double gaussrand()
{
	static double V1, V2, S;
	static int phase = 0;
	double X;

	if (phase == 0) {
		do {
			double U1 = (double)rand() / RAND_MAX;
			double U2 = (double)rand() / RAND_MAX;

			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
		} while (S >= 1 || S == 0);

		X = V1 * sqrt(-2 * log(S) / S);
	}
	else
		X = V2 * sqrt(-2 * log(S) / S);

	phase = 1 - phase;

	return X;
}

//wng
float *gauss(double Sigma, int n_point) {//dx:方差；n_point:点数
	int i;
	float *mem1;
	mem1 = malloc(n_point * sizeof(float));

	for (i = 0; i < n_point; i++){
		mem1[i] = (float)(gaussrand() * Sigma);
	}
	for (i = 0; i < n_point; i++){
		*(mem1 + i) += *(mixCode + i);//加入高斯噪声
	}
	return(mem1);
}

/*//without高斯噪声test，只是变成浮点数，不能定义数组返回指针，函数调用结束释放数组
float *awgn(int16_t *mixCode, int n_point){
	int16_t i;

	float *awgnCode;
	awgnCode = malloc(n_point * sizeof(float));
	for (i = 0; i < N; i++)
	{
		*(awgnCode + i) = *(mixCode + i) + 0.0f;//f代表这是一个浮点数，免于警告float
	}
	return awgnCode;
}
*/

int main(){
	double ebnoi;//信噪比
	uint32_t forTest = 1000;//测试次数
	uint32_t forTestPackup = forTest;

	uint32_t errorTime = 0;//bler
	uint16_t errorThis = 0;//ber
	double bler;
	FILE *outFile;

	uint8_t fileName[128];
	int16_t  set;
	
	srand((unsigned int)time(NULL));

//	sprintf(fileName, "L=%d_N=%d.txt", L, N);//文件名
//	outFile = creatTextFile(fileName);//建立文本文件

	for (set = 1, ebnoi = 1.0; ebnoi <= 2.6; set++, ebnoi += 0.1) {
//		printf("set %d %s loading...    %%", set, fileName);
		forTest = forTestPackup;
		errorTime = 0;
		unsigned __int64 timeSumup = 0;
		uint32_t avrgTime;
		double dx = enToDx(ebnoi);//每个文件方差不同
		double Sigma = sqrt(dx);
		while (forTest--) {
			randMesBits();
			encoAddCRC();
			mixMesFrz();	/*混合固定信息比特*/
			encoding(mixCode);//编码
			bpsk(mixCode);//mixCode和bpsk共用一个整数组
			float *gauss_out = gauss(Sigma, N_gauss);//生成高斯噪声,需要与mixCode相加	
			//gauss_out = awgn(mixCode, N_gauss);//测试算法正确性不加噪声，只变浮点数
		
			uint8_t *finalCode = decode(gauss_out, dx);//添加高斯噪声并输入解码器free(gauss_out);释放高斯噪声数组，否则内存泄漏,but i 在解码函数释放

			//printf("\n %d us", (end_t - start_t) * 1000000 / cpu_freq);

			for (uint16_t i = 0; i < N / 2 - CRCBYTE*8; i++) {
				if (*(finalCode + i) != *(unCoCode + i)) {
					errorTime++;
					break;
				}
			} 
		}
		bler = errorTime / (double)forTestPackup;
		printf("Set%d L=%d N=%d EBNOI=%g BLER=%g avrgTime=us\n", set, L, N, ebnoi, bler);
	}

	getchar();
	//system("pause");
}
