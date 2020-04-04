#define SCL_DECO_C_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "scl_alg.h"
#include "bitree.h"
#include "scl_deco.h"

#define S_PDATA_CUR_INHRT  SpathData[curStruNum]//继承data
#define S_PDATA_CUR_CP SpathData[curStruNum + *(pathOffset + mesBitNum)]//复制data,并行的路径*(pathOffset + mesBitNum)条，也是宏定义路径复制偏移量
#define S_PMES_CUR_INHRT  SpathMes[curStruNum]//继承mes
#define S_PMES_CUR_CP SpathMes[curStruNum + *(pathOffset + mesBitNum)]//复制mes
#define S_PMES_SNIP_CP SpathMes[curStruNum + L]//剪枝复制mes
#define END_LLR(ROL) (SpathMes[curStruNum].ptrSpathData->llr[0])//当前结构体最终llr

#define CUR_STAGE_BASE_ADDR(COL) (SpathMes[k].ptrSpathData->llr+colBaseTab[(COL)])
#define EX_STAGE_BASE_ADDR(COL) (SpathMes[k].ptrSpathData->llr+colBaseTab[(COL)+1])
const uint16_t colBaseTab[11] = { 0,1,3,7,15,31,63,127,255,511,1023 };//每一个stage在LLR数组的起始位置

const uint8_t pathNum[6] = { 1,2,4,8,16,32};//路径条数
const uint8_t pathOffset[6] = {0,1,2,4,8,16};
uint8_t mesBitNum;//当前信息位个数，是pathNum[6]索引,终止于L,第32条最大mesBitNum=6，
uint8_t fgMesBitNum;//fg做完k次才更新为mesBitNum
pathData SpathData[L];
pathMes SpathMes[2 * L];
uint8_t backToUnCoCode[N / 2];//去除固定比特，记录译码结果

//根据PM快速排序降序排列，取出PM前一半大更新解码值，PM，反馈值
int falling_cmp(const void *p1, const void *p2) {
	return ((ptrPathMes)p2)->pathMea > ((ptrPathMes)p1)->pathMea ? 1:-1;
}
//反馈值，参数：当前行，解码值，反馈值数组
void sumList(int16_t curRol, uint8_t code, uint8_t *sum) {//num 代表这是解码结果第几行0,1，2，3 ；code代表本位解码结果;sum解码值数组
	int16_t i, j;
	int16_t num;

	num = curRol;
	*(sum + num) = code;//无论奇数偶数，0列当前行做一次，
//允许计算到的列i增加的循环条件num > 0  && (num % 2 == 1)只做奇数行(0开始)，不做偶数，每列的当前行及上方连续行共算*(difNodTab + i)次即f个数，g不变，继承了前方
	if ((curRol + 1) % 2 == 0) {//奇数行才能进
		for (i = 0; num > 0 && (num % 2 == 1); i++) {//3做一次，5做两次，7做3次
			for (j = 0; j < *(difNodTab + i); j++) {//1列一次，2列2次，3列4次，此处将来还可以少做最后一次和码长等长的求反馈值。
				*(sum + curRol - j - *(difNodTab + i)) = (*(sum + curRol - j - *(difNodTab + i)) + *(sum + curRol - j)) % 2;
			}
			num >>= 1;// num /= 2;	,因为2/2=1不能仅仅借用此条件而不去做最外层判断	
		}
	}
}
//前L个不剪枝的算法,做完后前32Mes和所有Data一一对应
void total_cp_alg(int16_t curRol, uint8_t curStruNum) {//curSrucNum代表fg正在处理哪一个结构体
	uint8_t i;

	if (*(frz + curRol) == 0) {	//frozen bite,只继承不复制
		if (END_LLR(curRol) < 0) {//llr<0 PM = PM - llr; llr>0 PM  = PM
			S_PMES_CUR_INHRT.pathMea -= (float)fabs(END_LLR(curRol));
		}
		*(S_PDATA_CUR_INHRT.deCoCode + curRol) = 0;//解码值只能是0，反馈只有一种
		sumList(curRol, 0, S_PDATA_CUR_INHRT.sumForGnod);//不分裂，只在继承路径
	}
	else { //message bit，路径小于L所以必须复制,注意：只复制一次，total_cp_alg()会根据当前路径数目被fg调用多次
		if (curStruNum == 0) {//每一个fg只做一次，否则乱套
			mesBitNum++;//自加后代表当前信息位个数 
		}
		S_PDATA_CUR_CP = S_PDATA_CUR_INHRT;//Data整体后移复制
		S_PMES_CUR_CP.pathMea = S_PMES_CUR_INHRT.pathMea;//把PM保存过去
		if (END_LLR(curRol) > 0) {//SCL，最终llr>0
			//PM较大
			*(S_PDATA_CUR_INHRT.deCoCode + curRol) = 0;//PM较大的继承父结构体，更新解码值，PM不变
			sumList(curRol, 0, S_PDATA_CUR_INHRT.sumForGnod);
			//PM较小
			S_PMES_CUR_CP.pathMea -= (float)fabs(END_LLR(curRol));//PM较小，PM-llr
			*(S_PDATA_CUR_CP.deCoCode + curRol) = 1;//PM较小更新解码值			
			sumList(curRol, 1, S_PDATA_CUR_CP.sumForGnod);
		}
		else {//SCL，最终llr<=0
			 //PM较大
			*(S_PDATA_CUR_INHRT.deCoCode + curRol) = 1;//PM较大的继承父结构体，更新解码值，PM不变
			sumList(curRol, 1, S_PDATA_CUR_INHRT.sumForGnod);
			//PM较小
			S_PMES_CUR_CP.pathMea -= (float)fabs(END_LLR(curRol));
			*(S_PDATA_CUR_CP.deCoCode + curRol) = 0;//PM较小更新解码值
			sumList(curRol, 0, S_PDATA_CUR_CP.sumForGnod);
		}
		//这玩意统共只做一次，虽被fg循环，但在不剪枝时处理第L个信息位只有一次，前32Mes排序,当爹,Mes认Data
		if (curStruNum == L/2 - 1) {//已经完成32个Mes和Data
			//前L个mes降序排列，排序后SpathMes[]按SpathMes[i].ptrSpathData->pathMea大小降序排列
			qsort(&SpathMes[0], L, sizeof(SpathMes[0]), falling_cmp);
			for (i = 0; i < L; i++) {//第一次给dadRank赋值，意味着dadRank只与Mes结构体有关，0最大，L-1最小
				SpathMes[i].dadRank = i;
			}			
		}
	}
}
//snip_path_copy()做完得到前32Mes占用仅仅的32个SpathData结构体
//此时fg以及Mes刚刚处理完毕第2L个Mes结构体,需要对现有2L个Mes结构体L个Data结构体处理，以L=32说明
//做法：dadRank相同，优胜父亲的小PM儿子占领失败dad的data结构体，其余前32Mes的一子继承父
//注意1：64个PM降序排列，保证所有父亲的大儿子在前小儿子在后，进而二儿子同时是PM较小的儿子
//注意2：SpathData[i]与dadRank不一致，故[i]顺序不代表大小次序，SpathMes本身就是为了避免排序时SpathData物理交换
void snip_path_copy() {
	uint8_t i;
	uint8_t copyTime = 0;//记录做几次占领
	uint8_t seSonNum = 0;//计数变量，记录有超级或失败dad个数，每次重新归零
	uint8_t successDad[L] = { 0 };//元素值为2时找到优胜二儿
	uint8_t failureDad[L] = { 0 };//元素值为2时找到失败兄弟中PM较小的		
	uint8_t seSonIndex[L] = { 0 };//记录优胜二儿子Mes结构体下标,最多L/2个二儿子,但他们的下标0~31
	uint8_t noSonIndex[L] = { 0 };//记录失败兄弟中PM较小结构体下标，最多L/2个(ensure一个失败父亲only接受一次占领)
	//2L个mes降序排列
	qsort(&SpathMes[0], 2 * L, sizeof(SpathMes[0]), falling_cmp);	
    //对前32PM的Mes遍历,有优胜二儿子就是超级父亲
	for (seSonNum = 0, i = 0; i < L; i++) {
		successDad[SpathMes[i].dadRank]++;//数组dadRank，数组元素值代表相同dadRank的儿子个数
		if (successDad[SpathMes[i].dadRank] == 2) {//类似唱票,dadRank是候选人,i是投票人,现在要找出第二个投票人
			copyTime++;//成功父亲与失败父亲一样多
			seSonIndex[seSonNum++] = i;//mark i，记录二儿子同时是PM较小的儿子所在的Mes结构体下标
		}
	}
	//对后32PM的Mes遍历,有失败二儿子就是失败父亲，和上边类似(可怜父亲，为了安慰就借给你seSonNum用一下)
	for (seSonNum = 0, i = L; i < 2 * L; i++) {//对后32PM的Mes遍历
		failureDad[SpathMes[i].dadRank]++;//数组dadRank，数组元素值代表相同dadRank的儿子个数
		if (failureDad[SpathMes[i].dadRank] == 2) {//类似唱票,dadRank是候选人,i是投票人,现在要找出第二个投票人
			noSonIndex[seSonNum++] = i;//mark i，记录二儿子同时是PM较小的儿子所在的Mes结构体下标
		}
	}
	//MesData occupy!!超级爹的第二个儿子同时是PM较小的儿子覆盖失败的父亲
	for (i = 0; i <copyTime; i++) {
		*(SpathMes[(noSonIndex[i])].ptrSpathData) = *(SpathMes[(seSonIndex[i])].ptrSpathData);//Data内容替换
		SpathMes[(seSonIndex[i])].ptrSpathData = SpathMes[(noSonIndex[i])].ptrSpathData;//指向Data的指针替换
	}
	for (i = 0; i < L; i++) {	//对前32个PM喜当爹安排一下dadRank,后只能有L个dadRank
		SpathMes[i].dadRank = i;
	}
}
//剪枝算法
void snip_alg(int16_t curRol, uint8_t curStruNum) {//curSrucNum代表fg正在处理的结构体
	uint8_t i;

	if (*(frz + curRol) == 0) {	//frozen bite,只继承不复制
		if (END_LLR(curRol) < 0) {//llr<0 PM = PM - llr; llr>0 PM  = PM
			S_PMES_CUR_INHRT.pathMea -= (float)fabs(END_LLR(curRol));//当前处理的结构体
		}
		*(S_PDATA_CUR_INHRT.deCoCode + curRol) = 0;
		sumList(curRol, 0, S_PMES_CUR_INHRT.ptrSpathData->sumForGnod);//不分裂，只在继承路径
	}
	else { //message bit，Mes复制，只改PM（这里只复制一次，会根据当前路径数目调用多次完成全部）
		S_PMES_SNIP_CP = S_PMES_CUR_INHRT;//整体后移复制，主要是把dadRank,PM保存过去,tmpDeCoCode还会改
		if (END_LLR(curRol) > 0) {//SCL，最终llr>0,只改变Mes中PM,PM较大的继承PM解码0
			//PM较大
			S_PMES_CUR_INHRT.tmpDeCoCode = 0;
			//PM较小
			S_PMES_SNIP_CP.pathMea -= (float)fabs(END_LLR(curRol));//PM较小，PM-llr，解码1
			S_PMES_SNIP_CP.tmpDeCoCode = 1;
		}
		else {//SCL，最终llr<0，只改变Mes中PM，PM较大的继承PM解码1
		    //PM较大
			S_PMES_CUR_INHRT.tmpDeCoCode = 1;
			//PM较小
			S_PMES_SNIP_CP.pathMea -= (float)fabs(END_LLR(curRol));
			S_PMES_SNIP_CP.tmpDeCoCode = 0;
		}
		if (curStruNum == L - 1) {//2L条路径PM计算完毕
			snip_path_copy();//路径复制完毕，且SpathMes[i]已经按照PM降序排列
			for (i = 0; i < L; i++) {//下面对新晋的L条更新解码值，反馈值
				*(SpathMes[i].ptrSpathData->deCoCode + curRol) = SpathMes[i].tmpDeCoCode;
				if (curRol != N - 1) {//最后一个码不必求反馈
					sumList(curRol, SpathMes[i].tmpDeCoCode, SpathMes[i].ptrSpathData->sumForGnod);
				}
			}
		}
	}
}
void fnod(int16_t iniRol, int16_t col) {  //iniRow确定起始行在数组位置，iniRow确定行位置，col确定列												
	int16_t i = 0;/*记录负号个数*/
	uint8_t k;//路径复制后，fg都要做多次
	int16_t j;//当前行，以iniRol为起始
	float a, b;

	for (k = 0; k < *(pathNum + fgMesBitNum); k++) {//并行的路径*(pathNum + mesBitNum条，也是宏定义路径复制偏移量 
		for (j = 0; j < *(difNodTab + col); j++) {//列数左至右0，1，2决定连续f操作次数以及f输入变量间隔
			i = 0;//每次f操作负号计数变量清零
			a = *(EX_STAGE_BASE_ADDR(col) + j);//每列 用前一列的llr
			b = *(EX_STAGE_BASE_ADDR(col) + *(difNodTab + col) + j);
			if (a < 0) { i++; a = -a; }
			if (b < 0) { i++; b = -b; }
			if (b < a) { a = b; }//以a记录较小数

			if (i == 1)//符号函数为负数
				*(CUR_STAGE_BASE_ADDR(col) + j) = -a;
			else
				*(CUR_STAGE_BASE_ADDR(col) + j) = a;
			if (col == 0) {//解到最后的LLR了,路径复制吧,与g相同,要改一起改
				if (*(pathNum + fgMesBitNum) <L) {
					total_cp_alg(iniRol, k);//把解码行数,当前处理结构体传入
				}
				else {
					snip_alg(iniRol, k);//分裂剪枝维持L条路径
				}
			}
		}
	}
	fgMesBitNum = mesBitNum;//做完一个fg节点才能更新
}

void gnod(int16_t iniRol, int16_t col) { //a必须为左值,*sum的单行向量存储对应反馈值
	if(iniRol == 7){
		int test = 0;
	}
	int16_t j;//最后一个bug，j不能是uint8_t
	uint8_t  k;
	float a, b;
	int16_t sum;

	for (k = 0; k < *(pathNum + fgMesBitNum); k++) {//并行的路径
		for (j = 0; j < *(difNodTab + col); j++) {//列数左至右0，1，2决定连续g操作次数以及g输入变量间隔
			sum = *(SpathMes[k].ptrSpathData->sumForGnod + iniRol + j - *(difNodTab + col));//反推所需反馈值所在行,即每个g对应的f所在行位置
			a = *(EX_STAGE_BASE_ADDR(col) + j);//反馈值节点
			b = *(EX_STAGE_BASE_ADDR(col) + *(difNodTab + col) + j);
			if (sum == 0) { *(CUR_STAGE_BASE_ADDR(col) + j) = a + b; }						
			else { *(CUR_STAGE_BASE_ADDR(col) + j) = b - a; }							
			if (col == 0) {//解到最后的LLR了,与f相同,要改一起改
				if (*(pathNum + fgMesBitNum) <L) {
					total_cp_alg(iniRol, k);//把解码行数,当前处理结构体传入
				}
				else {
					snip_alg(iniRol, k);//分裂剪枝维持L条路径
				}
			}
		}
	}
	fgMesBitNum = mesBitNum;//做完一个fg节点才能更新
}
/*将接收值进行第一次似然译码*/
void firstLLR(float *gauss_out, double dx) {
	for (uint16_t i = 0; i < N; i++) {
		*(SpathData[0].llr + N - 1 + i) = (float)(*(gauss_out + i) * 2 / dx);//输入的LLR在数组最右侧
	}
}
//解码CRC
uint32_t Cheak_CRC_24(unsigned char * Data_in, uint32_t lenth) {//此CRC只适合一个字节表示一个比特的polar码,lenth为码长
	uint32_t R = 0;

	for (uint32_t i = 0; i < lenth; i++) {
		R = (R << 1) + Data_in[i];
		if (R >= 0x01000000)
			R = (R ^ CRC24_Gx);
	}
	return R;
}
uint8_t* selectPassCRC() {
	int16_t i,j,k;

	for (k = 0; k < L; k++) {
		for (i = 0, j = 0; i < N; i++) {
			if (*(frz + i) == 1) {
				*(backToUnCoCode + j++) = *(SpathMes[k].ptrSpathData->deCoCode + i);
			}
		}
		if (Cheak_CRC_24(backToUnCoCode, N / 2) == 0) {
			return backToUnCoCode;
		}
	} 
	return SpathMes[0].ptrSpathData->deCoCode;
}

//在多次解码仿真时，第二次开始全局变量PM值初始将不为0，mesBitNum初始将不为0，所以每次解码需要归零
void decodeInit() {
	uint8_t i;

	mesBitNum = 0;
	fgMesBitNum = 0;
	SpathMes[0].pathMea = 0;
	for (i = 0; i < L; i++) {//将Mes指向Data，之前是自然对应，以后靠指针联系
		SpathMes[i].ptrSpathData = &SpathData[i];//2L防止多次测试问题
	}
}
//解码函数
uint8_t* decode(float *gauss_out,double dx) {
	bitree root;

	decodeInit();
	firstLLR(gauss_out,dx);//进行第一次似然译码，生成一个N行，M+1列的LLR矩阵
	free(gauss_out);//将高斯噪声数组释放
	root = creatTree(M + 1, 0);//根节点col:M+1 Rol:0						   
	printBitree(root, fnod);//fnod对于根节点被printBitree()中if掉了，只是凑个形式数
	delBitree(root);//解码结果存入int16_t deCoCode[8]可以释放树
	return selectPassCRC();
}


