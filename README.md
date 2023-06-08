# 水文模型GR4J

该存储库用于存储水文模型DR4J的实现。

## Branch Description

GR4J相关材料分支用于存放模型的相关信息

代码分支用于存放C++实现

参考代码分支用于存放MATLAB实现

流域站点信息用于存放10个流域的数据

模型验证存放对于10个流域站点处理信息的模拟与最终结果的图片呈现

## 参数格式

### GR4J_Parameter.csv

存储GR4J模型参数

格式如下：

| 320.11 | x1: 产流水库容量 (mm)   |
| ------ | ----------------------- |
| 2.42   | x2: 地下水交换系数 (mm) |
| 69.63  | x3: 汇流水库容量 (mm)   |
| 1.39   | x4: 单位线汇流时间 (天) |

### others.csv

存储GR4J其他参数

格式如下：

| 260  | area: 流域面积(km2)                      |
| ---- | ---------------------------------------- |
| 0.6  | upperTankRatio: 产流水库初始填充率 S0/x1 |
| 0.7  | lowerTankRatio: 汇流水库初始填充率 R0/x3 |

### inputData.csv

存储模型数据

第一列存储降雨量（$mm$），第二列存储蒸发量（$mm$），第三列存储径流量（$m^3/s$）

| 14.1 | 0.46 | 8.45 |
| ---- | ---- | ---- |
| 3.7  | 0.46 | 14.9 |
| ...  | ...  | ...  |

## 样例测试

模拟纳什效率系数为0.919021

径流量图如下

![](https://github.com/XuShengXianggg/picturebed/blob/main/%E6%A0%B7%E4%BE%8B.png?raw=true)

## 模型验证

| 站点名称                                      | 站点ID  | 观测纳什效率系数 | 模拟纳什效率系数 | 误差率 |
| --------------------------------------------- | ------- | ---------------- | ---------------- | ------ |
| Rifle Creek at Fonthill                       | 919005A | 0.841            | 0.84085          | 0.018% |
| Mannus Creek at Yarramundi                    | 401017  | 0.842            | 0.843556         | 0.185% |
| Pirron YalloCreek C River at Pirron Yallock   | 234203  | 0.843            | 0.838799         | 0.498% |
| StHelens Creek at Calen                       | 124002A | 0.844            | 0.837351         | 0.788% |
| Gibbo River at Gibbo Park                     | 401217  | 0.846            | 0.845524         | 0.056% |
| Duck River at Scotchtown Road                 | 314214  | 0.847            | 0.847878         | 0.103% |
| Hellyer River at Guilford Junction            | 312061  | 0.851            | 0.847759         | 0.381% |
| Coomera River at Army Camp                    | 146010A | 0.853            | 0.852305         | 0.081% |
| Goulburn River at U/S of Snake Creek Junction | 405263  | 0.875            | 0.87383          | 0.134% |
| Reid Creek at Dam Site                        | 136006A | 0.875            | 0.874008         | 0.113% |

