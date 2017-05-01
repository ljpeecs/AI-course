//
//  main.cpp
//  Findmax
//
//  Created by ljp on 17/4/24.
//  Copyright © 2017年 LiuJiapeng. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ctime>
using namespace std;

const int scale = 300;              // 种群规模
const int len = 24;                 // 染色体长度
const int crossAmount = 12;         // 交叉时的交叉点个数
const int maxGen = 300;             // 最大进化代数
const double crossRate = 0.5;       // 交叉概率
const double mutationRate = 0.05;   // 变异概率

// 染色体类
class Chromosome {
public:
    bool gene[len];
    
    Chromosome() {
        for (int i = 0; i < len; i++)
            gene[i] = rand() % 2;
    }
    
    // 交叉操作
    void crossover(Chromosome &c) {
        // 按设置的交叉点个数随机选择数个交叉点
        int crossPoint[scale];
        for (int i = 0; i < crossAmount; i++)
            crossPoint[i] = rand() % len;
        
        // 将交叉点排序，方便后续操作
        sort(crossPoint, crossPoint + crossAmount);
        
        // 在选好的交叉点上交换等位基因
        for (int i = 0, j = 0; i < crossAmount; i++)
            while (i == crossPoint[j] && j < crossAmount) {
                swap(this->gene[i], c.gene[i]);
                j++;
            }
    }
    
    // 变异操作
    void mutate() {
        // 随机选择一个变异点
        int i = rand() % len;
        
        // 将变异点的基因取反
        gene[i] = !gene[i];
    }
};

Chromosome population[scale], pTemp[scale];   // 种群数组及其辅助数组
Chromosome bestCh;                            // 记录当前最优的染色体
double bestVal;                               // 记录当前最优的目标函数值

// 计算目标函数值
double f(double x) {
    return x + 10 * sin(5 * x) + 7 * cos(4 * x);
}

// 解码，将染色体的二进制编码映射到[0, 9]内的一个十进制实数
void decode(const Chromosome &c, double &x) {
    double divNum = pow(2, len);
    double sum = 0;
    double k = 1;
    for (int i = len - 1; i >= 0; i--) {
        sum += c.gene[i] * k;
        k *= 2;
    }
    x = 9 / divNum * sum;
}

// 目标函数加上一个正数使其在[0, 9]内大于0，作为适应度
double fitness(const Chromosome &c) {
    double x;
    decode(c, x);
    return f(x) + 17;
}

// 找出种群最优个体，并获取最优个体在[0, 9]内的值及对应的目标函数值
int getBest(Chromosome pop[scale], double &x, double &val) {
    double pFitness[scale];
    for (int i = 0; i < scale; i++)
        pFitness[i] = fitness(pop[i]);
    
    // 找出适应度最大的个体
    int max = 0;
    for (int i = 1; i < scale; i++)
        if (pFitness[i] > pFitness[max])
            max = i;
    
    // 获取该染色体在[0, 9]内的值及对应的目标函数值
    decode(pop[max], x);
    val = f(x);
    
    return max;
}

// 选择操作
void select(Chromosome pop[scale]){
    double pFitness[scale];
    for (int i = 0; i < scale; i++)
        pFitness[i] = fitness(pop[i]);
    
    double sum = 0;
    for (int i = 0; i < scale; i++)
        sum += pFitness[i];
    
    // 按适应度为个体分配选择区间
    double prob[scale];
    for (int i = 0; i < scale; i++)
        prob[i] = pFitness[i] / sum;
    for (int i = 1; i < scale; i++)
        prob[i] += prob[i - 1];
    
    // 使用轮盘赌的方法进行选择
    int selected[scale];
    double x, val;
    selected[0] = getBest(pop, x, val); // 精英选择
    for (int i = 1; i < scale; i++) {
        double randNum = rand() % 10001 / 10000.0;
        for (int j = 0; j < scale; j++)
            if (randNum <= prob[j]) {
                selected[i] = j;
                break;
            }
    }
    
    for (int i = 0; i < scale; i++)
        pTemp[i] = pop[i];
    for (int i = 0; i < scale; i++)
        pop[i] = pTemp[selected[i]];
}

// 完整的遗传算法
void GA(double &x, double &val) {
    // 初始化种群
    for (int i = 0; i < scale; i++)
        population[i] = Chromosome();
    
    // 初始化最优染色体
    bestCh = population[getBest(population, x, bestVal)];
    
    for (int gen = 1; gen <= maxGen; gen++) {
        // 选择操作
        select(population);
        
        // 交叉操作
        for (int i = 0; i < scale; i++) {
            double randNum = rand() % 10001 / 10000.0;
            if (randNum <= crossRate) {
                int randCh = rand() % scale;
                population[i].crossover(population[randCh]);
            }
        }
        
        // 变异操作
        for (int i = 0; i < scale; i++) {
            double randNum = rand() % 10001 / 10000.0;
            if (randNum < mutationRate)
                population[i].mutate();
        }
        
        // 获得这一代最优的目标函数值
        double bestNow;
        int best = getBest(population, x, bestNow);
        
        // 获得这一代平均的目标函数值
        double averageNow = 0, p;
        for (int i = 0; i < scale; i++) {
            decode(population[i], p);
            averageNow += f(p);
        }
        averageNow /= scale;
        
        cout << setw(3) << gen << " x=" << fixed << setprecision(6) << x
        << " f(x)=" << bestNow << "  average f(x)=" << averageNow << endl;
        
        // 防止种群退化
        if (bestNow < bestVal)
            population[best] = bestCh;
        else {
            bestCh = population[best];
            bestVal = bestNow;
        }
        
    }
    getBest(population, x, val);
}

int main(int argc, const char * argv[]) {
    srand(time(0));
    double x, maxVal;
    GA(x, maxVal);
    cout << "当x=" << fixed << setprecision(6) << x << "时，函数取得近似的最大值" << maxVal << endl;
    return 0;
}




