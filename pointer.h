#pragma once
#include <iostream>
using namespace std;
class PointerList
{
public:
	struct Pointer
	{
		int curplace;//指向的符号表中的位置
		Pointer* child;//儿子
		Pointer* father;//父亲
		Pointer(int _curplace) { curplace = _curplace; child = NULL; father = NULL; }
	};
public:
	Pointer *head;
	int counter;
	PointerList();
	void pushPointer();
	void deletePointer();
};

