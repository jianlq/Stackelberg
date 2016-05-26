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
		double use,numDemand,dnknow;
		double dist,latency,load,capacity;
		CEdge(int i, int a, int b, double c,double d){
			id = i;
			tail = a;
			head = b;
			dist = c;
			capacity=d;
			load=0; 
			latency=0;
			use = numDemand = dnknow = 0;
		}

	CEdge(int a, int b, double c, double d)
	{
	   tail = a;
       head = b;
       dist = c;
       capacity = d;
    }
	
    CEdge()
    {
        head=NULL;
        tail=NULL;
        dist=0;
        capacity=0;
    }

	~CEdge(){}
	 double getWeight()
      {
	  return dist;
      }
	int getHead()
	{return head;}
	int getTail()
	{return tail;}
	double getCap()
	{return capacity;}
	bool operator<(CEdge& x)//���������
	{
	    if(dist < x.dist)
            return 1;
        else
            return 0;
    }
};

class demand{
public:
int org;
int des;
double flow;
demand(int a,int b ,double c)
{
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
	vector<int> ver;	// ���еĶ���
	vector<vector<CEdge*> > adjL, adjRL; //������ȱ�
	map<int, CVertex*> mapVID_Vertex;
	// ��ʱ��ǵĶ��㼯��
	list<CVertex*> listTempMark;
	 // ��¼�붥������ĳ��ȱ�
	map<int, vector<CEdge*> > mapVID_listEdge;
	///��¼��ʱ״ָ̬��
	list<Status*> listTemp;
	///��¼�����Ϊ���״̬
	list<Status*> listSolu;
	///��¼path����
	vector<CPath*> listPath; 
	void Update(int v);
	
	////////////��ʲ
	double TEcplex(vector<demand> &eq,CGraph *GOR);
	double ORcplex(vector<demand> *eqO); //��ʲ���Ŵ����õ�
	vector<demand> eqfo;

	//******* �Ŵ� ********//
	vector<int> pathnum;//ÿ��req��·����Ŀ
	vector<int> reqPathNo; //����req�ߵ�·�����
	vector<vector<CPath*> > reqlistPath;//����req��·������
	void GAinit(vector<demand>& req);//��ʼ��ÿ��req��·��
	vector<double> loadx; //ÿ�����ϸ���
	double GATEcplex(vector<demand> *eq);

	///////////////////����
	double mlu;
	double delay;

    public:
	CGraph(char* inputFile);
	int getNumVertex();
	int getNumEdge();

	double  DijkstraAlg(int s,int t);
	void KSP(int s, int t, unsigned int k);

	void clearOcc(){
	    for(int i=0;i<m;i++){
			Link[i]->load=0; 
		    Link[i]->latency=0;
		}
	}
	~CGraph(){
		for(unsigned int i = 0; i < Link.size(); i++)
				if(!Link[i])
					delete Link[i];
		}
};

int CGraph::getNumVertex()
{   return n;}
int CGraph::getNumEdge()
{  return m;}

CGraph::CGraph(char* inputFile)
{
	ifstream file(inputFile);
	file >> n >> m;
	adjL.resize(n*n); 
	adjRL.resize(n*n); 
	set<int> vert;
	int a, b; double d;double c;
	for (int i = 0; i < m; i++)
	{
		file >> a >> b >> c >> d;
		loadx.push_back(0);
		vert.insert(a);
		vert.insert(b);
		int wt;
        wt = rand()%MAXWEIGHT+1;
		CEdge *e=new CEdge(i,a,b,wt,d);
		Link.push_back(e);
		adjL[a].push_back(e); //////////////���ȱ�
		adjRL[b].push_back(e); //////////// ��ȱ�
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
	for(i=vert.begin();i!=vert.end();i++)
	{ ver.push_back(*i);}
   
}

double CGraph::GATEcplex(vector<demand> *eq){
    IloEnv env;
    IloModel model(env);
    IloCplex GATEcplex(model);
	int totalnum=(*eq).size();
    IloArray<IloIntVarArray> x(env, totalnum);	
    for(int d=0; d < totalnum; d++)
		x[d] = IloIntVarArray(env, this->m, 0, 1);	

    //Լ��1  �����غ�Լ��  ��ÿ������������غ�Լ��
    for(int d = 0; d < totalnum; d++)
		for( int i = 0; i < this->n; i++){    
			IloExpr constraint(env);
			for(unsigned int k = 0; k <adjL[i].size(); k++) // ���ȱ�
				constraint += x[d][adjL[i][k]->id];
			for(unsigned int k = 0; k <adjRL[i].size(); k++) // ��ȱ�
				constraint -= x[d][adjRL[i][k]->id];	

			if(i == (*eq)[d].org) 
				model.add(constraint == 1);
			else if(i == (*eq)[d].des)
				model.add(constraint == -1);
			else
				model.add(constraint == 0);
		}

  for(int i = 0; i < this->m; i++){
		IloExpr constraint(env);
		for(int d = 0; d <  totalnum; d++)
			constraint += (*eq)[d].flow*x[d][i];
		model.add(constraint<=(Link[i]->capacity-Link[i]->load));  ///// 2016-1-5
		//cout<<Link[i]->load<"  ";
  }
//�Ż�Ŀ�� ��С������������  	
	IloNumVar z(env,0,1);
	for(int i = 0; i < this->m; i++){
		IloExpr load(env);
		for(int d = 0; d < totalnum; d++)
		  load += (*eq)[d].flow*x[d][i];
		model.add(load<=z*(Link[i]->capacity-Link[i]->load));	 ///// 2016-1-6
	}
	model.add(IloMinimize(env,z));

	GATEcplex.setOut(env.getNullStream());
	double obj = INF;
	GATEcplex.solve();
	if(GATEcplex.getStatus() == IloAlgorithm::Infeasible)
		 env.out() << "GATE No Solution" << endl;
	else{
	   loadx.clear();
	   obj=0;
	   for(int ij=0;ij<this->m;ij++){
	    double load=0;
		for(int d=0;d<totalnum;d++)
			load += GATEcplex.getValue(x[d][ij])*(*eq)[d].flow; //link�ϵĸ���	
		if(obj<(load+Link[ij]->load)/(Link[ij]->capacity)) obj=(load+Link[ij]->load)/(Link[ij]->capacity);
		loadx.push_back(load+Link[ij]->load);
	}	
  }  
   env.end();
   return obj;
}

double CGraph::DijkstraAlg(int s,int t)
{
    map<int, CVertex*>::iterator i,iend;
    iend = mapVID_Vertex.end();
    for( i=mapVID_Vertex.begin(); i != iend; i++)
    {
		i->second->d=INFINITY; //////////����ʼ��Ϊ����
        if ( i->second->ID == s)
            i->second->d = 0;
        listTempMark.push_back(i->second);
    }
    while( !listTempMark.empty() )
    {
        listTempMark.sort(pVertexComp);
        int j = (*listTempMark.begin())->ID;
        listTempMark.pop_front();
        Update(j);
    }
	return mapVID_Vertex[t]->d;
}

void CGraph::Update(int v)
{
    vector<CEdge*> lEdge = mapVID_listEdge[v];
    vector<CEdge*>::iterator i,iend;
    iend = lEdge.end();
    for( i = lEdge.begin(); i!=iend; i++)
    {
        double w = (*i)->getWeight();
        CVertex* h = mapVID_Vertex[(*i)->getHead()];
        CVertex* t = mapVID_Vertex[v];
		//cout<<"h -d"<<h->d<<" ";
        if ( t->d + w < h->d )
        {   h->d = t->d + w;
            h->p = v;
        }
    }
}

#endif