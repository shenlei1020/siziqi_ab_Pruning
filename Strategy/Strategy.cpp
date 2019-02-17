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
static int next_step[2]; //�洢��һ�����߲�
static int *mytop;  //�洢ÿһ����չ���top
static int **myboard;  // �洢ÿ����չ���board
static int DEPTH = 8; //���ü�֦���


int evaluate_board(bool type, const int M, const int N); //�������ۺ���
bool hasThreatPoint(bool type, int x, int y, const int M, const int N);  //�ж�һ�����Ƿ�����в��
bool userWin(const int x, const int y, const int M, const int N, int* const* board);  //�û�ʤ
bool machineWin(const int x, const int y, const int M, const int N, int* const* board);  //����ʤ
bool isTie(const int N, const int* top);  //ƽ��


int negmaxsearch(bool mytype,int mydepth, int myalpha, int mybeta, const int M, const int N) {
	int score=0;
	//�жϾ��棻
	if (machineWin(lastx, lasty, M, N, myboard) || userWin(lastx, lasty, M, N, myboard) || mydepth == 0|| isTie(N,mytop)) {
		return evaluate_board(mytype,M, N);
	}
	//��֦����Խ����lasty,������չ�����Լ�������Ч�ʡ���ȿ��ټ���2�㣡
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
	
	//�ڵ���չ
	for (int i = 0; i < N; i++) {
		if (mytop[sortindex[i]] == 0) continue; //�����в���չ
		//������չǰ�ĵ�
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
		//����mytop��myboard,lastx,lasty		
		lastx = mytop[sortindex[i]] - 1;
		lasty = sortindex[i];
		if (mytype) myboard[mytop[sortindex[i]] - 1][sortindex[i]] = 2;
		else myboard[mytop[sortindex[i]] - 1][sortindex[i]] = 1;
		if (mytop[sortindex[i]] == 1) {
			mytop[sortindex[i]] = 0;
		}
		else if (myboard[mytop[sortindex[i]]-2][sortindex[i]] == -1) mytop[sortindex[i]] -= 2; //�������ӵ㣬����mytop
		else mytop[sortindex[i]] -=1; //û�в������ӵ㣬����mytop

		//������ֵ�㷨�ĵݹ�
		score = -negmaxsearch(!mytype,mydepth-1,-mybeta,-myalpha,M,N); 
		
		//��ԭ��չǰ�ĵ�
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
		
		//alpha-beta��֦
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
	���Ժ����ӿ�,�ú������Կ�ƽ̨����,ÿ�δ��뵱ǰ״̬,Ҫ�����������ӵ�,�����ӵ������һ��������Ϸ��������ӵ�,��Ȼ�Կ�ƽ̨��ֱ����Ϊ��ĳ�������
	
	input:
		Ϊ�˷�ֹ�ԶԿ�ƽ̨ά����������ɸ��ģ����д���Ĳ�����Ϊconst����
		M, N : ���̴�С M - ���� N - ���� ����0��ʼ�ƣ� ���Ͻ�Ϊ����ԭ�㣬����x��ǣ�����y���
		top : ��ǰ����ÿһ���ж���ʵ��λ��. e.g. ��i��Ϊ��,��_top[i] == M, ��i������,��_top[i] == 0//��ռ���ʵ��λ��
		_board : ���̵�һά�����ʾ, Ϊ�˷���ʹ�ã��ڸú����տ�ʼ���������Ѿ�����ת��Ϊ�˶�ά����board
				��ֻ��ֱ��ʹ��board���ɣ����Ͻ�Ϊ����ԭ�㣬�����[0][0]��ʼ��(����[1][1])
				board[x][y]��ʾ��x�С���y�еĵ�(��0��ʼ��)
				board[x][y] == 0/1/2 �ֱ��Ӧ(x,y)�� ������/���û�����/�г������,�������ӵ㴦��ֵҲΪ0��������д�ĳ����Լ�����2
		lastX, lastY : �Է���һ�����ӵ�λ��, ����ܲ���Ҫ�ò�����Ҳ������Ҫ�Ĳ������ǶԷ�һ����
				����λ�ã���ʱ��������Լ��ĳ����м�¼�Է������ಽ������λ�ã�����ȫȡ�������Լ��Ĳ���
		noX, noY : �����ϵĲ������ӵ�(ע:��ʵ���������top�Ѿ����㴦���˲������ӵ㣬Ҳ����˵���ĳһ��
				������ӵ�����ǡ�ǲ������ӵ㣬��ôUI�����еĴ�����Ѿ������е�topֵ�ֽ�����һ�μ�һ������
				��������Ĵ�����Ҳ���Ը�����ʹ��noX��noY��������������ȫ��Ϊtop������ǵ�ǰÿ�еĶ�������,
				��Ȼ�������ʹ��lastX,lastY�������п��ܾ�Ҫͬʱ����noX��noY��)
		���ϲ���ʵ���ϰ����˵�ǰ״̬(M N _top _board)�Լ���ʷ��Ϣ(lastX lastY),��Ҫ���ľ�������Щ��Ϣ�¸������������ǵ����ӵ�
	output:
		������ӵ�Point
*/
extern "C" __declspec(dllexport) Point* getPoint(const int M, const int N, const int* top, const int* _board, 
	const int lastX, const int lastY, const int noX, const int noY){
	/*
		��Ҫ������δ���
	*/
	int x = -1, y = -1;//���ս�������ӵ�浽x,y��
	int** board = new int*[M];
	for(int i = 0; i < M; i++){
		board[i] = new int[N];
		for(int j = 0; j < N; j++){
			board[i][j] = _board[i * N + j];
		}
	}
	
	/*
		�������Լ��Ĳ������������ӵ�,Ҳ���Ǹ�����Ĳ�����ɶ�x,y�ĸ�ֵ
		�ò��ֶԲ���ʹ��û�����ƣ�Ϊ�˷���ʵ�֣�����Զ����Լ��µ��ࡢ.h�ļ���.cpp�ļ�
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
	//�ҵĴ��룺         //
	board[noX][noY] = -1; //�Ѳ������ӵ��������ӵ��������
    //��ʼ��myboard
	myboard = new int *[M];
	for (int i = 0; i < M; i++) {
		myboard[i] = new int[N];
		for (int j = 0; j < N; j++) {
			myboard[i][j] = board[i][j];
		}
	}
	//_cprintf("%d\t ", myboard[noX][noY]);
	//��ʼ��mytop
	mytop = new int [N];
	for (int i = 0; i < N; i++) {
		mytop[i] = top[i];
	}
	//��ʼ��lastx��lasty
	lastx = lastX;
	lasty = lastY;
	//_cprintf("%d\t%d\t ", lastx,lasty);
	//�����ɱ�����в��
	int xindex, yindex;
	bool isabcut = true; //��ǣ������ж��ò���ab��֦
	for (int i = 0; i < N; i++) {
		if (mytop[i] == 0) continue;
		else {
				xindex = mytop[i] - 1;
				yindex = i;
				if (hasThreatPoint(true, xindex, yindex, M, N)) { //�жϱ�ɱ��
					next_step[0] = mytop[i] - 1;
					next_step[1] = i;
					isabcut = false;					
				}
				else {
					if (hasThreatPoint(false, xindex, yindex, M, N)) { //�ж���в��
						next_step[0] = mytop[i] - 1;
						next_step[1] = i;
						isabcut = false;
					}
				}
			}
		}

	//�ڲ�������ʱ���ѵ�һ���ӷ��м�Զ�벻�����ӵ��λ��
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
	//�ѵõ�����һ���߷�����x,y
	x = next_step[0];
	y = next_step[1];
	delete[]mytop;
	for (int i = 0; i < M; i++) {
		delete[] myboard[i];
	}
	delete[]myboard;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		��Ҫ������δ���
	*/
	clearArray(M, N, board);
	return new Point(x, y);
}


/*
	getPoint�������ص�Pointָ�����ڱ�dllģ���������ģ�Ϊ��������Ѵ���Ӧ���ⲿ���ñ�dll�е�
	�������ͷſռ䣬����Ӧ�����ⲿֱ��delete
*/
extern "C" __declspec(dllexport) void clearPoint(Point* p){
	delete p;
	return;
}

/*
	���top��board����
*/
void clearArray(int M, int N, int** board){
	for(int i = 0; i < M; i++){
		delete[] board[i];
	}
	delete[] board;
}


/*
	������Լ��ĸ�������������������Լ����ࡢ����������µ�.h .cpp�ļ�������ʵ������뷨
*/
// ������������
//////////////////////////////////////////////////////////////////////////////////////
//�ҵĴ��룺      //
////////////////////
//2�ӵ�����ۺ���
int evaluate_each_point2(int x, int y, const int M, const int N) {

	int score2 = 0;//2���ܷ�

	//��������Ʒ�	
	//����2��
	//��������
	int bottomIndex2 = x + 1; //��׶���2��λ��
	while (bottomIndex2 < M) {
		if (myboard[bottomIndex2][y] != 2) {
			break;
		}
		bottomIndex2++;
	}
	int topIndex2 = x - 1; //��׶���2��λ��
	int vertcount2 = bottomIndex2 - topIndex2 - 1;//����2��������
	//�����Ƿ��ǻ���
	while (topIndex2 >= 0) {
		if (myboard[topIndex2][y] != 0 && myboard[topIndex2][y] != 2) {
			break;
		}
		if (x - topIndex2 > 4) break;
		topIndex2--;
	}
	int vertwidth2 = bottomIndex2 - topIndex2 - 1; //������ӷ�Χ�����ж��ǲ��ǻ���
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
	//���Ǻ���
	//����2��
	int leftIndex2 = y - 1; //����˳���2��λ��
	while (leftIndex2 >= 0) {
		if (myboard[x][leftIndex2] != 2) {
			break;
		}
		leftIndex2--;
	}
	int rightIndex2 = y + 1; //���Ҷ˳���2��λ��
	while (rightIndex2 < N) {
		if (myboard[x][rightIndex2] != 2) {
			break;
		}
		rightIndex2++;
	}
	int horicount2 = rightIndex2 - leftIndex2 - 1; //�����2��������

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
	int horiwidth2 = rightIndex22 - leftIndex22 - 1; //������ӷ�Χ�����ж��ǲ��ǻ���
	switch (horicount2) {
	case 1:
		if (horiwidth2 > 3) score2 += 2000;
		else score2 += 500;
		break;
	case 2:
		if (mytop[rightIndex2] == x + 1 && mytop[leftIndex2] == x + 1) { //!!�ж�ɱ���߷�
			score2 += 9999;
			break;
		}
		if (horiwidth2 > 3) score2 += 5000;
		else score2 += 1000;
		break;
	case 3:
		if (mytop[rightIndex2]==x+1 && mytop[leftIndex2] == x+1) {  //!!�ж�ɱ���߷�
			score2 += 999999;
			break;
		}
		if (horiwidth2 > 3) score2 += 10000;
		else score2 += 2000;
		break;
	default:
		return  99999999;
	}


	//������б��
	//����2
	int leftDelta2 = 1; //�������չ�ĸ���
	while (y - leftDelta2 >= 0 && x - leftDelta2 >= 0) {
		if (myboard[x - leftDelta2][y - leftDelta2] != 2) {
			break;
		}
		leftDelta2++;
	}
	int rightDelta2 = 1; //���Ҷ���չ�ĸ���
	while (y + rightDelta2 < N && x + rightDelta2 < M) {
		if (myboard[x + rightDelta2][y + rightDelta2] != 2) {
			break;
		}
		rightDelta2++;
	}
	int leftupcount2 = rightDelta2 + leftDelta2 - 1; //�����2��������

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
	int leftupwidth2 = rightDelta22 + leftDelta22 - 1; //������ӷ�Χ�����ж��ǲ��ǻ���
	switch (leftupcount2) {
	case 1:
		if (leftupwidth2 > 3) score2 += 2000;
		else score2 += 500;
		break;
	case 2:
		if (mytop[y - leftDelta2] == x - leftDelta2 + 1 && mytop[y + rightDelta2] == x + rightDelta2 + 1) {  //!!�ж�ɱ���߷�
			score2 += 9999;
			break;
		}
		if (leftupwidth2 > 3) score2 += 5000;
		else score2 += 1000;
		break;
	case 3:
		if (mytop[y - leftDelta2]== x - leftDelta2+1 && mytop[y + rightDelta2] == x + rightDelta2+1) {  //!!�ж�ɱ���߷�
			score2 += 999999;
			break;
		}
		if (leftupwidth2 > 3) score2 += 10000;
		else score2 += 2000;
		break;
	default:
		return 99999999;
	}

	//���Ƿ�б��
	//����2
	leftDelta2 = 1; //�������չ�ĸ���
	while (y - leftDelta2 >= 0 && x + leftDelta2 < M) {
		if (myboard[x + leftDelta2][y - leftDelta2] != 2) {
			break;
		}
		leftDelta2++;
	}
	rightDelta2 = 1; //���Ҷ���չ�ĸ���
	while (y + rightDelta2 < N && x - rightDelta2 >= 0) {
		if (myboard[x - rightDelta2][y + rightDelta2] != 2) {
			break;
		}
		rightDelta2++;
	}
	int leftdowncount2 = rightDelta2 + leftDelta2 - 1; //�����2��������

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
	int leftdownwidth2 = rightDelta22 + leftDelta22 - 1; //������ӷ�Χ�����ж��ǲ��ǻ���
	switch (leftdowncount2) {
	case 1:
		if (leftdownwidth2 > 3) score2 += 2000;
		else score2 += 500;
		break;
	case 2:
		if (mytop[y - leftDelta2] == x + leftDelta2 + 1 && mytop[y + rightDelta2] == x - rightDelta2 + 1) {  //!!�ж�ɱ���߷�
			score2 += 9999;
			break;
		}
		if (leftdownwidth2 > 3) score2 += 5000;
		else score2 += 1000;
		break;
	case 3:
		if ((mytop[y - leftDelta2] == x + leftDelta2 +1) && (mytop[y + rightDelta2] == x - rightDelta2 +1)) {  //!!�ж�ɱ���߷�
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

	int score1 = 0;//1���ܷ�
	//��������Ʒ�	
	//����1
	int bottomIndex1 = x + 1; //��׶���1��λ��
	while (bottomIndex1 < M) {
		if (myboard[bottomIndex1][y] != 1) {
			break;
		}
		bottomIndex1++;
	}
	int topIndex1 = x - 1; //��׶���1��λ��
	
	int vertcount1 = bottomIndex1 - topIndex1 - 1;//����1��������

	while (topIndex1 >= 0) {
		if (myboard[topIndex1][y] != 0 && myboard[topIndex1][y] != 1) {
			break;
		}
		if (x - topIndex1 > 4) break;
		topIndex1--;
	}
	int vertwidth1 = bottomIndex1 - topIndex1 - 1; //������ӷ�Χ�����ж��ǲ��ǻ���
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


	//���Ǻ���
	//����1��
	int leftIndex1 = y - 1; //����˳���1��λ��
	while (leftIndex1 >= 0) {
		if (myboard[x][leftIndex1] != 1) {
			break;
		}
		leftIndex1--;
	}
	int rightIndex1 = y + 1; //���Ҷ˳���1��λ��
	while (rightIndex1 < N) {
		if (myboard[x][rightIndex1] != 1) {
			break;
		}
		rightIndex1++;
	}
	int horicount1 = rightIndex1 - leftIndex1 - 1; //�����1��������

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
	int horiwidth1 = rightIndex11 - leftIndex11 - 1; //������ӷ�Χ�����ж��ǲ��ǻ���
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


	//������б��
	//����1
	int leftDelta1 = 1; //�������չ�ĸ���
	while (y - leftDelta1 >= 0 && x - leftDelta1 >= 0) {
		if (myboard[x - leftDelta1][y - leftDelta1] != 1) {
			break;
		}
		leftDelta1++;
	}
	int rightDelta1 = 1; //���Ҷ���չ�ĸ���
	while (y + rightDelta1 < N && x + rightDelta1 < M) {
		if (myboard[x + rightDelta1][y + rightDelta1] != 1) {
			break;
		}
		rightDelta1++;
	}
	int leftupcount1 = rightDelta1 + leftDelta1 - 1; //���1��������

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
	int leftupwidth1 = rightDelta11 + leftDelta11 - 1; //������ӷ�Χ�����ж��ǲ��ǻ���
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

	//���Ƿ�б��
	//����1
	leftDelta1 = 1; //�������չ�ĸ���
	while (y - leftDelta1 >= 0 && x + leftDelta1 < M) {
		if (myboard[x + leftDelta1][y - leftDelta1] != 1) {
			break;
		}
		leftDelta1++;
	}
	rightDelta1 = 1; //���Ҷ���չ�ĸ���
	while (y + rightDelta1 < N && x - rightDelta1 >= 0) {
		if (myboard[x - rightDelta1][y + rightDelta1] != 1) {
			break;
		}
		rightDelta1++;
	}
	int leftdowncount1 = rightDelta1 + leftDelta1 - 1; //�����2��������

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
	int leftdownwidth1 = rightDelta11 + leftDelta11 - 1; //������ӷ�Χ�����ж��ǲ��ǻ���
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

//type=true:�鿴��λ���Ƿ��ʤ��;type=false:�鿴��в��
bool hasThreatPoint(bool type, int x, int y, const int M, const int N){
	int value;
	if (type) value = 2;
	else value = 1;
	//��������������
	int bottomIndex = x; //��׶���1��λ��
	while (bottomIndex < M - 1) {
		if (myboard[bottomIndex + 1][y] != value) {
			break;
		}
		bottomIndex++;
	}
	if (bottomIndex - x >= 3) {
		return true;
	}
	//���Ǻ���������
	int leftIndex = y; //�������1��λ��
	while (leftIndex > 0) {
		if (myboard[x][leftIndex - 1] != value) {
			break;
		}
		leftIndex--;
	}
	int rightIndex = y; //���Ҷ���1��λ��
	while (rightIndex < N - 1) {
		if (myboard[x][rightIndex + 1] != value) {
			break;
		}
		rightIndex++;
	}
	if (rightIndex - leftIndex >= 3) {
		return true;
	}
	//������б��������
	int leftDelta = 0; //�������չ�ĸ���
	while (y - leftDelta > 0 && x - leftDelta > 0) {
		if (myboard[x - leftDelta - 1][y - leftDelta - 1] != value) {
			break;
		}
		leftDelta++;
	}
	int rightDelta = 0; //���Ҷ���չ�ĸ���
	while (y + rightDelta < N - 1 && x + rightDelta < M - 1) {
		if (myboard[x + rightDelta + 1][y + rightDelta + 1] != value) {
			break;
		}
		rightDelta++;
	}
	if (rightDelta + leftDelta >= 3) {
		return true;
	}
	//���Ƿ�б��������
	leftDelta = 0; //�������չ�ĸ���
	while (y - leftDelta > 0 && x + leftDelta < M - 1) {
		if (myboard[x + leftDelta + 1][y - leftDelta - 1] != value) {
			break;
		}
		leftDelta++;
	}
	rightDelta = 0; //���Ҷ���չ�ĸ���
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
//���̵����ۺ���
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
			//�ж��ߵĸò��ǲ���Σ���߲�
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