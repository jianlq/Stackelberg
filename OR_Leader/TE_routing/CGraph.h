#ifndef CGRAPH_H
#define CGRAPH_H
#include"Common.h"
#include <ilcplex/ilocplex.h>
#define INFINITY 10000.0

class CVertex{
    public:
	double d;
	int p;
	int ID;
	CVertex(){d = INFINITY; p = NULL;}
	CVertex(int i){ID=i;d = INFINITY; p = NULL;}
	~CVertex();
};
bool pVertexComp ( CVertex* x, CVertex* y )
{    if ( x->d < y->d ) return 1;
	return 0;
};

class Status{
    public:
    int ID;
    double d;
    list<int> passby;
    Status* parent;

    Status(){ID = NULL; d = INFINITY ; };
    Status(int id, double w){ID = id; d = w ; passby.clear();};
    Status(int id, double w, Status* p){ID = id; d = w ; parent = p; passby.clear();};
    Status(int id, double w, Status* p, list<int> listVID){ID = id; d = w ; parent = p; passby = listVID;};

    ~Status(){;};
};
bool pStatusComp ( Status* x, Status* y )
{    if ( x->d < y->d ) return 1;
	return 0;
};

class CEdge{
   public:
	   int  id, tail,head;
	   double use,dnknow;
	   double dist,latency,load,capacity;
	   CEdge() {}
	   CEdge(int i, int a, int b, double c,double d){
		   id = i;
		   tail = a;
		   head = b;
		   dist = c;
		   capacity=d;
		   load=0; 
		   latency=0.01;
		   use  = dnknow = 0;
	   }
	   ~CEdge(){}
	   double getWeight(){
		   return dist;
	   }
	   int getHead(){
		   return head;
	   }
	   int getTail(){
		   return tail;
	   }
	   double getCap(){
		   return capacity;
	   }
};

class demand{
public:
	int org;
	int des;
	double flow;
	demand(int a,int b ,double c){
		org=a;
		des=b;
		flow=c;
	}
};

class CPath{
    public:
    double length;
    vector<CEdge*> listEdge;
    list<CVertex*> listVertex;

    CPath(){length = 0; listEdge.clear(); listVertex.clear();};
    CPath(Status *beginning, Status *ending, map<int, CVertex*> mapVID_Vertex, map<int, vector<CEdge*> > mapVID_listEdge);
    ~CPath(){;};
};
bool pPathComp ( CPath* x, CPath* y )
{    if ( x->length < y->length ) return 1;
	return 0;
};
CPath::CPath(Status *beginning, Status *ending, map<int, CVertex*> mapVID_Vertex,map<int, vector<CEdge*> > mapVID_listEdge)
{
    length = ending->d;
    Status *status = ending;
    while(1)
    {
        int id = status->ID;
        //cout<<id<<"  ";
        listVertex.push_front(mapVID_Vertex[id]);
        if(status == beginning)
            break;
        status = status->parent;
       vector<CEdge*>::iterator it;
        for(it = mapVID_listEdge[status->ID].begin(); it != mapVID_listEdge[status->ID].end(); it++)
        {
            if((**it).getHead() == id)
                listEdge.push_back(*it);
        }
    }
}

class CGraph{
   public:
	int n, m;
	vector<CEdge*> Link;
	vector<int> ver;	// 所有的顶点
	vector<vector<CEdge*> > adjL, adjRL; //出、入度边
	map<int, CVertex*> mapVID_Vertex;
	map<int, vector<CEdge*> > mapVID_listEdge; // 记录与顶点关联的出度边	
	list<Status*> listTemp;///记录临时状态指针	
	list<Status*> listSolu;///记录被标记为解的状态
	vector<CPath*> listPath; ///记录path集合

	vector<vector<int> > reqPathID;
	double dijkstra(int id,int s, int t, double dm ,bool needpath,bool hop);
	double dijkstraOR(int s, int t);
	double LoadScaleFactor(vector<demand> &req);

	////////////纳什
	double TEcplex(vector<demand> &eq,CGraph *GOR);
	double ORcplex(vector<demand> *eqO); //纳什和遗传都用到
	vector<demand> eqfo;

	//******* 遗传 ********//
	vector<int> pathnum;//每个req的路径数目
	vector<int> reqPathNo; //所有req走的路径编号
	vector<vector<CPath*> > reqlistPath;//所有req的路径集合
	void GAinit(vector<demand>& req);//初始化每个req的路径	

	////独裁
	double mlu;
	double delay;

    public:
	CGraph(char* inputFile);
	void KSP(int s, int t, unsigned int k);

	void clearOcc(){
	    for(int i=0;i<m;i++){
			Link[i]->load=0; 
			Link[i]->use = Link[i]->dnknow = 0;
			Link[i]->latency = 0.01;
		}
	}
	~CGraph(){
		for(unsigned int i = 0; i < Link.size(); i++)
				if(!Link[i])
					delete Link[i];
		}
};

CGraph::CGraph(char* inputFile)
{
	ifstream file(inputFile);
	file >> n >> m;
	adjL.resize(n*n); 
	adjRL.resize(n*n); 
	reqPathID.resize(200);
	set<int> vert;
	int a, b; double d;double c;
	for (int i = 0; i < m; i++)
	{
		file >> a >> b >> c >> d;
		vert.insert(a);
		vert.insert(b);
		int wt;
        wt = rand()%MAXWEIGHT+1;
		CEdge *e=new CEdge(i,a,b,wt,d+MINCAPACITY);
		Link.push_back(e);
		adjL[a].push_back(e); //////////////出度边
		adjRL[b].push_back(e); //////////// 入度边
	}
	file.close();	
    vector<CEdge*> emptylist;
    vector<CEdge*>::iterator it;
    for(it=Link.begin(); it!=Link.end(); it++)
    {
        mapVID_Vertex.insert(pair<int,CVertex*>((**it).getTail(),new CVertex((**it).getTail())));
        mapVID_Vertex.insert(pair<int,CVertex*>((**it).getHead(),new CVertex((**it).getHead())));
        mapVID_listEdge.insert(pair<int,vector<CEdge*>>((**it).getTail(),emptylist));
        mapVID_listEdge.insert(pair<int,vector<CEdge*>>((**it).getHead(),emptylist));
        mapVID_listEdge[(**it).getTail()].push_back(*it);
    }
    set<int>::iterator i;
	for(i=vert.begin();i!=vert.end();i++){ 
		ver.push_back(*i);
	}
   
}

double CGraph::dijkstra(int id,int s, int t, double dm,bool needpath,bool hop){
	vector<int> p, flag;
	vector<double> d;//带宽利用率
	for(int i = 0; i < n; i++){
		p.push_back(-1);
		flag.push_back(0);
		d.push_back(INF);
	}
	d[s] = 0;
	int cur = s;
	do{
		flag[cur] = 1;
		for(unsigned int i = 0; i < adjL[cur].size(); i++){
			CEdge *e = adjL[cur][i];
			if(hop){
				if(e->capacity - e->use >= dm && d[e->head] > d[e->tail] + 1){
					d[e->head] = d[e->tail] + 1;
					p[e->head] = e->id;
				}
			}
			else{
			if(CONSTANT){
				if(e->capacity - e->use >= dm && d[e->head] > d[e->tail] + e->dist){
					d[e->head] = d[e->tail] + e->dist;
					p[e->head] = e->id;
				}
			}
			else{
				double util = ( dm + e->use )/e->capacity; // 带宽利用率最小
				double tail_util = max(util,d[e->tail]);
				if(e->capacity - e->use >= dm && d[e->head] > tail_util ){
					d[e->head] = tail_util;
					p[e->head] = e->id;
				}
			}
		  }
		}
		cur = -1;
		for(int i = 0; i < n; i++)
			if(!flag[i] && (cur == -1 || d[cur] > d[i] ))
				cur = i;
	}while(cur != -1);

	cur = t;
	do{
		if(p[cur] == -1)
			break;
		Link[p[cur]]->use += dm;
		cur = Link[p[cur]]->tail;
	}while(cur != s);

	reqPathID[id].clear();
	if(needpath){
		cur = t;
		do{
			if(p[cur] == -1)
				break;
			this->reqPathID[id].push_back(p[cur]);
			cur = Link[p[cur]]->tail;
		}while(cur != s);
		reverse(reqPathID[id].begin(),reqPathID[id].end());
	}
	return d[t];
}

double CGraph::dijkstraOR(int s, int t){
	vector<int> p, flag;
	vector<double> d;//latency
	for(int i = 0; i < n*n; i++){  // graph OR n是顶点个数和顶点编号可能为其他大于n的数
		p.push_back(-1);
		flag.push_back(0);
		d.push_back(INF);
	}
	d[s] = 0;
	int cur = s;
	do{
		flag[cur] = 1;
		for(unsigned int i = 0; i < adjL[cur].size(); i++){
			CEdge *e = adjL[cur][i];
			if(CONSTANT){  //dist
				if( d[e->head] > d[e->tail] + e->dist){
					d[e->head] = d[e->tail] + e->dist;
					p[e->head] = e->id;
				}
			}
			else{   ////latency				
				if( d[e->head] > d[e->tail] + e->latency ){
					d[e->head] = d[e->tail] + e->latency;
					p[e->head] = e->id;
				}
			}
		}
		cur = -1;
		for(unsigned int i = 0; i < this->ver.size(); i++)
			if(!flag[ver[i]] && (cur == -1 || d[cur] > d[ver[i]] ))
				cur = ver[i];
	}while(cur != -1);

	cur = t;
	do{
		if(p[cur] == -1)
			break;
		cur = Link[p[cur]]->tail;
	}while(cur != s);

	return d[t];
}

double CGraph::LoadScaleFactor(vector<demand> &req){
	int num = req.size();
	this->clearOcc(); 
	for(int d = 0 ;d < num; ++d){
		this->dijkstra(d,req[d].org,req[d].des,req[d].flow,0,1);
	}
	double util = 0;
	for(int i = 0; i < this->m; i++){
		double xc = this->Link[i]->use/this->Link[i]->capacity;
		if(util < xc )
			util = xc;
	}
	return util;
}

#endif