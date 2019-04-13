#include <pthread.h>
#include "stdlib.h"
#include "linked_list.hpp"
#include "wait_list.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "MyThreadPool.h"

class MsgStruct {
public:
	MsgStruct():m_text(""),m_time(""),m_user_name(""){}
	MsgStruct(string usrname,string tm,string text):m_text(text),m_time(tm),m_user_name(usrname) {}
private:
	string m_text;
	string m_time;
	string m_user_name;
};

class InsertWorker : public Worker{
	public:
		InsertWorker(unordered_set<MsgStruct*>& msg) : _msg(msg) {}
		~InsertWorker(){}
	public:
		int doWork() {
			/*插入完后delete指针指向的堆内存*/
		}
	private:
		unordered_set<MsgStruct*> _msg;
};
