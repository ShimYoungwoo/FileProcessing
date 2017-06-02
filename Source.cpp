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
	fp2.open("Students.hash", ios::binary);	//hash 파일

	ofstream fp3;
	fp3.open("Students.DB", ios::binary | ios::app);	//DB 파일 : 덮어쓰기 가능

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
		/* 1. Student 데이터를 input file에서 읽어와 타입 변환 */
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

		/* 2. 타입 변환된 데이터를 Student struct에 저장 */
		strcpy(std.name, wholename.c_str());
		std.studentID = stdID;
		std.score = sc;
		std.advisorID = admID;

		/* 3. hash function으로 저장할 블럭 계산 및 해쉬테이블에 insert */
		Node *t = new Node(std.studentID);
		insertItem(t);

		/* 4. hash function으로 정해진 DB 블럭에 데이터 저장
		* 위치 옮기기 : seekp
		* 4096*blockNum + item수*sizeof(std)
		*/
		//hash table에서 key를 이용해 블럭넘버 찾아오기
		int blockNum = findBlockNum(t->getKey());
		fp3.seekp(MAX_BLOCK_SIZE*blockNum + sizeof(Student)*countNum(blockNum), ios::beg);	//시작(ios::beg)부터 4096*blockNum + item수*sizeof(std)
		fp3.write((char*)&std, sizeof(std));	//덮어쓰기로 DB 파일 수정 가능
	}

	/* Students.hash 파일에 hash table 쓰기 */
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
	/* table 크기를 2배로 확장 */
	int sizeOld = tableSize;	//기존 table size
	tableSize = tableSize * 2;
	//cout << "table size up " << sizeOld << " to " << tableSize << endl;
	LinkedList **tableOld;	//기존 table을 백업하기 위함
	int *blockTableOld;
	tableOld = table;	//기존 table을 백업
	blockTableOld = blockTable;

	table = new LinkedList*[tableSize];	//기존 table의 크기를 2배로 확장
	blockTable = new int[tableSize];
	for (int i = 0; i < tableSize; i++) {
		table[i] = NULL;
		blockTable[i] = NULL;
	}
	//	cout << "complete table size double\n";

	/* 기존 table(tableOld)의 리스트와 리스트 내부 노드를 확장된 table에 insert */
	for (int i = 0; i < sizeOld; i++) {
		if (tableOld[i] != NULL && blockTableOld[i] != NULL) {
			LinkedList *list = tableOld[i];	//기존 table의 i번째 LinkedList
			int b = list->getBlockNum();	//기존 LinkedList의 블럭넘버
			Node *t = list->getFirst();
			while (t != NULL) {
				insertItem(t);	//노드 t를 다시 table에 insert
				t = t->getNext();
			}
		}
	}
	delete[] tableOld;	//기존 table 제거
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