#include<pthread.h>
#include<iostream>
#include<limits.h>
#include<string.h>
#include<time.h>
#include"tcpsever.h"
#include"InsertWorker.h"
#include"MyThreadPool.h"
using namespace std;


pthread_t tidp;
pthread_mutex_t local_mutex;
pthread_mutex_t mutex_3;
if ( 0 != pthread_mutex_init(&local_mutex,NULL)) cout << "mutex init failed" << endl;
if ( 0 != pthread_mutex_init(&mutex_3,NULL)) cout << "mutex init failed" << endl;

_threadPool = new MyThreadPool;
_threadPool->open(2, 204800000);
_threadPool->activate();

/* 
1各线程的负载量 sock1读出clifd时子线程自己修改
3所有MsgStruct先被插入的位置 */
static unordered_map<int,int> local_pth2num; //1
static unordered_set<MsgStruct*> local_sub_msg_set; //3 mutex_3

Tcpsever::Tcpsever()
{
	int fd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == fd)
	{
		cerr<<"fd creat fail;errno:"<<endl;
		return;
	}

	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(7878);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bind(fd,(struct sockaddr*)&saddr,sizeof(saddr));
	listen(fd,20);
	_listen_fd = fd;

	_pth_num = 4;

	_base = event_base_new();
	if (_base != NULL) cout << "Ser event_base init success" << endl;
	else {
		cout << "event_base init error" << endl;
		exit(0);
	}

}

Tcpsever::~Tcpsever() {
}

struct event_base* Tcpsever::get_base(){
	return _base;
}

int Tcpsever::get_pth_num(){
	return _pth_num; 
}

void listen_cb(int fd,short event,void* arg) {
	cout << " call listen_cb " << endl;
	Tcpsever * mthis = (Tcpsever *)arg;
	//listen_fd发生事件 即有客户端前来连接 
	struct sockaddr_in c_addr; 	
	socklen_t len = sizeof(c_addr);
	int cli_fd = accept(fd,(struct sockaddr*)&c_addr,&len);
	if (-1 == cli_fd) {
		cerr << "accept cli_fd failed!" << endl;
		return;
	}

	//将客户端套接字发送给子线程处理
	//查询获得监听量最少的子线程
	int min = INT_MAX;
	int min_sock_1 = local_pth2num.begin()->first;
	for(map<int,int>::iterator it = local_pth2num.begin(); it != local_pth2num.end(); ++it) {
		if (it->second < min) {
			min = it->second;
			min_sock_1 = it->first;
		}
	}
	int min_sock_0 = mthis->_sock1_sock0[min_sock_1];
	cout << "write sock0:"<<min_sock_0<<endl;
	//写双端队列与子线程通信 
	char buff[1024] = {0};
	sprintf(buff,"%d#",cli_fd);
	if (-1 == write(min_sock_0,buff,1024)) {
		cerr<<"write cli_fd to sock_0 failed!"<<endl;
		return;
	}
	return;
}

void Tcpsever::run()
{
	//创建socketpair
	create_socket_pair();

	//启动线程
	create_pth();

	//将监听套接子libevent
	struct event* listen_event = event_new(_base,_listen_fd,EV_READ|EV_PERSIST,listen_cb,(void *)this);
	if(NULL == listen_event)
	{
		cerr<<"event new fail;errno:"<<errno<<endl;
		return;
	}

	event_add(listen_event,NULL);
	
	event_base_dispatch(_base);   //while(1){ epoll_wait();}
}



void sock_0_cb(int fd,short event,void* arg) {
	return;
}

void Tcpsever::create_socket_pair() {
	//申请
	for(int i = 0; i < _pth_num; i++)
	{
		//创建父进程与子线程通信的双向管道socketpair
		int buff[2] = {0};
		if (-1 == socketpair(AF_UNIX,SOCK_STREAM,0,buff)) {
			cerr << "create socketpair error!" << endl;
			return;
		}

		//存放多组socketpair
		vector<int> tmp = {buff[1],buff[0]};
		cout << "pth: "<< i << " sock0 : " << buff[0] << " sock1 : " << buff[1] << endl;
		// vector idx0:1端 idx1:0端
		_sockpair_vec.push_back(tmp);
		// 初始化各线程监听量都为0
		local_pth2num[buff[1]] = 0;
		_sock1_sock0[buff[1]] = buff[0];

		//将socketpair0端加入到libevent
		struct event* sock_0_event = event_new(_base,buff[0],EV_READ|EV_PERSIST,sock_0_cb,(void *)this);
		if(NULL == sock_0_event)
		{
			cerr<<"create sock_0_event failed!"<<errno<<endl;
			return;
		}

		event_add(sock_0_event,NULL);
	}
	return;
}

string get_time() {
	time_t tm;
	time (&tm);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&tm));
	return tmp;
}

void cli_cb(int fd,short event,void* arg)
{
	/* 假设收到的信息是 user_name#test#time 
	   new msg_struct insert(+mutex) p to global set */
	
	string tm = get_time();
	cout << tm << " cli event arrive : " << fd << endl;
	//buff->contral     
	char buff[1024] = {0};
	while(0 < read(fd,buff,1024))
	{
		/* 处理buff 按#分开 new MsgStruct对象放入local_sub_msg_set */
		MsgStruct* msg = new MsgStruct( , , );
		pthread_mutex_lock(&mutex_3);
		local_sub_msg_set.insert(MsgStruct);
		pthread_mutex_unlock(&mutex_3);

		if(-1 == write(fd,"ok",3))
		{
			cerr<<"send fail;errno:"<<errno<<endl;
			return;
		}
	}

	//将set分割为 (INSERTDB_MAX/CUT_SIZE) 个set 作为单位工作线程的插入量
	//线程同步问题
	pthread_mutex_lock(&local_mutex,NOWAIT);
	if (local_sub_msg_set.size() == INSERTDB_MAX) {
		pthread_mutex_lock(&mutex_3);
		auto photo_set = local_sub_msg_set;
		local_sub_msg_set.clear();
		pthread_mutex_unlock(&mutex_3);
		unordered_set<MsgStruct *> tmp_set;
		int i = 0;
		int f = 0;
		for(auto it = photo_set.begin(); it != photo_set.end(); ++it) {
			tmp_set.insert(*it);
			if ( i == CUT_SIZE or ( i == ( photo_set.size() - (f*CUTSIZE) ) ) ) {
				InsertWorker* tWorker = new InsertWorker(tmp_set);
				jobs.push_back(dynamic_cast<Worker*>(tWorker));
				_threadPool->add_worker(dynamic_cast<Worker*>(tWorker));
				tmp_set.clear();
				i = 0;
				f++;
			}
		}
	}
	pthread_mutex_unlock(&local_mutex);
	return;
}

void sock_1_cb(int fd,short event,void *arg)
{
	cout << " sock1 event arrive : " << fd << endl;
	struct base_sock1* my_base_sock1 = (struct base_sock1*)arg;

	//recv   cli_fd
	char buff[1024];
	if (-1 == read(fd,buff,1024)) {
		cerr<<"sock_1 read cli_fd failed!"<<endl;
		return;
	}
	int i = 0;
	for(; i < 1024; i++) {
		if ( buff[i] == '#' ) break;	
	}
	char n_buff[i];
	for(int j = 0; j < i; j++) n_buff[j] = buff[j];
	int cli_fd = atoi(n_buff);
	/*
		EV_TIMEOUT: 超时
		EV_READ: 只要网络缓冲中还有数据，回调函数就会被触发
		EV_WRITE: 只要塞给网络缓冲的数据被写完，回调函数就会被触发
		EV_SIGNAL: POSIX信号量
		EV_PERSIST: 不指定这个属性的话，回调函数被触发后事件会被删除
		EV_ET: Edge-Trigger边缘触发，相当于EPOLL的ET模式
	*/
	struct event* cli_event = event_new(my_base_sock1->_base,cli_fd,EV_READ|EV_ET,cli_cb,(void *)my_base_sock1->_base);
	cout << " pth(sock1) begin to listen cli: " << cli_fd << endl;

	event_add(cli_event,NULL);

	local_pth2num[my_base_sock1->sock1] += 1;

	//sprintf(buff,"%lu#",local_pth2num[my_base_sock1->sock1]);
	//write(fd,buff,1024);

	event_base_dispatch(my_base_sock1->_base);

	return;
}

void* pth_run(void *arg)
{

	struct event_base* _base = event_base_new();//libevent
	struct base_sock1 my_base_sock1;
	my_base_sock1._base = _base;
	int * p_sock1 = (int *)arg;
	my_base_sock1.sock1 = *p_sock1;
	struct event* sock_1_event = event_new (_base,*p_sock1,EV_READ|EV_PERSIST,sock_1_cb,(void *)&my_base_sock1); 
	cout << " pth begin to listen sock1: " << *p_sock1 << endl;
	event_add(sock_1_event,NULL);
	event_base_dispatch(_base);

	return NULL;
}

void Tcpsever::create_pth() {
	for(int i = 0; i < _pth_num; i++) {
		//0 是1端 1是0端
		int * p_sock1 = &_sockpair_vec[i][0];
		int ret = pthread_create(&tidp,NULL,pth_run,(void *)p_sock1);
	}
	return;
}
