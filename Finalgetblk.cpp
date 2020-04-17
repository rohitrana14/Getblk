#include<iostream>
#include<string>
#include<cstdlib>
#include<unistd.h>
#include<time.h>
#include<thread>
#include<mutex>
#include <condition_variable>

using namespace std;


#define NUMBEROFQUEUES 4
#define NUMBEROFBUFFERS 5
#define NUMBEROFREQUESTS 5
#define MAXBLOCKNO 100

std::mutex  m,m1, mprint, s[MAXBLOCKNO];
std::condition_variable cv;
bool isAvailable, isfree[MAXBLOCKNO];

struct buffer
{
  //Data Members of Buffer Header
  int blockNo;
  int status;
  std::string data;
  struct buffer *hashQ_fp;
  struct buffer *hashQ_bp;
  struct buffer *free_fp;
  struct buffer *free_bp;
  // Constructor to intialise values
public:
  buffer() : blockNo(0), status(0), data(""), hashQ_fp(nullptr), hashQ_bp(nullptr), free_fp(nullptr), free_bp(nullptr) {}

  //Constructor to assign values
  buffer(int blockNo, int status = 0, std::string data = " ", buffer *hashQ_fp = 0, buffer *hashQ_bp = 0, buffer *free_fp = 0, buffer *free_bp = 0)
  {
    this->blockNo = blockNo;
    this->status = status;
    this->data = data;
    this->hashQ_fp = hashQ_fp;
    this->hashQ_fp = hashQ_fp;
    this->free_fp = free_fp;
    this->free_bp = free_bp;
  }
  //to display buffer information
  void display()
  {
	cout << "*****Buffer Information*****\n";

	cout << "Block Number : ";
	if (blockNo != 0)
	{
		cout << blockNo << endl;
	}
	else
	{
		cout << "Buffer Not Allocated\n";
	}
	cout << "Status : ";
	if (status == 0)
	{
		cout << "Buffer Not Allocated\n";
	}
	else if (status == 1)
	{
		cout << "Buffer Currently Locked\n";
	}
	else if (status == 2)
	{
		cout << "Buffer Data Invalid\n";
	}
	else if (status == 3)
	{
		cout << "Buffer Marked Delayed Write\n";
	}
	else if (status == 4)
	{
		cout << "Process Waiting for Buffer\n";
	}
	cout << "Data : " << data << endl;
	}

};

class doublyLinkList
{

  buffer *head;
  buffer *tail;

public:
  doublyLinkList() : head(nullptr), tail(nullptr) {}

  //Check if Linked List is Empty or Not
  bool isEmpty()
  {
	if (head == nullptr)
	{
		return true;
	}
	return false;
	}

  //Find a Buffer in  Hash Queue
  buffer *findBuff_hashQ(int blockNum)
  {
	if (!isEmpty())
	{
		buffer *temp = head;
		while (temp != nullptr)
		{
			if (temp->blockNo == blockNum)
			{
				return temp;
			}
			temp = temp->hashQ_fp;
		}
	}
	else
		return nullptr;
}

  // Find Buffer in FreeList
  buffer *findBuff_fList(int blockNum)
  {
	if (!isEmpty())
	{
		buffer *temp = head;
		do
		{
			if (temp->blockNo == blockNum)
			{
				return temp;
			}
			temp = temp->free_fp;
		} while (temp != head);
	}
	else
		return nullptr;
}

  //Add Buffer at Start of the Free List
  void insertAtHead_fList(buffer *insertBuff)
  {
	if (head == nullptr)
	{
		insertBuff->free_fp = insertBuff->free_bp = insertBuff;
		head = tail = insertBuff;
	}
	else
	{
		insertBuff->free_fp = head;
		insertBuff->free_bp = tail;
		tail->free_fp = head->free_bp = insertBuff;
		head = insertBuff;
	}
}

  //Add Buffer at Tail of the Hash Queue
  void insertAt_hashQ_Tail(buffer *insertBuff)
  {
	if (head == nullptr)
	{
		insertBuff->hashQ_fp = insertBuff->hashQ_bp = nullptr;
		head = tail = insertBuff;
	}
	else
	{
		insertBuff->hashQ_bp = tail;
		insertBuff->hashQ_fp = nullptr;
		tail->hashQ_fp = insertBuff;
		tail = insertBuff;
	}
}

  // Add Buffer at Tail of Free List
  void insertAt_fList_Tail(buffer *insertBuff)
  {
	if (head == nullptr)
	{
		insertBuff->free_fp = insertBuff->free_bp = insertBuff;
		head = tail = insertBuff;
	}
	else
	{
		insertBuff->free_bp = tail;
		insertBuff->free_fp = head;
		head->free_bp = tail->free_fp = insertBuff;
		tail = tail->free_fp;
	}
}

  //Remove Buffer from Head of the Free List
  buffer *removeFrom_fList_Head()
  {
	buffer *removed = nullptr;
	if (!isEmpty())
	{
		if (head == head->free_fp)
		{ //Only Buffer in the Linked List
			removed = head;
			head = tail = nullptr;
		}
		else
		{
			tail->free_fp = head->free_fp;
			head->free_fp->free_bp = head->free_bp;
			removed = head;
			head = head->free_fp;
		}
	}
	return removed;
}

  //Remove a Specific Buffer from Hash Queue
  void removeSpecificBuf_hashQ(int blockNum)
  {
	buffer *temp = findBuff_hashQ(blockNum);
	//Hash Queue Removal
	if (temp != nullptr)
	{
		if (temp->hashQ_fp == nullptr && temp->hashQ_bp == nullptr)
		{ //Only Buffer in HashQueue
			head = tail = nullptr;
		}
		else
		{
			if (temp == head)
			{ //If Head Buffer is the Buffer to BeRemoved
				head = head->hashQ_fp;
				head->hashQ_bp = nullptr;
			}
			else if (temp == tail)
			{ //If Tail Buffer is the Buffer toBeRemoved
				tail = tail->hashQ_bp;
				tail->hashQ_fp = nullptr;
			}
			else
			{
				temp->hashQ_bp->hashQ_fp = temp->hashQ_fp;
				temp->hashQ_fp->hashQ_bp = temp->hashQ_bp;
			}
		}
	}
}


  //Remove a Specific Buffer from Free List
  void removeSpecificBuf_fList(int blockNum)
  {
	buffer *temp = findBuff_fList(blockNum);
	if (temp != nullptr)
	{
		if (temp->free_fp == temp)
		{ //Only Buffer in Linked List
			head = tail = nullptr;
		}
		else
		{
			if (temp == head)
			{ //If Head Buffer is the Buffer toBeRemoved from Free List
				head = head->free_fp;
			}
			else if (temp == tail)
			{ //If Tail Buffer is the Buffer toBeRemoved from Free List
				tail = tail->free_bp;
			}
			temp->free_bp->free_fp = temp->free_fp;
			temp->free_fp->free_bp = temp->free_bp;
		}
	}
}


  buffer *head_fList() { return head; }

  // To Display Content of Free List
  void display_fList()
  {
	buffer *temp = head;
	cout << "\n\t\t\t Displaying FreeList \n";
	if (!isEmpty())
	{
		cout << "Head :[" << head->blockNo << "]\tTail :[" << tail->blockNo << "]\n";
		cout << "\n[{ blockNo }, status , data ] \n\t";
		do
		{
			cout << "[ <" << temp->blockNo << ">," << temp->status << "," << temp->data << "]  <---> ";
			temp = temp->free_fp;
		} while (temp != head);
	}
	cout << "\n-------------------------------------------------------------------------------------------\n\n";
}

  //To Display content of Hash Queue
  void display_hashQ()
{
	if (!isEmpty())
	{
		buffer *temp = head;
		cout << "\n\t\t\t Displaying HashQueue [BlockNo mod  " << (temp->blockNo) % NUMBEROFQUEUES << " ] \n";
		cout << "Head :[" << head->blockNo << "]\tTail :[" << tail->blockNo << "]\n";
		while (temp != nullptr)
		{
			cout << "[ <" << temp->blockNo << ">," << temp->status << "," << temp->data << "]  <--->  ";
			temp = temp->hashQ_fp;
		}
	}
	cout << "\n------------------------------------------------------------------------------------------------\n\n";
}
};

doublyLinkList *hashQ[NUMBEROFQUEUES];
doublyLinkList *freeList;

// Calculates the hash value 
int hashValue(int blockNum){
    int hv= (blockNum % NUMBEROFQUEUES);
    return hv;
    
}
// Buffer Release
void brelse(buffer *buf)
{ // To relse a buffer by process when it completes its work with it
	if (buf->status == 3)
	{
		buf->status = 0;
		cout << "\n\t Buffer " << buf->blockNo << " marked delayed is being  written to disk ";
		sleep(2);
		freeList->insertAtHead_fList(buf);
	}
	else
	{
		buf->status = 0;

		int rNo = rand() % 5;
		if (rNo == 1)
		{
			buf->status = 3;
		}
		cout << "\n\t Buffer " << buf->blockNo << " is released " << buf->status;
		freeList->insertAt_fList_Tail(buf);
	}

	isAvailable = true;
	isfree[buf->blockNo] = true;
	cv.notify_all(); // All waiting processes notified about relesed buffer
}

//Update Data in Buffer Buffer
void updateBuffer(buffer *bufferOnUpdate, int blockNum)
{ 
	bufferOnUpdate->blockNo = blockNum;
	bufferOnUpdate->status = 1;
}

void bwrite(buffer *wBuffer)
{
	if (wBuffer->status == 3)
	{ // For Delayed Write
		wBuffer->status = 0;
		sleep(4);
		freeList->insertAtHead_fList(wBuffer);
		
	} 

}
void updateStatus(buffer *temp, int st)
{
	temp->status = st;
}

//Get Buffer
buffer *getblk(int blockNum, int pid)
{
	buffer *allocBuffer = nullptr;

	while (allocBuffer == nullptr)
	{
		m1.lock();
		int hashQNo = hashValue(blockNum);
		buffer *blockBuffer = hashQ[hashQNo]->findBuff_hashQ(blockNum);
		if (blockBuffer != nullptr)
		{ //Buffer  on Hash Queue
			if (blockBuffer->status == 1)
			{ // Scenerio  5: Buffer on Hash Queue & Busy
				mprint.lock();
				cout << "\n\n\n\t ****____Scenerio 5:____**** \n\t\t ----> Buffer Busy.\n\t\t -----> Process  " << pid << "  Goes Sleep.\n";
				mprint.unlock();
				m1.unlock();
				std::unique_lock<std::mutex> lk(s[blockNum]);
				while (!isfree[blockNum])
				{
					cv.wait(lk);
				}
				mprint.lock();
				cout << " \n\t Process NO " << pid << " wakes up";
				mprint.unlock();
				continue;
			}
			else
			{
				blockBuffer = hashQ[hashQNo]->findBuff_hashQ(blockNum);
				blockBuffer->status = 0;
				if (blockBuffer->status == 0)
				{
					mprint.lock();
					cout << "\n\n\n\t ****_____ Scenerio 1:____****\n\t\t ----> Buffer Allocated \n"; //Scenerio 1: Buffer on Hash Queue & Free
					mprint.unlock();
					blockBuffer->status = 1;					 // Mark buffer busy
												
					freeList->removeSpecificBuf_fList(blockNum); //Remove Buffer Buffer from Free List
																 
					allocBuffer = blockBuffer;
					isfree[blockNum] = false;
					m1.unlock();
				}
			}
		}
		else
		{												 //Buffer  not on Hash Queue
													
			buffer *freeBuffer = freeList->head_fList(); //We get nullptr if free list is empty
										
			if (freeBuffer == nullptr)
			{ //Scenerio 4: No Buffer Available
				mprint.lock();
				cout << "\n\n\n\t ****____Scenerio 4: ____**** \n\t   ----> No Free Buffer Available. Process Sleeps\n";
				mprint.unlock();
				m1.unlock();
	
				std::unique_lock<std::mutex> lk(m);
				while (!isAvailable)
				{
					cv.wait(lk);
				}

				continue;

			}
			else if (freeBuffer->status == 3)
			{ //Scenerio 3: Buffer on Free List Marked Delayed Write
				mprint.lock();
				cout << "\n\n\n\t ****____ Scenerio 3 : _____****\n\t\t   ---->Delayed Write\n";
				mprint.unlock();
				freeBuffer = freeList->removeFrom_fList_Head();
				thread tdw(brelse, freeBuffer);
				tdw.detach();
				m1.unlock();
				continue;
			}
			else
			{ //Scenerio  2: Buffer allocated from Free List
				mprint.lock();
				cout << "\n\n\n\t****_____ Scenerio 2 :____**** \n\t\t ----> Buffer from header of  free list Allocated to " << pid << "(hash queue updated)";
				mprint.unlock();
				freeBuffer = freeList->removeFrom_fList_Head();
				if (freeBuffer->blockNo != 0)
				{
					hashQ[hashValue(freeBuffer->blockNo)]->removeSpecificBuf_hashQ(freeBuffer->blockNo);
				}
				hashQ[hashValue(blockNum)]->insertAt_hashQ_Tail(freeBuffer);
				updateBuffer(freeBuffer, blockNum);
				allocBuffer = freeBuffer;
				isfree[blockNum] = false;
				m1.unlock();
			}
		}
		if (freeList->isEmpty())
			isAvailable = false;
	}
	return allocBuffer;
}

void process(int pId);

void display();

int main()
{
	//Creating an Array of Hash Queue Lists
	for (int i = 0; i < NUMBEROFQUEUES; i++)
	{
		hashQ[i] = new doublyLinkList();
	}

	//Creating FreeList
	freeList = new doublyLinkList();

	//Initializing HashQueue and FreeList

	for (int i = 0; i < NUMBEROFBUFFERS; i++)
	{
		freeList->insertAtHead_fList(new buffer());
	}
	display(); //Initially FreeList and HashQueue

	thread t1(process, 1);
	thread t2(process, 2);
	sleep(1);
	thread t3(process, 3);
	sleep(1);
	thread t4(process, 4);

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	cout << "\nState of free list and hash after execution of process";
	display();
	return 0;
}

void process(int pId)
{
	int request = NUMBEROFREQUESTS;
	while (request > 0)
	{
		srand(time(0));
		int blockNum = rand() % (MAXBLOCKNO) + 1;
		{
			mprint.lock();
			cout << "\nProcess number  = " << pId << "  , requested block number =  " << blockNum;
			mprint.unlock();
		}
		buffer *block = getblk(blockNum, pId);
		if (block != NULL)
		{
			mprint.lock();
			updateBuffer(block, blockNum);
			cout << "\n\t\t\t --->  Block Number " << blockNum << " is allocated to Process " << pId << endl;
			mprint.unlock();
			sleep(2);
			brelse(block);
		}
		request--;
	}
}

void display()
{
	freeList->display_fList();
	for (int i = 0; i < NUMBEROFQUEUES; i++)
	{
		hashQ[i]->display_hashQ();
	}
}

