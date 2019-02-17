#include <iostream>
#include <conio.h>
#include <atlstr.h>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include "Point.h"
#include "Strategy.h"
#include "Judge.h"

using namespace std;

static int lastx;
static int lasty;
static int next_step[2]; //存储下一步的走步
static int *mytop;  //存储每一次拓展后的top
static int **myboard;  // 存储每次拓展后的board
static int DEPTH = 8; //设置剪枝深度


int evaluate_board(bool type, const int M, const int N); //局面评价函数
bool hasThreatPoint(bool type, int x, int y, const int M, const int N);  //判断一个点是否是威胁点
bool userWin(const int x, const int y, const int M, const int N, int* const* board);  //用户胜
bool machineWin(const int x, const int y, const int M, const int N, int* const* board);  //策略胜
bool isTie(const int N, const int* top);  //平局


int negmaxsearch(bool mytype,int mydepth, int myalpha, int mybeta, const int M, const int N) {
	int score=0;
	//判断局面；
	if (machineWin(lastx, lasty, M, N, myboard) || userWin(lastx, lasty, M, N, myboard) || mydepth == 0|| isTie(N,mytop)) {
		return evaluate_board(mytype,M, N);
	}
	//剪枝排序！越靠近lasty,优先拓展，可以极大提升效率。深度可再加深2层！
	int* sortindex = new int[N];
	int count = 0;
	for (int delta = 0; delta < N; delta++) {
		if (count == N) break;
		for (int j = 0; j < N; j++) {
			if (abs(j - lasty) == delta) {
				sortindex[count] = j;
				count++;
				if (count == N) break;
			}
		}
	}
	
	//节点拓展
	for (int i = 0; i < N; i++) {
		if (mytop[sortindex[i]] == 0) continue; //满的列不拓展
		//备份拓展前的点
		int backx = lastx;
		int backy = lasty;
		int *backtop = new int [N];
		for (int j = 0; j < N; j++) {
			backtop[j] = mytop[j];
		}
		int **backboard = new int*[M];
		for (int p = 0; p < M; p++) {
			backboard[p] = new int[N];
			for (int q = 0; q < N;q++) {
				backboard[p][q] = myboard[p][q];
			}
		}
		//更新mytop和myboard,lastx,lasty		
		lastx = mytop[sortindex[i]] - 1;
		lasty = sortindex[i];
		if (mytype) myboard[mytop[sortindex[i]] - 1][sortindex[i]] = 2;
		else myboard[mytop[sortindex[i]] - 1][sortindex[i]] = 1;
		if (mytop[sortindex[i]] == 1) {
			mytop[sortindex[i]] = 0;
		}
		else if (myboard[mytop[sortindex[i]]-2][sortindex[i]] == -1) mytop[sortindex[i]] -= 2; //不可落子点，更新mytop
		else mytop[sortindex[i]] -=1; //没有不可落子点，更新mytop

		//负极大值算法的递归
		score = -negmaxsearch(!mytype,mydepth-1,-mybeta,-myalpha,M,N); 
		
		//还原拓展前的点
		lastx = backx;
		lasty = backy;
		for (int j = 0; j < N; j++) {
			mytop[j] = backtop[j];
		}
		for (int p = 0; p < M; p++) {
			for (int q = 0; q < N; q++) {
				myboard[p][q] = backboard[p][q];
			}
		}
		delete[]backtop;
		for (int j = 0; j < M; j++) {
			delete[]backboard[j];
		}
		delete[]backboard;
		
		//alpha-beta剪枝
		if (score > myalpha) {
			if (mydepth == DEPTH) {
					next_step[0] = mytop[sortindex[i]] - 1;
					next_step[1] = sortindex[i];
			}
			if (score >= mybeta) {
				return mybeta;
			}
			myalpha = score;
		}
	}
	delete[]sortindex;
	return myalpha;
}


/*
	策略函数接口,该函数被对抗平台调用,每次传入当前状态,要求输出你的落子点,该落子点必须是一个符合游戏规则的落子点,不然对抗平台会直接认为你的程序有误
	
	input:
		为了防止对对抗平台维护的数据造成更改，所有传入的参数均为const属性
		M, N : 棋盘大小 M - 行数 N - 列数 均从0开始计， 左上角为坐标原点，行用x标记，列用y标记
		top : 当前棋盘每一列列顶的实际位置. e.g. 第i列为空,则_top[i] == M, 第i列已满,则_top[i] == 0//已占领的实际位置
		_board : 棋盘的一维数组表示, 为了方便使用，在该函数刚开始处，我们已经将其转化为了二维数组board
				你只需直接使用board即可，左上角为坐标原点，数组从[0][0]开始计(不是[1][1])
				board[x][y]表示第x行、第y列的点(从0开始计)
				board[x][y] == 0/1/2 分别对应(x,y)处 无落子/有用户的子/有程序的子,不可落子点处的值也为0。！！！写的程序自己就是2
		lastX, lastY : 对方上一次落子的位置, 你可能不需要该参数，也可能需要的不仅仅是对方一步的
				落子位置，这时你可以在自己的程序中记录对方连续多步的落子位置，这完全取决于你自己的策略
		noX, noY : 棋盘上的不可落子点(注:其实这里给出的top已经替你处理了不可落子点，也就是说如果某一步
				所落的子的上面恰是不可落子点，那么UI工程中的代码就已经将该列的top值又进行了一次减一操作，
				所以在你的代码中也可以根本不使用noX和noY这两个参数，完全认为top数组就是当前每列的顶部即可,
				当然如果你想使用lastX,lastY参数，有可能就要同时考虑noX和noY了)
		以上参数实际上包含了当前状态(M N _top _board)以及历史信息(lastX lastY),你要做的就是在这些信息下给出尽可能明智的落子点
	output:
		你的落子点Point
*/
extern "C" __declspec(dllexport) Point* getPoint(const int M, const int N, const int* top, const int* _board, 
	const int lastX, const int lastY, const int noX, const int noY){
	/*
		不要更改这段代码
	*/
	int x = -1, y = -1;//最终将你的落子点存到x,y中
	int** board = new int*[M];
	for(int i = 0; i < M; i++){
		board[i] = new int[N];
		for(int j = 0; j < N; j++){
			board[i][j] = _board[i * N + j];
		}
	}
	
	/*
		根据你自己的策略来返回落子点,也就是根据你的策略完成对x,y的赋值
		该部分对参数使用没有限制，为了方便实现，你可以定义自己新的类、.h文件、.cpp文件
	*/
	//Add your own code below
	/*
     //a naive example
	for (int i = N-1; i >= 0; i--) {
		if (top[i] > 0) {
			x = top[i] - 1;
			y = i;
			break;
		}
	}
    */
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//我的代码：         //
	board[noX][noY] = -1; //把不可落子点与无落子点区别出来
    //初始化myboard
	myboard = new int *[M];
	for (int i = 0; i < M; i++) {
		myboard[i] = new int[N];
		for (int j = 0; j < N; j++) {
			myboard[i][j] = board[i][j];
		}
	}
	//_cprintf("%d\t ", myboard[noX][noY]);
	//初始化mytop
	mytop = new int [N];
	for (int i = 0; i < N; i++) {
		mytop[i] = top[i];
	}
	//初始化lastx，lasty
	lastx = lastX;
	lasty = lastY;
	//_cprintf("%d\t%d\t ", lastx,lasty);
	//处理必杀棋和威胁棋
	int xindex, yindex;
	bool isabcut = true; //标记，用于判断用不用ab剪枝
	for (int i = 0; i < N; i++) {
		if (mytop[i] == 0) continue;
		else {
				xindex = mytop[i] - 1;
				yindex = i;
				if (hasThreatPoint(true, xindex, yindex, M, N)) { //判断必杀棋
					next_step[0] = mytop[i] - 1;
					next_step[1] = i;
					isabcut = false;					
				}
				else {
					if (hasThreatPoint(false, xindex, yindex, M, N)) { //判断威胁棋
						next_step[0] = mytop[i] - 1;
						next_step[1] = i;
						isabcut = false;
					}
				}
			}
		}

	//在策略先手时，把第一颗子放中间远离不可落子点的位置
	if (!(lastX>=0 && lastX<M && lastY >=0 && lastY< N)) {
		if ((N - 1 - noY) > noY) {
			next_step[0] = mytop[N-4] - 1;
			next_step[1] = N-4;
			isabcut = false;
		}
		else {
			next_step[0] = mytop[3] - 1;
			next_step[1] = 3;
			isabcut = false;
		}
		//_cprintf("lastX = %d\nlastY = %d\n", lastX, lastY);
	}
	if (isabcut) {
		int score = negmaxsearch(true, DEPTH, -999999999, 999999999, M, N);
	}
	//_cprintf("%d\t ", score);
	//把得到的下一步走法传给x,y
	x = next_step[0];
	y = next_step[1];
	delete[]mytop;
	for (int i = 0; i < M; i++) {
		delete[] myboard[i];
	}
	delete[]myboard;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		不要更改这段代码
	*/
	clearArray(M, N, board);
	return new Point(x, y);
}


/*
	getPoint函数返回的Point指针是在本dll模块中声明的，为避免产生堆错误，应在外部调用本dll中的
	函数来释放空间，而不应该在外部直接delete
*/
extern "C" __declspec(dllexport) void clearPoint(Point* p){
	delete p;
	return;
}

/*
	清除top和board数组
*/
void clearArray(int M, int N, int** board){
	for(int i = 0; i < M; i++){
		delete[] board[i];
	}
	delete[] board;
}


/*
	添加你自己的辅助函数，你可以声明自己的类、函数，添加新的.h .cpp文件来辅助实现你的想法
*/
// 局盘评估函数
//////////////////////////////////////////////////////////////////////////////////////
//我的代码：      //
////////////////////
//2子点的评价函数
int evaluate_each_point2(int x, int y, const int M, const int N) {

	int score2 = 0;//2子总分

	//考虑纵向计分	
	//计算2子
	//计算连子
	int bottomIndex2 = x + 1; //最底端是2的位置
	while (bottomIndex2 < M) {
		if (myboard[bottomIndex2][y] != 2) {
			break;
		}
		bottomIndex2++;
	}
	int topIndex2 = x - 1; //最底端是2的位置
	int vertcount2 = bottomIndex2 - topIndex2 - 1;//计算2子连子数
	//计算是否是活子
	while (topIndex2 >= 0) {
		if (myboard[topIndex2][y] != 0 && myboard[topIndex2][y] != 2) {
			break;
		}
		if (x - topIndex2 > 4) break;
		topIndex2--;
	}
	int vertwidth2 = bottomIndex2 - topIndex2 - 1; //最大走子范围，再判断是不是活字
	switch (vertcount2) { //8000,4000,2000;2000,1000,500
	case 1:
		if (vertwidth2 > 3) score2 += 1000;
		else score2 += 500;
		break;
	case 2:
		if (vertwidth2 > 3) score2 += 2000;
		else score2 += 1000;
		break;
	case 3:
		if (vertwidth2 > 3) score2 += 4000;
		else score2 += 2000;
		break;
	default:
		return 99999999;
	}
	//考虑横向
	//计算2子
	int leftIndex2 = y - 1; //最左端超出2的位置
	while (leftIndex2 >= 0) {
		if (myboard[x][leftIndex2] != 2) {
			break;
		}
		leftIndex2--;
	}
	int rightIndex2 = y + 1; //最右端超出2的位置
	while (rightIndex2 < N) {
		if (myboard[x][rightIndex2] != 2) {
			break;
		}
		rightIndex2++;
	}
	int horicount2 = rightIndex2 - leftIndex2 - 1; //先算出2的连子数

	int leftIndex22 = leftIndex2;
	while (leftIndex22 >= 0) {
		if (myboard[x][leftIndex22] != 0 && myboard[x][leftIndex22] != 2) {
			break;
		}
		if (y - leftIndex22 > 4) break;
		leftIndex22--;
	}
	int rightIndex22 = rightIndex2;
	while (rightIndex22 < N) {
		if (myboard[x][rightIndex22] != 0 && myboard[x][rightIndex22] != 2) {
			break;
		}
		if (rightIndex22 - y> 4) break;
		rightIndex22++;
	}
	int horiwidth2 = rightIndex22 - leftIndex22 - 1; //最大走子范围，再判断是不是活字
	switch (horicount2) {
	case 1:
		if (horiwidth2 > 3) score2 += 2000;
		else score2 += 500;
		break;
	case 2:
		if (mytop[rightIndex2] == x + 1 && mytop[leftIndex2] == x + 1) { //!!判断杀棋走法
			score2 += 9999;
			break;
		}
		if (horiwidth2 > 3) score2 += 5000;
		else score2 += 1000;
		break;
	case 3:
		if (mytop[rightIndex2]==x+1 && mytop[leftIndex2] == x+1) {  //!!判断杀棋走法
			score2 += 999999;
			break;
		}
		if (horiwidth2 > 3) score2 += 10000;
		else score2 += 2000;
		break;
	default:
		return  99999999;
	}


	//考虑正斜线
	//计算2
	int leftDelta2 = 1; //向左端拓展的个数
	while (y - leftDelta2 >= 0 && x - leftDelta2 >= 0) {
		if (myboard[x - leftDelta2][y - leftDelta2] != 2) {
			break;
		}
		leftDelta2++;
	}
	int rightDelta2 = 1; //向右端扩展的个数
	while (y + rightDelta2 < N && x + rightDelta2 < M) {
		if (myboard[x + rightDelta2][y + rightDelta2] != 2) {
			break;
		}
		rightDelta2++;
	}
	int leftupcount2 = rightDelta2 + leftDelta2 - 1; //先算出2的连子数

	int leftDelta22 = leftDelta2;
	while (y - leftDelta22 >= 0 && x - leftDelta22 >= 0) {
		if (myboard[x - leftDelta22][y - leftDelta22] != 0 && myboard[x - leftDelta22][y - leftDelta22] != 2) {
			break;
		}
		if (leftDelta22> 4) break;
		leftDelta22++;
	}
	int rightDelta22 = rightDelta2;
	while (y + rightDelta22 < N && x + rightDelta22 < M) {
		if (myboard[x + rightDelta22][y + rightDelta22] != 0 && myboard[x + rightDelta22][y + rightDelta22] != 2) {
			break;
		}
		if (rightDelta22> 4) break;
		rightDelta22++;
	}
	int leftupwidth2 = rightDelta22 + leftDelta22 - 1; //最大走子范围，再判断是不是活字
	switch (leftupcount2) {
	case 1:
		if (leftupwidth2 > 3) score2 += 2000;
		else score2 += 500;
		break;
	case 2:
		if (mytop[y - leftDelta2] == x - leftDelta2 + 1 && mytop[y + rightDelta2] == x + rightDelta2 + 1) {  //!!判断杀棋走法
			score2 += 9999;
			break;
		}
		if (leftupwidth2 > 3) score2 += 5000;
		else score2 += 1000;
		break;
	case 3:
		if (mytop[y - leftDelta2]== x - leftDelta2+1 && mytop[y + rightDelta2] == x + rightDelta2+1) {  //!!判断杀棋走法
			score2 += 999999;
			break;
		}
		if (leftupwidth2 > 3) score2 += 10000;
		else score2 += 2000;
		break;
	default:
		return 99999999;
	}

	//考虑反斜线
	//计算2
	leftDelta2 = 1; //向左端拓展的个数
	while (y - leftDelta2 >= 0 && x + leftDelta2 < M) {
		if (myboard[x + leftDelta2][y - leftDelta2] != 2) {
			break;
		}
		leftDelta2++;
	}
	rightDelta2 = 1; //向右端扩展的个数
	while (y + rightDelta2 < N && x - rightDelta2 >= 0) {
		if (myboard[x - rightDelta2][y + rightDelta2] != 2) {
			break;
		}
		rightDelta2++;
	}
	int leftdowncount2 = rightDelta2 + leftDelta2 - 1; //先算出2的连子数

	leftDelta22 = leftDelta2;
	while (y - leftDelta22 >= 0 && x + leftDelta22 < M) {
		if (myboard[x + leftDelta22][y - leftDelta22] != 0 && myboard[x + leftDelta22][y - leftDelta22] != 2) {
			break;
		}
		if (leftDelta22> 4) break;
		leftDelta22++;
	}
	rightDelta22 = rightDelta2;
	while (y + rightDelta22 < N && x - rightDelta22 >= 0) {
		if (myboard[x - rightDelta22][y + rightDelta22] != 0 && myboard[x - rightDelta22][y + rightDelta22] != 2) {
			break;
		}
		if (rightDelta22> 4) break;
		rightDelta22++;
	}
	int leftdownwidth2 = rightDelta22 + leftDelta22 - 1; //最大走子范围，再判断是不是活字
	switch (leftdowncount2) {
	case 1:
		if (leftdownwidth2 > 3) score2 += 2000;
		else score2 += 500;
		break;
	case 2:
		if (mytop[y - leftDelta2] == x + leftDelta2 + 1 && mytop[y + rightDelta2] == x - rightDelta2 + 1) {  //!!判断杀棋走法
			score2 += 9999;
			break;
		}
		if (leftdownwidth2 > 3) score2 += 5000;
		else score2 += 1000;
		break;
	case 3:
		if ((mytop[y - leftDelta2] == x + leftDelta2 +1) && (mytop[y + rightDelta2] == x - rightDelta2 +1)) {  //!!判断杀棋走法
			score2 += 999999;
			break;
		}
		if (leftdownwidth2 > 3) score2 += 10000;
		else score2 += 2000;
		break;	
	default:
		return 99999999;
	}

	return score2;
}

int evaluate_each_point1(int x, int y, const int M, const int N) {

	int score1 = 0;//1子总分
	//考虑纵向计分	
	//计算1
	int bottomIndex1 = x + 1; //最底端是1的位置
	while (bottomIndex1 < M) {
		if (myboard[bottomIndex1][y] != 1) {
			break;
		}
		bottomIndex1++;
	}
	int topIndex1 = x - 1; //最底端是1的位置
	
	int vertcount1 = bottomIndex1 - topIndex1 - 1;//计算1子连子数

	while (topIndex1 >= 0) {
		if (myboard[topIndex1][y] != 0 && myboard[topIndex1][y] != 1) {
			break;
		}
		if (x - topIndex1 > 4) break;
		topIndex1--;
	}
	int vertwidth1 = bottomIndex1 - topIndex1 - 1; //最大走子范围，再判断是不是活字
	switch (vertcount1) {
	case 1:
		if (vertwidth1 > 3) score1 += 1000;
		else score1 += 500;
		break;
	case 2:
		if (vertwidth1 > 3) score1 += 2000;
		else score1 += 1000;
		break;
	case 3:
		if (vertwidth1 > 3) score1 += 4000;
		else score1 += 2000;
		break;
	default:
		return 99999999;
	}
	//_cprintf("(%d,%d) = %d\n", x, y, bottomIndex-x+1);


	//考虑横向
	//计算1子
	int leftIndex1 = y - 1; //最左端超出1的位置
	while (leftIndex1 >= 0) {
		if (myboard[x][leftIndex1] != 1) {
			break;
		}
		leftIndex1--;
	}
	int rightIndex1 = y + 1; //最右端超出1的位置
	while (rightIndex1 < N) {
		if (myboard[x][rightIndex1] != 1) {
			break;
		}
		rightIndex1++;
	}
	int horicount1 = rightIndex1 - leftIndex1 - 1; //先算出1的连子数

	int leftIndex11 = leftIndex1;
	while (leftIndex11 >= 0) {
		if (myboard[x][leftIndex11] != 0 && myboard[x][leftIndex11] != 1) {
			break;
		}
		if (y - leftIndex11 > 4) break;
		leftIndex11--;
	}
	int rightIndex11 = rightIndex1;
	while (rightIndex11 < N) {
		if (myboard[x][rightIndex11] != 0 && myboard[x][rightIndex11] != 1) {
			break;
		}
		if (rightIndex11 - y> 4) break;
		rightIndex11++;
	}
	int horiwidth1 = rightIndex11 - leftIndex11 - 1; //最大走子范围，再判断是不是活字
	switch (horicount1) {
	case 1:
		if (horiwidth1 > 3) score1 += 2000;
		else score1 += 500;
		break;
	case 2:
		if (mytop[leftIndex1] == x + 1 && mytop[rightIndex1] == x + 1) {
			score1 += 9999;
			break;
		}
		if (horiwidth1 > 3) score1 += 5000;
		else score1 += 1000;
		break;
	case 3:
		if (mytop[leftIndex1] == x+1 && mytop[rightIndex1] == x+1) {
			score1 += 999999;
			break;
		}
		if (horiwidth1 > 3) score1 += 10000;
		else score1 += 2000;
		break;
	default:
		return  99999999;
	}


	//考虑正斜线
	//计算1
	int leftDelta1 = 1; //向左端拓展的个数
	while (y - leftDelta1 >= 0 && x - leftDelta1 >= 0) {
		if (myboard[x - leftDelta1][y - leftDelta1] != 1) {
			break;
		}
		leftDelta1++;
	}
	int rightDelta1 = 1; //向右端扩展的个数
	while (y + rightDelta1 < N && x + rightDelta1 < M) {
		if (myboard[x + rightDelta1][y + rightDelta1] != 1) {
			break;
		}
		rightDelta1++;
	}
	int leftupcount1 = rightDelta1 + leftDelta1 - 1; //算出1的连子数

	int leftDelta11 = leftDelta1;
	while (y - leftDelta11 >= 0 && x - leftDelta11 >= 0) {
		if (myboard[x - leftDelta11][y - leftDelta11] != 0 && myboard[x - leftDelta11][y - leftDelta11] != 1) {
			break;
		}
		if (leftDelta11> 4) break;
		leftDelta11++;
	}
	int rightDelta11 = rightDelta1;
	while (y + rightDelta11 < N && x + rightDelta11 < M) {
		if (myboard[x + rightDelta11][y + rightDelta11] != 0 && myboard[x + rightDelta11][y + rightDelta11] != 1) {
			break;
		}
		if (rightDelta11> 4) break;
		rightDelta11++;
	}
	int leftupwidth1 = rightDelta11 + leftDelta11 - 1; //最大走子范围，再判断是不是活字
	switch (leftupcount1) {
	case 1:
		if (leftupwidth1 > 3) score1 += 2000;
		else score1 += 500;
		break;
	case 2:
		if (mytop[y - leftDelta1] == x - leftDelta1 + 1 && mytop[y + rightDelta1] == x + rightDelta1 + 1) {
			score1 += 9999;
			break;
		}
		if (leftupwidth1 > 3) score1 += 5000;
		else score1 += 1000;
		break;
	case 3:
		if (mytop[y - leftDelta1] == x - leftDelta1+1 && mytop[y + rightDelta1] == x + rightDelta1+1) {
			score1 += 999999;
			break;
		}
		if (leftupwidth1 > 3) score1 += 10000;
		else score1 += 2000;
		break;	
	default:
		return  99999999;
	}

	//考虑反斜线
	//计算1
	leftDelta1 = 1; //向左端拓展的个数
	while (y - leftDelta1 >= 0 && x + leftDelta1 < M) {
		if (myboard[x + leftDelta1][y - leftDelta1] != 1) {
			break;
		}
		leftDelta1++;
	}
	rightDelta1 = 1; //向右端扩展的个数
	while (y + rightDelta1 < N && x - rightDelta1 >= 0) {
		if (myboard[x - rightDelta1][y + rightDelta1] != 1) {
			break;
		}
		rightDelta1++;
	}
	int leftdowncount1 = rightDelta1 + leftDelta1 - 1; //先算出2的连子数

	leftDelta11 = leftDelta1;
	while (y - leftDelta11 >= 0 && x + leftDelta11 < M) {
		if (myboard[x + leftDelta11][y - leftDelta11] != 0 && myboard[x + leftDelta11][y - leftDelta11] != 1) {
			break;
		}
		if (leftDelta11> 4) break;
		leftDelta11++;
	}
	rightDelta11 = rightDelta1;
	while (y + rightDelta11 < N && x - rightDelta11 >= 0) {
		if (myboard[x - rightDelta11][y + rightDelta11] != 0 && myboard[x - rightDelta11][y + rightDelta11] != 1) {
			break;
		}
		if (rightDelta11> 4) break;
		rightDelta11++;
	}
	int leftdownwidth1 = rightDelta11 + leftDelta11 - 1; //最大走子范围，再判断是不是活字
	switch (leftdowncount1) {
	case 1:
		if (leftdownwidth1 > 3) score1 += 2000;
		else score1 += 500;
		break;
	case 2:
		if (mytop[y - leftDelta1] == x + leftDelta1 + 1 && mytop[y + rightDelta1] == x - rightDelta1 + 1) {
			score1 += 9999;
			break;
		}
		if (leftdownwidth1 > 3) score1 += 5000;
		else score1 += 1000;
		break;
	case 3:
		if (mytop[y - leftDelta1] == x + leftDelta1+1 && mytop[y + rightDelta1] == x - rightDelta1+1) {
			score1 += 999999;
			break;
		}
		if (leftdownwidth1 > 3) score1 += 10000;
		else score1 += 2000;
		break;
	default:
		return 99999999;
	}

	return score1;
}

//type=true:查看该位置是否必胜棋;type=false:查看威胁棋
bool hasThreatPoint(bool type, int x, int y, const int M, const int N){
	int value;
	if (type) value = 2;
	else value = 1;
	//考虑纵向连成线
	int bottomIndex = x; //最底端是1的位置
	while (bottomIndex < M - 1) {
		if (myboard[bottomIndex + 1][y] != value) {
			break;
		}
		bottomIndex++;
	}
	if (bottomIndex - x >= 3) {
		return true;
	}
	//考虑横向连成线
	int leftIndex = y; //最左端是1的位置
	while (leftIndex > 0) {
		if (myboard[x][leftIndex - 1] != value) {
			break;
		}
		leftIndex--;
	}
	int rightIndex = y; //最右端是1的位置
	while (rightIndex < N - 1) {
		if (myboard[x][rightIndex + 1] != value) {
			break;
		}
		rightIndex++;
	}
	if (rightIndex - leftIndex >= 3) {
		return true;
	}
	//考虑正斜线连成线
	int leftDelta = 0; //向左端拓展的个数
	while (y - leftDelta > 0 && x - leftDelta > 0) {
		if (myboard[x - leftDelta - 1][y - leftDelta - 1] != value) {
			break;
		}
		leftDelta++;
	}
	int rightDelta = 0; //向右端扩展的个数
	while (y + rightDelta < N - 1 && x + rightDelta < M - 1) {
		if (myboard[x + rightDelta + 1][y + rightDelta + 1] != value) {
			break;
		}
		rightDelta++;
	}
	if (rightDelta + leftDelta >= 3) {
		return true;
	}
	//考虑反斜线连成线
	leftDelta = 0; //向左端拓展的个数
	while (y - leftDelta > 0 && x + leftDelta < M - 1) {
		if (myboard[x + leftDelta + 1][y - leftDelta - 1] != value) {
			break;
		}
		leftDelta++;
	}
	rightDelta = 0; //向右端扩展的个数
	while (y + rightDelta < N - 1 && x - rightDelta > 0) {
		if (myboard[x - rightDelta - 1][y + rightDelta + 1] != value) {
			break;
		}
		rightDelta++;
	}
	if (rightDelta + leftDelta >= 3) {
		return true;
	}
	return false;
}
//棋盘的评价函数
int evaluate_board(bool type,const int M,const int N) {
	int temp_score2 = 0;
	int temp_score1 = 0;
	int xindex, yindex;
	for (int i = 0; i < N; i++) {	
		if (mytop[i] == M) continue;
		else {
			xindex = mytop[i];
			yindex = i;
			//if (myboard[xindex][yindex] == 2) temp_score2 += evaluate_each_point2(xindex, yindex, M, N);
			//if (myboard[xindex][yindex] == 1) temp_score1 += evaluate_each_point1(xindex, yindex, M, N);
			//判断走的该步是不是危险走步
			if (myboard[xindex][yindex] == 2) {
				temp_score2 += evaluate_each_point2(xindex, yindex, M, N);
				if (mytop[i] > 0) {
					if (hasThreatPoint(false, xindex - 1, yindex, M, N)) temp_score2 -= 9999999;
				}
			}
			if (myboard[xindex][yindex] == 1) {
				temp_score1 += evaluate_each_point1(xindex, yindex, M, N);
				if (mytop[i] > 0) {
					if (hasThreatPoint(true, xindex - 1, yindex, M, N)) temp_score1 -= 9999999;
				}
			}
			//////
		}
	}
	if (type) return (int(0.5*temp_score2) - temp_score1);
	else return (temp_score1 - int(0.5*temp_score2));
}