// 这个文件就当做扩展内容看看吧....
// 只是对3（find）操作进行了一下另外一种手法，目的是
// 把数据库中的数据用这个链表来表示，避免每一次增删改查都直接修改数据库，
// 那么在建立连接数据库时，就需要把数据库中的数据先全部用链表的表示，然后修改就是改链表


#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <ctime>
#include <cmath>

#include <vector>
#include <termios.h>
#include <sstream>
#include <string.h>
#include <assert.h>

#include <mysql.h>
using namespace std;


char strHost[] = "127.0.0.1";
char strUser[] = "root";
char strPasswd[] = "956040";
char strDb[] = "eating";
char strSQL[200];


class Client
{
public:
    string client_id;
    string client_name;
    string client_tele;
    string client_addre;
public:
    void show_client();
};


void Client::show_client()
{
    cout<<"The client ID is "<<client_id<<endl;
    cout<<"The client's name is "<<client_name<<endl;
    cout<<"The client's telephone number is "<<client_tele<<endl;
    cout<<"The client's address is "<<client_addre<<endl;
}

class ClientNode
{
public:
    Client client;
    ClientNode *next;
};

ClientNode *headclient = NULL;
int list_size = 0;

// 下面这个类的主要思想是这样的，它被实例化之后是一个对象，它里面保存了数据库的入口，就是mysql，然后也有result之类的
// 通过对实例化这个类的对象（我们记作ws）进行操作，自然就能在当前这个问题中，避免作用域的问题，
// 因为只有当整个程序或者主动释放ws时，里面的数据库的入口什么才关闭，就像~ClientManage()函数中那样..
class ClientManage
{
public:
    ClientManage();
    ~ClientManage();


    void addclient();
    void delclient(string s);
    void findclient(string s);

    void showAllData();
    void updateMysql();


private:
    // 为了保证作用域一致，把他们放在一个类中，这样就成了这个类的成员变量，然后当这个类实例化时，他们就成了某个对象的属性。
    MYSQL mysql; // 这个其实就是主入口
    MYSQL_RES * results;
    MYSQL_FIELD *fileds;
    MYSQL_ROW rows;

    void createList();
    void saveclient(ClientNode *p);
    void undoclient(ClientNode *p);
};

// 构造函数
ClientManage::ClientManage()
{
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,strHost,strUser,strPasswd,strDb,0,NULL,0))
    {
        cout<<"fail to connect dataset."<<endl;

    }else{
        cout << "init success." << endl;
        createList();
        showAllData();

        cout << "list_size = " << list_size << endl;
    }
}

// 析构函数
ClientManage::~ClientManage()
{
    if( &mysql != NULL )
    {
        mysql_close(&mysql);
    }

}

void ClientManage::createList()
{
    if(mysql_query(&mysql, "select * from client"))        //执行SQL语句
    {
        printf("Query failed (%s)\n",mysql_error(&mysql));

    }
    else
    {
        printf("query success\n");
    }

    if( !(results = mysql_store_result(&mysql) ))
    {
        printf("couldn't get result from %s\n",mysql_error(&mysql) );
    }

    int cols = mysql_field_count(&mysql); // 列
    int rows = mysql_affected_rows(&mysql); // 行

    //  cout << "rows = " << rows << ", cols = " << cols << endl;

    MYSQL_ROW line = NULL;
    int index = 0;
    while(index++ < rows )
    {
        line = mysql_fetch_row(results); // 这个操作获取每一行

        ClientNode *p = new ClientNode();

        p->client.client_id = line[0];
        p->client.client_name = line[1];
        p->client.client_tele = line[2];
        p->client.client_addre = line[3];
        p->next = headclient;
        headclient = p;

        list_size ++;
    }

}

void ClientManage::showAllData()
{
    //        cout << row << "  " << col << endl;

    cout << "==============================" << endl;
    if(mysql_query(&mysql, "select * from client"))        //执行SQL语句
    {
        printf("Query failed (%s)\n",mysql_error(&mysql));

    }
    else
    {
        printf("query success\n");
    }

    if( !(results = mysql_store_result(&mysql) ))
    {
        printf("couldn't get result from %s\n",mysql_error(&mysql) );
    }

    int cols = mysql_field_count(&mysql); // 列
    int rows = mysql_affected_rows(&mysql); // 行
    cout << "rows = " << rows << ", cols = " << cols << endl;

    MYSQL_ROW line = NULL;
    int index = 0;
    cout << "====== current database =======" << endl;
    while(index++ < rows )
    {
        line = mysql_fetch_row(results); // 这个操作获取每一行

        for(int tmp = 0; tmp < cols ; ++tmp) // 将每一列的内容展示出来
        {
            cout << line[tmp] << " ";
        }
        cout << endl;
    }

    cout << "=================================" << endl;
}

void ClientManage::addclient()
{
    string cid, cname, ctele, caddre;
    ClientNode *p = new ClientNode();

    cout<<"what's the client ID:"<<endl;cin>>cid;
    p->client.client_id = cid;
    cout<<"what's the client name:"<<endl;cin>>cname;
    p->client.client_name = cname;
    cout<<"what's the telephone number:"<<endl;cin>>ctele;
    p->client.client_tele = ctele;
    cout<<"what's the address:"<<endl;cin>>caddre;
    p->client.client_addre = caddre;

    p->next = headclient;
    headclient = p;
    saveclient(p);

    showAllData();

}

void ClientManage::delclient(string s)//delete
{

    ClientNode *list_start = headclient;

    ClientNode *list_next;

    if(list_start == NULL)
    {
      cout << "dataset is empty" << endl;
      return;
    }else{
      // 这边相当于有两个指针，放两个的原因是因为ClientNode里面只有一个next指针，即单项的，
      // 在进行数据删除操作时，假设删除1-2-3中的2，需要把1-3连接起来，对于单项链表就需要两个指针



        list_next = list_start->next;
        while(list_next != NULL)
        {
          //检测list_next
          if(list_next->client.client_id == s)
          {
            ClientNode *tmp = list_next;        // as for 1-2-3
            list_start->next = list_next->next; //start=1,next=2, 1-3,2-3
            list_next = list_next->next;        //start=1,next=3, 1-3
            delete(tmp);                        // delete 2
          }else{
            list_start = list_next;
            list_next = list_next->next;
          }
        }


      // 当原来检测过第一个是s，且删除时，在进入if(list_start != NULL),时候，由于是对list_next进行判别，这边少判别了list_start
      if(list_start->client.client_id == s)
      {
        ClientNode *tmp = list_start;
        list_start = list_start->next;
        delete(tmp); // 把tmp对应的地址空间删除，即原来list_start那个节点
      }


    }


}


void ClientManage::findclient(string s)//select
{
    // 这边只需要对headclient这个链表操作就可以了
    vector< vector<string>> data_target;
    vector< string> data_line;

    ClientNode *list_start = headclient;
    while(list_start != NULL)
    {

        if(list_start->client.client_id == s)
        {
            data_line.push_back(list_start->client.client_id);
            data_line.push_back(list_start->client.client_name);
            data_line.push_back(list_start->client.client_tele);
            data_line.push_back(list_start->client.client_addre);

            data_target.push_back(data_line);
            data_line.clear();
        }else{
//            cout << "- now " <<  list_start->client.client_id << endl;
        }
        list_start = list_start->next;
    }

    // 显示
    if(data_target.size() == 0)
    {
        cout << "cannot find " << s << endl;
    }else{
        for(int i = 0; i < data_target.size(); ++i)
        {
            cout << "#" << i+1 << ": " << data_target[i][0]
            << data_target[i][1]
            << data_target[i][2]
            << data_target[i][3] << endl;
        }
    }

}


void ClientManage::updateMysql()
{
    // 在这边对mysql中的数据库进行修改。
    // 但是这个我没有尝试了...说白了就是把当前headclient开始的链表中的数据传入数据库中....
    // 但是这边就涉及到如何处理数据库中原来的数据之间的关系，比如要怎么做才能保证高效之类的...


    // 这个文件就当做扩展内容看看吧....



}


void ClientManage::saveclient(ClientNode *p)
{
    memset((void*)strSQL,0,200);
    sprintf(strSQL,"insert into client values ('%s','%s','%s','%s')", p->client.client_id.c_str(), p->client.client_name.c_str(), p->client.client_tele.c_str(), p->client.client_addre.c_str());
    if(mysql_real_query(&mysql,strSQL,strlen(strSQL)) != 0)
    {
        cout<<"fail"<<endl;
    }else{
        cout << " add info success!" << endl;

    }
}

void ClientManage::undoclient(ClientNode *p)
{
    memset((void*)strSQL,0,200);
    sprintf(strSQL,"delete from client where client_id = '%s'",p->client.client_id.c_str());
    if(mysql_real_query(&mysql,strSQL,strlen(strSQL)) != 0)
    {
        cout<<"fail"<<endl;
    }

}





int main()
{

    ClientManage *clientmanage = new ClientManage();

    int x;

    while(1)
    {
      cout<<"press 1/2/3 to add/delete/find the client information. -1 to quit."<<endl;
      cin>>x;
      if( x == -1)
      {
        break;
      }
      switch(x)
      {
          case 1:
          {
              clientmanage->addclient();
              break;
          }
          case 2:
          {
            while(1)
            {
              cout<<"please enter the client ID, -1 to quit"<<endl;
              string a; cin>>a;
              if(a == -1)
              {
                break;
              }
              else{
                  clientmanage->delclient(a);
              }


            }

            updateMysql();
              break;
          }
          case 3:
          {
            while(1)
            {
              cout<<"please enter the client ID, -1 to quit."<<endl;
              string b;
              cin>>b;
              if(b == -1)
              {
                break;
              }else{
                clientmanage->findclient(b);
              }

            }

            updateMysql();

              break;
          }
          default:
              break;
      }

    }


}
