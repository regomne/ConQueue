// ConQueue.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ConcurrentQueue.h"

using namespace std;

int cnt=0;

struct Foo
{
    int i;
    Foo(){i=cnt++; cout<<"Foo cstr..."<<i<<endl;}
    ~Foo(){cout<<"Foo decstr..."<<i<<endl;}
};

struct strt
{
	ConcurrentQueue<int>* q;
	int start;
	int dis;
};

DWORD WINAPI thProc(LPARAM lp)
{
	strt* s=(strt*)lp;
	printf("starting %d\n",s->start);
	for(int i=s->start;i<1000000;i+=s->dis)
	{
		s->q->Enqueue(i);
	}
	printf("over %d\n",s->start);
	return 0;
}

DWORD WINAPI thProc2(LPARAM lp)
{
    auto q=(ConcurrentQueue<shared_ptr<Foo>>*)lp;
    shared_ptr<Foo> foo;
    q->Dequeue(&foo);
    q->Dequeue(&foo);
    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{

	ConcurrentQueue<int> q;
	strt a={&q,0,5};
	strt b={&q,1,5};
	strt c={&q,2,5};
	strt d={&q,3,5};
	strt e={&q,4,5};
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)thProc,&a,0,0);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)thProc,&b,0,0);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)thProc,&c,0,0);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)thProc,&d,0,0);
	CreateThread(0,0,(LPTHREAD_START_ROUTINE)thProc,&e,0,0);

	bool* ha=new bool[1000000];
	memset(ha,false,1000000);
	int i,cnt=0;
	while(true)
	{
		if(q.Dequeue(&i))
		{
			if(ha[i])
			{
				printf("error!\n");
				break;
			}
			ha[i]=true;
			if(i>5 && !ha[i-5])
			{
				printf("error2\n");
				break;
			}
			if(i==1000000-1)
			{
				printf("Waiting %d\n",cnt);
				break;
			}
		}
		else
		{
			cnt++;
			Sleep(1);
		}
	}
    {
    shared_ptr<Foo> foo(new Foo());
    shared_ptr<Foo> foo2(new Foo());
    ConcurrentQueue<shared_ptr<Foo>> q;
    q.Enqueue(foo);
    q.Enqueue(foo2);
    auto thread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)thProc2,(LPVOID)&q,0,0);
    WaitForSingleObject(thread,-1);
    }
    getch();
	return 0;
}

