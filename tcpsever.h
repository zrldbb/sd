#ifndef TCPSEVER_H
#define TCPSEVER_H
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstdlib>
#include<event.h>
#include<map>
#include<unordered_map>
#include<unordered_set>
#include<vector>
#include<stdlib.h>

/*
#define INSERTDB_MAX 100000
#define CUT_SIZE 1000
*/

#define INSERTDB_MAX 10000
#define CUT_SIZE 100

using namespace std;
class Tcpsever
{
	public:
		Tcpsever();
		~Tcpsever();
		void run();
		struct event_base* get_base();
		int get_pth_num();

	private:
		void create_socket_pair();
		void create_pth();
		int _listen_fd;//listenfd
		int _pth_num;//线程个数
		vector<vector<int>> _sockpair_vec;//socketpair
		struct event_base* _base;//libevent
		unordered_map<int,int> _sock1_sock0;

	friend void listen_cb(int fd,short event,void* arg);
	friend void sock_0_cb(int fd,short event,void* arg);
};

struct base_sock1{
	struct event_base* _base;
	int sock1;	
};

#endif 
