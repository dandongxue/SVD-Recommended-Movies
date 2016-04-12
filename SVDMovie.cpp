#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
using namespace std;
const int USERMAX = 6;//用户编号上限
const int ITEMMAX = 7;//电影编号上限
const int FEATURE = 30;//自定义特征因子数目feature50  可变
const int ITERMAX = 170;//定义迭代次数
double rating[USERMAX][ITEMMAX];//存放打分记录表
int I[USERMAX][ITEMMAX];//indicate if the item is rated
double UserF[USERMAX][FEATURE];//分解后的用户特征表
double ItemF[ITEMMAX][FEATURE];//分解后的电影特征表
double BIASU[USERMAX];//用户偏差
double BIASI[ITEMMAX];//特征值偏差
double lamda = 0.15;  //
double gamma = 0.04;  //这是啥参数
double mean;// 电影平均打分值
double predict(int i, int j)//预测i号用户对j电影的预期值
{
	double rate = mean + BIASU[i] + BIASI[j];//电影总体平均值+用户单体偏差+电影单体偏差
	for (int f = 0; f < FEATURE; f++)
		rate += UserF[i][f] * ItemF[j][f];//统计结果值  不错不错 开始明了了
	
	if (rate < 1)
		rate = 1;     //确保合理的范围
	else if (rate>5)
		rate = 5;
	return rate;
}
double calRMSE()//统计下均方差 看下是否变小
{
	int cnt = 0;
	double total = 0;
	for (int i = 0; i < USERMAX; i++)
	{
		for (int j = 0; j < ITEMMAX; j++)
		{
			double rate = predict(i, j);
			total += I[i][j] * (rating[i][j] - rate)*(rating[i][j] - rate);//只统计非零的，也就是进行过打分的
			cnt += I[i][j];
		}
	}
	double rmse = pow(total / cnt, 0.5);
	return rmse;
}
double calMean()//总体电影打分平均值
{
	double total = 0;
	int cnt = 0;
	for (int i = 0; i < USERMAX; i++)
		for (int j = 0; j < ITEMMAX; j++)
		{
			total += I[i][j] * rating[i][j];
			cnt += I[i][j];
		}
	return total / cnt; 
}
void initBias()//初始化用户偏差和电影偏差
{
	memset(BIASU, 0, sizeof(BIASU));
	memset(BIASI, 0, sizeof(BIASI));
	mean = calMean();//总体电影打分平均值
	cout<<"mean: "<<mean<<endl;
	for (int i = 0; i < USERMAX; i++)
	{
		double total = 0;
		int cnt = 0;
		for (int j = 0; j < ITEMMAX; j++)
		{
			if (I[i][j]) //这是一个标记位 用于指示用户是否为影片打过分
			{
				total += rating[i][j] - mean;//统计每个用户打分的偏差
				cnt++;
			}
		}
		if (cnt > 0)
			BIASU[i] = total / (cnt);//用户的偏差
		else
			BIASU[i] = 0;
	}
	for (int j = 0; j < ITEMMAX; j++)
	{
		double total = 0;
		int cnt = 0;
		for (int i = 0; i < USERMAX; i++)
		{
			if (I[i][j])
			{
				total += rating[i][j] - mean;//统计电影打分偏差
				//if(j==5)cout<<"==:"<<rating[i][j] - mean<<" ==";
				cnt++;
			}
		}
		if (cnt > 0)
			BIASI[j] = total / (cnt);//电影打分的偏差
		else
			BIASI[j] = 0;
	}
}
void train()
{
	//read rating matrix
	memset(rating, 0, sizeof(rating));
	memset(I, 0, sizeof(I));
	ifstream in("ua.txt");
	if (!in)
	{
		cout << "file not exist" << endl;
		exit(1);
	}
	int userId, itemId, rate;
	string timeStamp;
	while (in >> userId >> itemId >> rate)
	{
		rating[userId][itemId] = rate;//记录用户给每个电影的打分情况打分
		I[userId][itemId] = 1;//标记位，用于指示是否为其打过分
		if(rate==0)
			I[userId][itemId]=0;

	}
	initBias();//统计偏差
	//train matrix decomposation
	//P,Q应该的初始化，一般使用  0.1 * rand(0,1) / sqrt(dim)  dim指特征的维数
	for (int i = 0; i < USERMAX; i++)
		for (int f = 0; f < FEATURE; f++)
			UserF[i][f] = 0.1*((rand() % 10)/10.0)/sqrt(20) ;//为分界矩阵用户因子随机生成初始值 这是一种有效的初始化优方法
	for (int j = 0; j < ITEMMAX; j++)
		for (int f = 0; f < FEATURE; f++)
			ItemF[j][f] = 0.1*((rand() % 10)/10.0)/sqrt(20) ;//为分界矩阵影片因子随机生成初始值
	int iterCnt = 0;
	while (iterCnt++< ITERMAX) //设定迭代次数20次
	{
		for (int i = 0; i < USERMAX; i++)
		{
			for (int j = 0; j < ITEMMAX; j++)
			{
				if (I[i][j])//代表未对这部电影打分  以下是核心处理阶段
				{
					double predictRate = predict(i, j);
					double eui = rating[i][j] - predictRate;//这是预测值和实际值的打分偏差
					BIASU[i] += gamma*(eui - lamda*BIASU[i]);//偏差值调整
					BIASI[j] += gamma*(eui - lamda*BIASI[j]);//偏差值调整
					for (int f = 0; f < FEATURE; f++)
					{
						UserF[i][f] += gamma*(eui*ItemF[j][f] - lamda*UserF[i][f]);//这个偏差矫正公式没理解
						ItemF[j][f] += gamma*(eui*UserF[i][f] - lamda*ItemF[j][f]);//这个偏差矫正公式没理解
					}				
				}
			}
		}
		double rmse = calRMSE();
		cout << "Loop " << iterCnt << " : rmse is " << rmse << endl;
	}
}
void test()
{
	ifstream in("ua.test");
	if (!in)
	{
		cout << "file not exist" << endl;
		exit(1);
	}
	int userId, itemId, rate;
	string timeStamp;
	double total = 0;
	double cnt = 0;
	while (in >> userId >> itemId >> rate >> timeStamp)
	{
		double r = predict(userId, itemId);
		total += (r - rate)*(r - rate);
		cnt += 1;
	}
	cout << "test rmse is " <<total/cnt<<"   "<<pow(total / cnt, 0.5) << endl;
}
int main()
{
	train();
	int ForMov=0;
	double temp=-100;
	for(int i=1;i<ITEMMAX;i++)
	{	  
	   if(I[5][i]==0)
	   {
		   if(predict(5,i)>temp)
		   {
		   temp=predict(5,i);
		   ForMov=i;
		   }
	   }
	}
	cout<<"预测电影： "<<ForMov<<endl;
//	cout<<"PredictNum:"<<predict(5,3)<<" 54 "<<predict(5,4)<<" 55 "<<predict(5,5)<<" 46 "<<predict(4,6)<<endl;
//	test();
	return 0;
}
