#pragma once
#include <iostream>
using namespace std;
class PointerList
{
public:
	struct Pointer
	{
		int curplace;//ָ��ķ��ű��е�λ��
		Pointer* child;//����
		Pointer* father;//����
		Pointer(int _curplace) { curplace = _curplace; child = NULL; father = NULL; }
	};
public:
	Pointer *head;
	int counter;
	PointerList();
	void pushPointer();
	void deletePointer();
};

