#include <bits/stdc++.h>

using namespace std;

double x[10];
double area, upperTankRatio, lowerTankRatio;
int t, size, maxDayDelay;
string line, word;
istringstream ss;
vector<double> P, E, Qobs, Qobs_mm;
vector<double> Pn, En, Ps, Es, Perc, Pr;
vector<double> SH1, UH1, SH2, UH2;
double S0, R0, S_TEMP, R_TEMP, Ratio_TEMP;
vector<double> S, R, Qr, Qd, Q;
double F, R_Fast, R_Slow;
int cnt;
double Q_accum, Q_ave, NSE, Q_diff1, Q_diff2;

double SH1_CURVE(double t, double x4);
double SH2_CURVE(double t, double x4);

int main()
{
    // 第一步：读取GR4J模型所需要得数据，为模型执行做准备
    // 加载GR4J模型参数
	// x1: 产流水库容量 (mm)
	// x2: 地下水交换系数 (mm)
	// x3: 汇流水库容量 (mm)
	// x4: 单位线汇流时间 (天)
	
	ifstream infile1("GR4J_Parameter.csv", ios::in);
	if(!infile1.is_open())
	{
		cout << "Error: opening file GR4J_Parameter.csv fail" << endl;
		return 0;
	}

	t = 1;
	while(getline(infile1, line))
	{
		ss.clear();
		ss.str(line);
		while (getline(ss, word, ','))
		{
			if(t % 2 == 1)	x[t / 2 + 1] = stod(word);
			t ++;
		}
	}
	infile1.close();
	// for(int i = 1; i <= 4; i ++)	cout << "x" << i << ": " << x[i] << endl;

	// 加载GR4J其他参数
	// area: 流域面积(km2)
	// upperTankRatio: 产流水库初始填充率 S0/x1
	// lowerTankRatio: 汇流水库初始填充率 R0/x3
	ifstream infile2("others.csv", ios::in);
	if(!infile2.is_open())
	{
		cout << "Error: opening file others.csv fail" << endl;
		return 0;
	}

	t = 1;
	while(getline(infile2, line))
	{
		ss.clear();
		ss.str(line);
		while (getline(ss, word, ','))
		{
			if(t == 1)	area = stod(word);
			else if(t == 3)	upperTankRatio = stod(word);
			else if(t == 5)	lowerTankRatio = stod(word);
			t ++;
		}
	}
	infile2.close();
	// cout << "area: " << area << "\nupperTankRatio: " << upperTankRatio << "\nlowerTankRatio: " << lowerTankRatio << endl;

	// 加载数据文件
	// P: 日降雨量(mm)
	// E: 蒸散发量(mm)
	// Qobs: 流域出口观测流量(m3/s)
	ifstream infile3("inputData.csv", ios::in);
	if(!infile3.is_open())
	{
		cout << "Error: opening file inputData.csv fail" << endl;
		return 0;
	}

	t = 0;
	P.push_back(0.0);
	E.push_back(0.0);
	Qobs.push_back(0.0);
	while(getline(infile3, line))
	{
		ss.clear();
		ss.str(line);
		while (getline(ss, word, ','))
		{
			if(t % 3 == 0)	P.push_back(stod(word));
			else if(t % 3 == 1)	E.push_back(stod(word));
			else if(t % 3 == 2)	Qobs.push_back(stod(word));
			t ++;
		}
	}
	infile3.close();
	// for(auto p : P)	cout << p << endl;
	// for(auto e : E)	cout << e << endl;
	// for(auto Qob : Qobs)	cout << Qob << endl;

	// 将径流量单位从m3/s转换为mm/d，其中86.4为转化系数3600*24*1000/10^6
	for(int i = 0; i < Qobs.size(); i ++)	Qobs_mm.push_back(Qobs[i] * 86.4 / area);
	// for(int i = 1; i < Qobs.size(); i ++)		cout << fixed << setprecision(4) << Qobs_mm[i] << endl;
	// for(auto Qob : Qobs_mm)	cout << Qob << endl;

	// 第二步：根据逐日降雨量及逐日蒸发量，计算流域出口断面逐日径流量
	size = P.size() - 1;
	// cout << size << endl;

	// 初始化变化，存储GR4J模型中间变量值
	// Pn：降雨扣除损失（蒸发）后得净雨
	// En：当日蒸发未被满足部分，此部分未得到满足得蒸发将消耗土壤中得水分
	// Ps：中间变量，记录净雨补充土壤含水量
	// Es: 中间变量，记录剩余蒸发能力消耗土壤含水量
	// Perc: 中间变量，记录产流水库壤中流产流量
	// Pr: 记录产流总量
	vector<double> tmp1(size + 1, 0.0);
	Pn = tmp1, En = tmp1, Ps = tmp1, Es = tmp1, Perc = tmp1, Pr = tmp1;

	// 根据输入参数x4计算S曲线以及单位线，这里假设单位线长度UH1为10，UH2为20;即x4取值不应该大于10
	maxDayDelay=10;
	// 定义几个数组以存储SH1,UH1,SH2,UH2
	vector<double> tmp2(maxDayDelay + 1, 0.0);
	SH1 = tmp2, UH1 = tmp2;
	vector<double> tmp3(2 * maxDayDelay + 1, 0.0);
	SH2 = tmp3, UH2 = tmp3;

	// 计算SH1以及SH2
	for(int i = 1; i <= maxDayDelay; i ++)	SH1[i] = SH1_CURVE(i,x[4]);
	for(int i = 1; i <= 2 * maxDayDelay; i ++)	SH2[i] = SH2_CURVE(i,x[4]);

	// 计算UH1以及UH2
	for(int i = 1; i <= maxDayDelay; i ++)
	{
		if(i == 1)	UH1[i] = SH1[i];
		else UH1[i] = SH1[i] - SH1[i - 1];
	}
    for(int i = 1; i <= 2 * maxDayDelay; i ++)
	{
		if(i == 1)	UH2[i] = SH2[i];
		else UH2[i] = SH2[i] - SH2[i - 1];
	}
    
	// for(int i = 1; i <= maxDayDelay; i ++)	cout << SH1[i] << " ";
	// cout << endl;
	// for(int i = 1; i <= 2 * maxDayDelay; i ++)	cout << SH2[i] << " ";
	// cout << endl;
	// for(int i = 1; i <= maxDayDelay; i ++)	cout << UH1[i] << " ";
	// cout << endl;
	// for(int i = 1; i <= 2 * maxDayDelay; i ++)	cout << UH2[i] << " ";
	// cout << endl;

	// 计算逐日En及Pn值,En及Pn为GR4J模型得输入，可以提前计算出来
	for(int i = 1; i <= size; i ++)
	{
		if(P[i] >= E[i])
		{
			Pn[i] = P[i] - E[i];
      		En[i] = 0;
		}
		else
		{
			Pn[i] = 0;
       		En[i] = E[i] - P[i];
		}
	}
	// for(int i = 1; i <= size; i ++)
	// {
	// 	cout << Pn[i] << " " << En[i] << endl;
	// }

	// 定义一些产汇流计算需要用到得数值及数组
	// S0: 产流水库初始土壤含水量=比例*产流水库容量
	// R0: 汇流水库初始土壤含水量=比例*汇流水库容量
	// S：产流水库逐日水量
	// R：汇流水库逐日水量
	// S_TEMP: 存储当前产流水库储量
	// R_TEMP: 存储当前汇流水库储量
	// Qr：汇流水库快速流出流量
	// Qd：汇流水库慢速流出流量
	// Q：汇流总出流量
	// UH_Fast: 用于记录UH1单位线作用下得产流信息
	// UH_Slow: 用于记录UH2单位线作用下得产流信息

	S0 = upperTankRatio * x[1];
	R0 = lowerTankRatio * x[3];
	S_TEMP = S0;                          
	R_TEMP = R0;
	vector<double> tmp4(size + 1, 0.0);
	S = tmp4, R = tmp4, Qr = tmp4, Qd = tmp4, Q = tmp4;
	vector<vector<double>> UH_Fast(size + 1, vector<double>(maxDayDelay + 1));
	vector<vector<double>> UH_Slow(size + 1, vector<double>(maxDayDelay * 2 + 1));                         
	
	// 获取En及Pn值后，更新产流水库蓄水量
	for(int i = 1; i <= size; i ++)
	{
		// cout << fixed << setprecision(3);
		// cout << S_TEMP / x[1] << " ";
		// cout << Pn[i] << " ";
		// cout << En[i] << " ";
		S[i] = S_TEMP;
		R[i] = R_TEMP;
		// Pn(i)及En(i)有且仅有一个大于0，所以下面两个if判断只可能一个为真
		// 净雨量大于0，此时一部分净雨形成地面径流，一部分下渗
		if(Pn[i] != 0)
		{
			Ps[i] = x[1] * (1 - pow(S[i] / x[1], 2)) * tanh(Pn[i] / x[1]) / (1 + S[i] / x[1] * tanh(Pn[i] / x[1]));    
      		Es[i] = 0;
		}
		// 净蒸发能力大于0，此时土壤中水分一部分用于消耗
		if(En[i] != 0)
		{
			Ps[i] = 0;
      		Es[i] = (S[i] * (2 - (S[i] / x[1])) * tanh(En[i] / x[1])) / (1 + (1 - S[i] / x[1]) * tanh(En[i] / x[1]));   
		}
		// cout << Ps[i] << " ";
		// cout << Es[i] << " ";
		// 更新上层水库蓄水量, 此处仅仅用于同学们理解，便于与Excel表格对照
		S_TEMP = S[i] - Es[i] + Ps[i];
    	Ratio_TEMP = S_TEMP / x[1];
		// 计算产流水库渗漏，这部分可以视作壤中流
		Perc[i] = S_TEMP * (1 - pow(1 + pow(4.0 / 9.0 * S_TEMP / x[1], 4), -0.25));
		// cout << S_TEMP / x[1] << " ";
		// cout << Perc[i] << " ";
		// 计算产流总量,其中Pn-Ps可以视作为地表产流
		Pr[i] = Perc[i] + (Pn[i] - Ps[i]);
		// 更新当前产流水库水量，作为次日产流水库水量
		S_TEMP = S_TEMP - Perc[i];
		// cout << S_TEMP / x[1] << " ";
		// cout << Pr[i];
		// 至此产流过程全部结束，进入汇流阶段

		// 汇流计算
		// 计算地表水与地下水之间得水量交换
		F = x[2] * pow(R[i] / x[3], 3.5);
		// cout << R[i] / x[3];
		// cout << F;
		// 计算地表水汇流，将产流量按照90%(快速)和10%(慢速)划分
		// 快速地表径流汇流使用单位先UH1; 慢速地表径流汇流使用单位先UH2; 
		R_Fast = Pr[i] * 0.9;
   		R_Slow = Pr[i] * 0.1;
		
		// double UH_Fast[size + 1][maxDayDelay + 1];
		// double UH_Slow[size + 1][maxDayDelay * 2 + 1]; 
		if(i == 1)
		{
			// 第1时段产流量在时间上的分配
			for(int j = 1; j <= maxDayDelay; j ++)
				UH_Fast[i][j] = R_Fast * UH1[j];
			// 第1时段产流量在时间上的分配
			for(int j = 1; j <= maxDayDelay * 2; j ++)
				UH_Slow[i][j] = R_Slow * UH2[j];
		}
		else
		{
			// 先计算当前时段产流量在时间上得分配
			for(int j = 1; j <= maxDayDelay; j ++)
				UH_Fast[i][j] = R_Fast * UH1[j];
			for(int j = 1; j <= maxDayDelay - 1; j ++)
				UH_Fast[i][j] = UH_Fast[i][j] + UH_Fast[i - 1][j + 1];
			// 先计算当前时段产流量在时间上得分配
			for(int j = 1; j <= maxDayDelay * 2; j ++)
				UH_Slow[i][j] = R_Slow * UH2[j];
			for(int j = 1; j <= 2 * maxDayDelay - 1; j ++)
           		UH_Slow[i][j] = UH_Slow[i][j] + UH_Slow[i - 1][j + 1];
		}
		// 更新汇流水库水量变化;
		R_TEMP = max(0.0, R_TEMP + UH_Fast[i][1] + F);
		// cout << R_TEMP / x[3] << " ";
		// 计算汇流水库快速流出流量
		Qr[i] = R_TEMP * (1 - pow(1 + pow(R_TEMP / x[3], 4), -0.25));
		// 再次更新汇流水库水量变化
		R_TEMP = R_TEMP - Qr[i];
		// cout << Qr[i] << " " << R_TEMP / x[3] << " ";
		// 计算汇流水库慢速流出流量
		Qd[i] = max(0.0, UH_Slow[i][1] + F);

		// 计算汇流总出流量
		Q[i] = Qr[i] + Qd[i];
		// cout << Qd[i] << " ";
		// cout << Q[i];
		// cout << endl;
	}

	// 精度评估
	// 1. 通过调整模型参数，使得模型得到更好的拟合效果的加分，具体视Nash-Sutcliffe指数
	// 2. 假设第一年365天作为预热期，消除产流水库和汇流水库初始值选取对模拟精度的影响
	// cnt: 计数器：记录总天数
	// Q_accum: 记录累计径流量
	// Q_ave: 记录平均径流量
	// NSE: 记录纳什效率系数
	double minn = 1.0, ansk = 1.0, maxn = 0.0;
	int tmp = 0;
	for(int k = 1; k < 10000; k ++)
	{
		cnt = 0, Q_accum = 0.0, Q_ave = 0.0, Q_diff1 = 0.0, Q_diff2 = 0.0, NSE = 0.0;
		for(int i = size * k / 10000; i <= size; i ++)
		{
			cnt ++;
			Q_accum += Qobs_mm[i];
		}
		// cout << cnt << endl;
		// 计算观测流量平均值
		Q_ave = Q_accum / cnt;
		// cout << Q_ave << endl;
		for(int i = size * k / 10000; i <= size; i ++)
		{
			// 计算Nash-Sutcliffe指数分子
			Q_diff1 = Q_diff1 + pow(Qobs_mm[i] - Q[i], 2);
			// cout << Q_diff1 << endl;
			// 计算Nash-Sutcliffe指数分母
			Q_diff2 = Q_diff2 + pow(Qobs_mm[i] - Q_ave, 2);
		}
		NSE = 1 - Q_diff1 / Q_diff2;
		// cout << Q_diff1 << " " << Q_diff2 << endl;
		// cout << "k = " << k << " NSE = " << NSE << endl;
		maxn = max(maxn, NSE);
		if(fabs(NSE - 0.843) < minn)
		{
			minn = fabs(NSE - 0.843);
			ansk = NSE;
			tmp = k;			
		}
	}
	cout << ansk << endl;
	cout << tmp *1.0 / 10000 * 100 << endl;
	cout << minn / 0.843 * 100 << endl;
	cout << maxn << endl;
	ofstream outfile;
	outfile.open("answer.csv", ios::out | ios::trunc);
	outfile << "date" << ',' << "ObserveValue" << ',' << "AnalogValue" << endl;
	for(int i = 1; i <= size; i ++)
		outfile << to_string(i) << ',' << to_string(Qobs_mm[i]) << ',' << to_string(Q[i]) << endl;
	outfile.close();
    return 0;
}

double SH1_CURVE(double t, double x4)
{
	// Function calculates time delay for HU1
	double sh;
	if(t <= 0.0)    sh = 0.0;
    else if(t < x4)    sh = pow((t / x4), 2.5);
    else if(t >= x4)    sh = 1.0;
	return sh;
}

double SH2_CURVE(double t, double x4)
{
	// Function calculates time delay for HU2
	double sh;
	if(t <= 0.0)   sh = 0.0;
    else if(t <= x4)   sh = 0.5 * pow((t / x4), 2.5);
    else if(t < 2 * x4)   sh = 1 - 0.5 * pow((2 - t / x4), 2.5);
    else if(t >= 2 * x4)   sh = 1.0;
	return sh;
}