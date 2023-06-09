% 第一步：读取GR4J模型所需要得数据，为模型执行做准备
% 加载GR4J模型参数
para=load('GR4J_Parameter.txt');
x1=para(1,1);                       % x1: 产流水库容量 (mm)
x2=para(2,1);                       % x2: 地下水交换系数 (mm)
x3=para(3,1);                       % x3: 汇流水库容量 (mm)
x4=para(4,1);                       % x4: 单位线汇流时间 (天)

% 加载GR4J其他参数
other_para=load('others.txt');
area=other_para(1,1);               % 流域面积(km2)
upperTankRatio=other_para(2,1);     % 产流水库初始填充率 S0/x1
lowerTankRatio=other_para(3,1);     % 汇流水库初始填充率 R0/x3

% 加载数据文件
data=load('inputData.txt');
P = data(:,1);                      % 第二列:日降雨量(mm)
E = data(:,2);                      % 第三列:蒸散发量(mm)
Qobs=data(:,3);                     % 第四列:流域出口观测流量(m3/s)

Qobs_mm=Qobs*86.4/area;             % 将径流量单位从m3/s转换为mm/d，其中86.4为转化系数3600*24*1000/10^6

% 第二步：根据逐日降雨量及逐日蒸发量，计算流域出口断面逐日径流量
nStep=size(data,1);                 % 计算数据中有多少天

% 初始化变化，存储GR4J模型中间变量值
Pn=zeros(nStep,1);                  % Pn：降雨扣除损失（蒸发）后得净雨
En=zeros(nStep,1);                  % En：当日蒸发未被满足部分，此部分未得到满足得蒸发将消耗土壤中得水分
Ps=zeros(nStep,1);                  % Ps：中间变量，记录净雨补充土壤含水量
Es=zeros(nStep,1);                  % Es: 中间变量，记录剩余蒸发能力消耗土壤含水量
Perc=zeros(nStep,1);                % Perc: 中间变量，记录产流水库壤中流产流量
Pr=zeros(nStep,1);                  % Pr: 记录产流总量

%根据输入参数x4计算S曲线以及单位线，这里假设单位线长度UH1为10，UH2为20;即x4取值不应该大于10
maxDayDelay=10;
%定义几个数组以存储SH1,UH1,SH2,UH2
SH1=zeros(1,maxDayDelay); 
UH1=zeros(1,maxDayDelay); 
SH2=zeros(1,2*maxDayDelay);
UH2=zeros(1,2*maxDayDelay);
%计算SH1以及SH2
for i= 1:maxDayDelay
    SH1(i)=SH1_CURVE(i,x4);
end
for i= 1:2*maxDayDelay
    SH2(i)=SH2_CURVE(i,x4);
end
%计算UH1以及UH2
for i= 1:maxDayDelay
    if i==1
        UH1(i)=SH1(i);
    else
        UH1(i)=SH1(i)-SH1(i-1);
    end
end
for i= 1:2*maxDayDelay
    if i==1
        UH2(i)=SH2(i);
    else
        UH2(i)=SH2(i)-SH2(i-1);
    end
end
% 计算逐日En及Pn值,En及Pn为GR4J模型得输入，可以提前计算出来
for i = 1:nStep
   if(P(i)>=E(i))                   % 若当日降雨量大于等于当日蒸发量，净降雨量Pn=P-E,净蒸发能力En=0
      Pn(i)=P(i)-E(i);
      En(i)=0;
   else                             % 若当日降雨量小于当日蒸发量，净效降雨量Pn=0,净蒸发能力En=E-P
       Pn(i)=0;
       En(i)=E(i)-P(i);
   end
end

%定义一些产汇流计算需要用到得数值及数组
S0=upperTankRatio*x1;               % 产流水库初始土壤含水量=比例*产流水库容量
R0=lowerTankRatio*x3;               % 汇流水库初始土壤含水量=比例*汇流水库容量
S=zeros(nStep,1);                   % S：产流水库逐日水量
R=zeros(nStep,1);                   % R：汇流水库逐日水量
UH_Fast=zeros(nStep,maxDayDelay);   % UH_Fast: 用于记录UH1单位线作用下得产流信息
UH_Slow=zeros(nStep,maxDayDelay*2); % UH_Slow: 用于记录UH2单位线作用下得产流信息
S_TEMP=S0;                          % 用S_TEMP存储当前产流水库储量
R_TEMP=R0;                          % 用R_TEMP存储当前汇流水库储量
Qr=zeros(nStep,1);                  % Qr：汇流水库快速流出流量
Qd=zeros(nStep,1);                  % Qd：汇流水库慢速流出流量
Q=zeros(nStep,1);                   % Q：汇流总出流量

% 获取En及Pn值后，更新产流水库蓄水量
for i = 1:nStep
    S(i)=S_TEMP;
    R(i)=R_TEMP;
%Pn(i)及En(i)有且仅有一个大于0，所以下面两个if判断只可能一个为真
    if Pn(i) ~= 0                   %净雨量大于0，此时一部分净雨形成地面径流，一部分下渗
      Ps(i)=x1*(1-((S(i)/x1)^2))*tanh(Pn(i)/x1)/(1+S(i)/x1*tanh(Pn(i)/x1));    
      Es(i)=0;
    end
    if En(i) ~= 0                   %净蒸发能力大于0，此时土壤中水分一部分用于消耗
      Ps(i)=0;
      Es(i)=(S(i)*(2-(S(i)/x1))*tanh(En(i)/x1))/(1+(1-S(i)/x1)*tanh(En(i)/x1));      
    end
%更新上层水库蓄水量, 此处仅仅用于同学们理解，便于与Excel表格对照
    S_TEMP=S(i)-Es(i)+Ps(i);
    Ratio_TEMP=S_TEMP/x1;
%计算产流水库渗漏，这部分可以视作壤中流
    Perc(i)=S_TEMP*(1-(1+(4.0/9.0*(S_TEMP/x1))^4)^(-0.25));
%计算产流总量,其中Pn-Ps可以视作为地表产流
    Pr(i)=Perc(i)+(Pn(i)-Ps(i));
%更新当前产流水库水量，作为次日产流水库水量
    S_TEMP=S_TEMP-Perc(i);
%%%% 至此产流过程全部结束，进入汇流阶段

%%%% 汇流计算
%计算地表水与地下水之间得水量交换
   F=x2*(R(i)/x3)^3.5;

%计算地表水汇流，将产流量按照90%(快速)和10%(慢速)划分
%快速地表径流汇流使用单位先UH1; 慢速地表径流汇流使用单位先UH2; 
   R_Fast=Pr(i)*0.9;
   R_Slow=Pr(i)*0.1;
   if i==1
       UH_Fast(i,:)=R_Fast*UH1;                         %第1时段产流量在时间上的分配
       UH_Slow(i,:)=R_Slow*UH2;                         %第1时段产流量在时间上的分配
   else
       UH_Fast(i,:)=R_Fast*UH1;                         %先计算当前时段产流量在时间上得分配
       for j=1:maxDayDelay-1
          UH_Fast(i,j)=UH_Fast(i,j)+UH_Fast(i-1,j+1);   %第2时段总汇流=第2时段产流量当前汇流+第1时段产流量第2部分汇流
       end                                              %以此类推

       UH_Slow(i,:)=R_Slow*UH2;                         %先计算当前时段产流量在时间上得分配
       for j=1:2*maxDayDelay-1
           UH_Slow(i,j)=UH_Slow(i,j)+UH_Slow(i-1,j+1);  %第2时段总汇流=第2时段产流量当前汇流+第1时段产流量第2部分汇流
       end                                              %以此类推
   end

   %更新汇流水库水量变化; 
   R_TEMP=max(0,R_TEMP+UH_Fast(i,1)+F);
   %计算汇流水库快速流出流量
   Qr(i)=R_TEMP*(1-(1+(R_TEMP/x3)^4)^(-0.25));
   %再次更新汇流水库水量变化; 
   R_TEMP=R_TEMP-Qr(i);

   %计算汇流水库慢速流出流量
   Qd(i)=max(0,UH_Slow(i,1)+F);

   %计算汇流总出流量
   Q(i)=Qr(i)+Qd(i);
end

%精度评估
% 1. 通过调整模型参数，使得模型得到更好的拟合效果的加分，具体视Nash-Sutcliffe指数
% 2. 假设第一年365天作为预热期，消除产流水库和汇流水库初始值选取对模拟精度的影响
count=0;                    % 计数器：记录总天数
Q_accum=0.0;                % 记录累计径流量
Q_ave=0.0;                  % 记录平均径流量
NSE=0.0;                    % 记录纳什效率系数
Q_diff1=0.0;
Q_diff2=0.0;
for i = 366:nStep
    count=count+1;
    Q_accum=Q_accum+Qobs_mm(i);
end

tmp=zeros(nStep,1);

Q_ave=Q_accum/count;              %计算观测流量平均值
for i = 366:nStep
    Q_diff1=Q_diff1+(Qobs_mm(i)-Q(i))^2;      %计算Nash-Sutcliffe指数分子
    tmp(i - 366 + 1) = Q_diff1;
    Q_diff2=Q_diff2+(Qobs_mm(i)-Q_ave)^2;     %计算Nash-Sutcliffe指数分母

end
NSE=1-Q_diff1/Q_diff2;

% 评估径流模拟效果：模型流域出口断面流量及模拟得到的流域出口断面流量
% 1. 绘制相关图
axis = 1:1:nStep;
h1=figure;
plot(axis,Q,'r--',axis,Qobs_mm,'k-');
title('GR4J模型模拟效果图, NSE=',num2str(NSE));
xlabel('时间（天）');
ylabel('流量（mm/d）');
legend('模拟径流量','观测径流量');
clc;