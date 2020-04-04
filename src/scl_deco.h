
#ifndef SCL_DECO_H_
#define SCL_DECO_H_
typedef struct//存储路径信息
{
	float llr[2 * N - 1];
	uint8_t deCoCode[N];//记录译码结果
	uint8_t sumForGnod[N]; //反馈值
}pathData, *ptrPathData;

typedef struct//用于比较路径度量值，较大PM覆盖较小PM的存储单元
{
	float pathMea;//路径度量值，由于pathData只有L个，PM要2L个
	uint8_t tmpDeCoCode;
	uint8_t dadRank;//记录dad排名,son排序完成后，son有相同dad的话，PM较小的son替换排名倒数的dad
	ptrPathData ptrSpathData;
}pathMes, *ptrPathMes;

#ifndef SCL_DECO_C_
extern void fnod(int16_t iniRol, int16_t col);
extern void gnod(int16_t iniRol, int16_t col);
extern uint8_t* decode(float *gauss_out,double dx);
extern pathMes SpathMes[2 * L];
#endif 

#endif // !SCL_DECO_H_