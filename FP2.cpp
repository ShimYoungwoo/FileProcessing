#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#define DEFAULT_TABLE_SIZE 8
#define MAX_BLOCK_SIZE 4096
using namespace std;

class Node {	//for hash
private:
	Node *next;	//���� ���(������)
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
	int blockNum;	//�ش� ����Ʈ�� ���� ���ѹ�
	int numItem;	//�ش� ����Ʈ�� ����ִ� ������(���) ��
	int size;	//�ش� ����Ʈ�� ũ�� : 4k�� ������ overflow �߻�
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

struct Student {
	char name[20];
	unsigned studentID;	//key for hashing
	float score;	//index for indexing
	unsigned advisorID;
};

class DHash {
	LinkedList **table;	//block number and Nodes
	int tableSize;	//size of table
	int *blockTable;	//hash table for block number
public:
	DHash();
	~DHash();
	void insertItem(Node *t);
	void setHashTable(ifstream &fp);
	int hashFunc(unsigned k);	//compute block number
	void doubleTable();	//extend hash table about double
	void printTable();
	int countNum(int blockNum);	//count total item matched to blockNum in table
	int findBlockNum(unsigned k);	//find block number of key in hash table
};

DHash::DHash() {
	tableSize = DEFAULT_TABLE_SIZE;	//default table size : 8
	table = new LinkedList*[tableSize];
	blockTable = new int[tableSize];
	for (int i = 0; i < tableSize; i++) {
		table[i] = NULL;
	}
	for (int i = 0; i < tableSize; i++) {
		blockTable[i] = -1;
	}
}

DHash::~DHash() {
	delete table;
	delete blockTable;
}

void DHash::insertItem(Node *t) {
	int hash;
	hash = hashFunc(t->getKey());
	if (table[hash] == NULL) {
		table[hash] = new LinkedList();
		table[hash]->setBlockNum(hash);
		table[hash]->insertItem(t);
		blockTable[hash] = hash;
		//cout << "insert item block "<<table[hash]->getBlockNum()<<" in table[" << hash << "]\n";
	}
	else {
		if (table[hash]->IsFull()) {
			cout << "overflow, double table size\n";
			doubleTable();
			insertItem(t);
			//printTable();
			cout << endl;
		}
		else {
			table[hash]->insertItem(t);
			//cout << "insert item in table[" << hash << "]\n";
		}
	}
}

void DHash::setHashTable(ifstream &fp) {
	if (fp.is_open() != true) {
		cout << "error\n";
		return;
	}

	ofstream fp2;
	fp2.open("Students.hash", ios::binary);	//hash ����

	ofstream fp3;
	fp3.open("Students.DB", ios::binary | ios::app);	//DB ���� : ����� ����

	if (fp2.is_open() != true || fp3.is_open() != true) {
		cout << "error2\n";
		return;
	}

	int num;	//number of student records
	string buffer;
	getline(fp, buffer);
	stringstream str(buffer);
	str >> num;
	cout << "number of data : " << num << endl;

	Student std;

	for (int i = 0; i < num; i++) {
		/* 1. Student �����͸� input file���� �о�� Ÿ�� ��ȯ */
		string buffer2;
		getline(fp, buffer2);
		//cout << buffer2 << endl;
		stringstream str2(buffer2);

		string wholename;
		string lastname;
		unsigned stdID;
		float sc;
		unsigned admID;

		str2 >> wholename >> lastname;

		char* tmp = new char[lastname.length() + 1];
		strcpy(tmp, lastname.c_str());

		char** ptr = new char*[4];
		ptr[0] = strtok(tmp, ",");
		lastname = ptr[0];

		wholename = wholename + " " + lastname;

		int j = 0;
		while (ptr[j] != NULL) {
			j++;
			ptr[j] = strtok(NULL, ",");
		}

		stdID = atoi(ptr[1]);
		sc = atof(ptr[2]);
		admID = atoi(ptr[3]);

		/* 2. Ÿ�� ��ȯ�� �����͸� Student struct�� ���� */
		strcpy(std.name, wholename.c_str());
		std.studentID = stdID;
		std.score = sc;
		std.advisorID = admID;

		/* 3. hash function���� ������ �� ��� �� �ؽ����̺� insert */
		Node *t = new Node(std.studentID);
		insertItem(t);

		/* 4. hash function���� ������ DB ���� ������ ����
		* ��ġ �ű�� : seekp
		* 4096*blockNum + item��*sizeof(std)
		*/
		//hash table���� key�� �̿��� ���ѹ� ã�ƿ���
		int blockNum = findBlockNum(t->getKey());
		fp3.seekp(MAX_BLOCK_SIZE*blockNum + sizeof(Student)*countNum(blockNum), ios::beg);	//����(ios::beg)���� 4096*blockNum + item��*sizeof(std)
		fp3.write((char*)&std, sizeof(std));	//������ DB ���� ���� ����
	}

	/* Students.hash ���Ͽ� hash table ���� */
	for (int i = 0; i < tableSize; i++) {
		fp2.write((char*)&blockTable[i], sizeof(blockTable[i]));
	}

	fp2.close();
	fp3.close();
}

int DHash::findBlockNum(unsigned k) {	//k �־ hash table���� ���ѹ� ã��
	int bn = hashFunc(k);
	int result = -1;

	for (int i = 0; i < tableSize; i++) {
		if (table[i] != NULL) {
			if (table[i]->getBlockNum() == bn) {
				result = bn;
			}
		}
	}

	return result;
}

int DHash::countNum(int blockNum) {	//�ش� ���� �� �� record�� ����ִ���
	int result = 0;

	for (int i = 0; i < tableSize; i++) {
		if (table[i] != NULL) {
			if (table[i]->getBlockNum() == blockNum) {
				result = result + table[i]->getNumItem();
			}
		}
	}
	return result;
}

int DHash::hashFunc(unsigned k) { //hash function
	int result;

	result = k % tableSize;

	return result;
}

void DHash::doubleTable() {
	/* table ũ�⸦ 2��� Ȯ�� */
	int sizeOld = tableSize;	//���� table size
	tableSize = tableSize * 2;

	LinkedList **tableOld;	//���� table�� ����ϱ� ����
	int *blockTableOld;	//���� hash table
	tableOld = table;	//���� table �� ���
	blockTableOld = blockTable;

	table = new LinkedList*[tableSize];	//���� table�� ũ�⸦ 2��� Ȯ��
	blockTable = new int[tableSize];
	for (int i = 0; i < tableSize; i++) {
		table[i] = NULL;
		blockTable[i] = NULL;
	}

	/* ���� table(tableOld)�� ����Ʈ�� ����Ʈ ���� ��带 Ȯ��� table�� insert */
	for (int i = 0; i < sizeOld; i++) {
		if (tableOld[i] != NULL && blockTableOld[i] != NULL) {
			LinkedList *list = tableOld[i];	//���� table�� i��° LinkedList
			int b = list->getBlockNum();	//���� LinkedList�� ���ѹ�
			Node *t = list->getFirst();
			while (t != NULL) {
				insertItem(t);	//��� t�� �ٽ� table�� insert
				t = t->getNext();
			}
		}
	}
	delete[] tableOld;	//���� table ����
	delete[] blockTableOld;
}

void DHash::printTable() {
	for (int i = 0; i < tableSize; i++) {
		if (table[i] != NULL) {
			cout << "table[" << i << "] block number : ";
			cout << blockTable[i] << endl;
		}
	}
}

int main() {
	ifstream fp;
	fp.open("sampleData.csv");

	DHash hash;
	hash.setHashTable(fp);
	hash.printTable();

	fp.close();

	return 0;
}