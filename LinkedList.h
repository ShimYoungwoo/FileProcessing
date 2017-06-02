#include <iostream>
#define MAX_BLOCK_SIZE 4096
using namespace std;

class Node {	//for hash
private:
	Node *next;	//다음 노드(아이템)
	unsigned key;	//studentID
public:
	Node() {
		next = NULL;
		key = -1;
	}
	Node(unsigned k) {
		next = NULL;
		key = k;
	}
	Node *getNext() {
		return next;
	}
	unsigned getKey() {
		return key;
	}
	void setNext(Node *n) {
		next = n;
	}
	void setKey(unsigned k) {
		key = k;
	}
};

class LinkedList {
private:
	int blockNum;	//해당 리스트가 속한 블럭넘버
	int numItem;	//해당 리스트에 들어있는 아이템(노드) 수
	int size;	//해당 리스트의 크기 : 4k를 넘으면 overflow 발생
	Node *first;
public:
	LinkedList() {
		blockNum = -1;
		numItem = 0;
		size = 0;
		first = NULL;
	}
	LinkedList(int b) {
		blockNum = b;
		numItem = 0;
		size = 0;
		first = NULL;
	}
	~LinkedList() {
		deleteList(first);
	}
	void deleteList(Node *t) {
		if (t->getNext() == NULL) {
			delete t;
		}
		else {
			deleteList(t->getNext());
			delete t;
		}
	}
	void insertItem(Node *t);
	Node *getFirst() {
		return first;
	}
	Node *getItem(unsigned k);
	int getBlockNum() {
		return blockNum;
	}
	void setBlockNum(int b) {
		blockNum = b;
	}
	int getNumItem() {
		return numItem;
	}
	int getSize() {
		return size;
	}
	void printList();
	bool IsFull();
};

void LinkedList::insertItem(Node *t) {
	if (first == NULL) {
		first = t;
		t->setNext(NULL);
		numItem = numItem + 1;
		size = size + 32;
	}
	else {
		Node *p = first;
		if (IsFull()) {
			cout << "overflow!\n";
		}
		else {
			while (p->getNext() != NULL) {
				p = p->getNext();
			}
			p->setNext(t);
			t->setNext(NULL);
			numItem = numItem + 1;
			size = size + 32;
		}
	}
}

Node* LinkedList::getItem(unsigned k) {
	Node *t = first;
	while (t->getNext() != NULL) {
		if (t->getKey() == k) {
			return t;
		}
		t = t->getNext();
	}
	cout << "no matched Item\n";
	return NULL;
}

void LinkedList::printList() {
	if (first == NULL) {
		cout << "no Item to print\n";
		return;
	}

	cout << "[block number : " << blockNum << " ]" << endl;
	Node *t = first;
	while (t->getNext() != NULL) {
		cout << t->getKey() << " ";
		t = t->getNext();
	}
	cout << endl;
}

bool LinkedList::IsFull() {
	bool result = false;

	if (MAX_BLOCK_SIZE - size < 32) {
		cout << "list is full, overflow\n";
		result = true;
	}

	return result;
}