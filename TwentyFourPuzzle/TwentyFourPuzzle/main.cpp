//
//  main.cpp
//  TwentyFourPuzzle
//
//  Created by ljp on 17/5/10.
//  Copyright © 2017年 LiuJiapeng. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#define SIZE 5
using namespace std;

int state1D[SIZE * SIZE], state2D[SIZE][SIZE], path[100], limit, flag, length;
int goal[25][2] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0},
    {1, 1}, {1, 2}, {1, 3}, {1, 4}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4},
    {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}}; // 各个数字应在位置对照表
int Move[4][2] = {{-1, 0}, {0, -1}, {0, 1}, {1, 0}}; // 上左右下四个方向坐标的变化量
char op[4] = {'U', 'L', 'R', 'D'};
double nodenum = 0;

int rowRight[SIZE] = {0, 0, 0, 0, 0};
int colRight[SIZE] = {0, 0, 0, 0, 0};
int rowRightOrder[SIZE][SIZE] = {{-1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1},};
int colRightOrder[SIZE][SIZE] = {{-1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1}, {-1, -1, -1, -1, -1},};
const int addition[6][11] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 2, 2, 4, 0, 0, 0, 0, 0, 0, 0}, {0, 2, 2, 2, 4, 4, 6, 0, 0, 0, 0},
    {0, 2, 2, 2, 2, 4, 4, 4, 6, 6, 8},};            // 由元素数和逆序数找曼哈顿距离应加的值

int reverse(int a[SIZE * SIZE]) {
    int rev = 0;
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        if (a[i] == 0)
            continue;
        for (int j = i + 1; j < SIZE * SIZE; j++)  // 每一个都跟其后所有的比一圈 查找小于i的个数相加
        {
            if (a[i] > a[j] && a[j] != 0)
                rev++;
        }
    }
    if (rev % 2 == 1)
        return 1;
    else
        return 0;
}

/* 估价函数，曼哈顿距离，小于等于实际最少步 */
int mdsum(int a[][SIZE]) {
    int sum = 0;
    int r = 0, c[5] = {0, 0, 0, 0, 0};          // 存放2个RightOrder数组最新元素的下标
    int col1 = -1, row5 = -1;                   // 记录1所在的列和5所在的行
    bool conflict1 = false, conflict5 = false;  // 记录1和5是否有逆序冲突
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (a[i][j] != 0) {
                int t = a[i][j];
                sum += abs(i - goal[t][0]) + abs(j - goal[t][1]);
                if (i == goal[t][0]) {          // 若该数在目标行
                    rowRight[i]++;
                    rowRightOrder[i][r] = t;
                    r++;
                }
                if (j == goal[t][1]) {          // 若该数在目标列
                    colRight[j]++;
                    colRightOrder[j][c[j]] = t;
                    c[j]++;
                }
                if (t == 1)
                    col1 = j;
                else if (t == 5)
                    row5 = i;
            }
        }
        r = 0;
    }
    
    int rowRev = 0;
    for (int i = 0; i < SIZE; i++) {                // 对每一行
        if (rowRight[i] >= 2) {                     // 若至少2个数在目标行
            for (int j = 0; j < rowRight[i]; j++)   // 计算逆序数
                for (int k = j + 1; k < rowRight[i]; k++)
                    if (rowRightOrder[i][j] > rowRightOrder[i][k])
                        rowRev++;
            sum += addition[rowRight[i]][rowRev];
            rowRev = 0;
        }
    }
    
    int colRev = 0;
    for (int i = 0; i < SIZE; i++) {                // 对每一列
        if (colRight[i] >= 2) {                     // 若至少2个数在目标列
            for (int j = 0; j < colRight[i]; j++)   // 计算逆序数
                for (int k = j + 1; k < colRight[i]; k++)
                    if (colRightOrder[i][j] > colRightOrder[i][k])
                        colRev++;
            sum += addition[colRight[i]][colRev];
            colRev = 0;
        }
        
    }
    
    if (col1 == 1) {
        for (int i = 0; i < colRight[1]; i++) {     // 是否有逆序
            for (int j = i + 1; j < colRight[1]; j++)
                if (colRightOrder[1][i] > colRightOrder[1][j]) {
                    conflict1 = true;
                    break;
                }
            if (conflict1)
                break;
        }
    }
    if (row5 == 1) {
        for (int i = 0; i < rowRight[1]; i++) {     // 是否有逆序
            for (int j = i + 1; j < rowRight[1]; j++)
                if (rowRightOrder[1][i] > rowRightOrder[1][j]) {
                    conflict5 = true;
                    break;
                }
            if (conflict5)
                break;
        }
    }
    
    if (col1 != 0 && row5 != 0 && !conflict1 && !conflict5 && sum != 0)
        sum += 2;
    
    for (int i = 0; i < SIZE; i++)
        rowRight[i] = colRight[i] = 0;
    
    return sum;
}

void dfs(int bx, int by, int len, int pre_move) {
    int nx, ny;
    int md = mdsum(state2D);
    nodenum++;
    
    // 检查是否已达目标状态
    if (md == 0) {
        flag = 1;
        length = len;
        return;
    }
    
    if (md + len >= limit)
        return;
    
    for (int i = 0; i < 4; i++) {
        // 与上一次移动方向相反，则跳过
        if (i + pre_move == 3)
            continue;
        
        // 得到空格的新坐标
        nx = bx + Move[i][0];
        ny = by + Move[i][1];
        
        if (0 <= nx && nx < SIZE && 0 <= ny && ny < SIZE) { // 判断是否会移出界
            // 把空格移到新的位置
            swap(state2D[bx][by], state2D[nx][ny]);
            // 更新路径
            path[len] = i;
            // 扩展节点
            dfs(nx, ny, len + 1, i);
            // 达到了目标状态就返回
            if (flag)
                return;
            // 未达到目标状态则恢复移动前的状态
            swap(state2D[bx][by], state2D[nx][ny]);
        }
    }
}

int main(int argc, const char * argv[]) {
    int bx = -1, by = -1;
    double start, finish;
    flag = 0, length = 0;
    srand(time(0));
    memset(path, -1, sizeof(path));  // 已定义path[100]数组，将path填满-1
    
    for (int i = 0; i < SIZE * SIZE; i++)
        state1D[i] = i;
    
    // 由一维的状态得到二维的状态，同时获取空格位置
    for (int i = 0; i < SIZE * SIZE; i++) {
        state2D[i / SIZE][i % SIZE] = state1D[i];
        if (state1D[i] == 0) {
            bx = i / SIZE;
            by = i % SIZE;
        }
    }
    
    // 打乱n步
    int nx = -1, ny = -1, n = 150;
    for (int i = 1; i <= n; i++) {
        bool valid = false;
        while (!valid) {
            int r = rand() % 4;
            nx = bx + Move[r][0];
            ny = by + Move[r][1];
            if (0 <= nx && nx < SIZE && 0 <= ny && ny < SIZE)
                valid = true;
        }
        swap(state2D[bx][by], state2D[nx][ny]);
        bx = nx;
        by = ny;
    }
    
    if (bx == -1 || by == -1) {
        cout << "Can't find blank. Exit.\n";
        return 0;
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++)
            cout << setw(2) << state2D[i][j] << " ";
        cout << endl;
    }
    
    start = clock();
    if (reverse(state1D) == 0) {
        limit = mdsum(state2D);     // 全部的曼哈顿距离之和
        while (true) {
            dfs(bx, by, 0, -1);
            if (flag)
                break;
            limit++;
        }
        for(int i = 0; i < length; i++)
            cout << op[path[i]];    // 根据path输出URLD路径
        cout << "\n";
    } else
        cout << "This puzzle is not solvable.\n";
    finish = clock();
    cout << "Time used = " << (finish - start) / CLOCKS_PER_SEC << endl;
    cout << "Nodes generated = " << nodenum << endl;
    return 0;
}
