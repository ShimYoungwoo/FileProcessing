#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "LinkedList.h"
#define DEFAULT_TABLE_SIZE 8
using namespace std;

struct Student {
	char name[20];
	unsigned studentID;	//key for hashing
	float score;	//index for indexing
	unsigned advisorID;
};

class DHash {
	LinkedList **table;	//block number and Nodes
	int tableSize;	//size of table
	int *blockTable;	//hash table with block number
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
	int getBlockNum(int i) {
		int result = blockTable[i];
		return result;
	}
};

DHash::DHash() {
	tableSize = DEFAULT_TABLE_SIZE;	//default table size : 16
	table = new LinkedList*[tableSize];
	for (int i = 0; i < tableSize; i++) {
		table[i] = NULL;
	}
	blockTable = new int[tableSize];
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
		//cout << wholename << endl;

		int j = 0;
		while (ptr[j] != NULL) {
			//cout<<ptr[j]<<endl;
			j++;
			ptr[j] = strtok(NULL, ",");
		}

		stdID = atoi(ptr[1]);
		//cout << "studentID : " << stdID << endl;
		sc = atof(ptr[2]);
		//cout << "score : " << sc << endl;
		admID = atoi(ptr[3]);
		//cout << "adminID : " << admID << endl;

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

int DHash::findBlockNum(unsigned k) {
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

int DHash::countNum(int blockNum) {
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

int DHash::hashFunc(unsigned k) {
	int result;

	result = k % tableSize;

	return result;
}

void DHash::doubleTable() {
	/* table ũ�⸦ 2��� Ȯ�� */
	int sizeOld = tableSize;	//���� table size
	tableSize = tableSize * 2;
	//cout << "table size up " << sizeOld << " to " << tableSize << endl;
	LinkedList **tableOld;	//���� table�� ����ϱ� ����
	int *blockTableOld;
	tableOld = table;	//���� table�� ���
	blockTableOld = blockTable;

	table = new LinkedList*[tableSize];	//���� table�� ũ�⸦ 2��� Ȯ��
	blockTable = new int[tableSize];
	for (int i = 0; i < tableSize; i++) {
		table[i] = NULL;
		blockTable[i] = NULL;
	}
	//	cout << "complete table size double\n";

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
			cout << getBlockNum(i) << endl;
			//table[i]->printList();
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