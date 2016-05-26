#ifndef EVOLUTIONBIT_H
#define EVOLUTIONBIT_H
#include"KSP.h"

// 个体 individual
class evoluDivbit{
	private:
		static const int MUT = 5; //变异位数
		static const int HER = 20; //学习位数
		int consider;
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem; 
		vector<demand> *demor; 
	public:
		vector<vector<int>> x;//解  各个req经过的路径编号 二进制编码 req*3
		double ability; //个体能力
		double delay,mlu;//个体能力值的体现
		double mlubase;
		// 构造函数
		evoluDivbit() {;}
		double GAabilityNoCplex();
		double GAability();
		void Caldelay(vector<demand> &eq);
		void Init(){
			delay = INF;
			mlu = INF;
			ability = MIN;
		}
		//m为req的数量
		evoluDivbit(int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor,double mlubest, int con){
			Init();
			x.resize(m);//
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			mlubase=mlubest;
			consider = con;
			randNature(); 
		}
		evoluDivbit(vector<vector<int> > &tx, CGraph *g,CGraph *gor, vector<demand> *d, vector<demand> *dor,double mlubest,int con){
			Init();
			x.clear();
			G = g;
			GOR=gor;
			dem = d;
			demor=dor;
			mlubase=mlubest;
			consider = con;
			for(unsigned int i = 0; i < tx.size(); i++)
				x.push_back(tx[i]);
		}
		//////////////交配杂交  crossover
		evoluDivbit mate(evoluDivbit other){
			vector<vector<int> > nx;	
			/*
			//分成两截互换 在num处截断
			int n=dem->size();
			int num=rand()%(n/2);
			for(int i=0;i<num;i++) 
				nx.push_back(x[i]);
			for(unsigned int ij=num;ij<x.size();ij++)
			   nx.push_back(other.x[ij]);	
			return evoluDivbit(nx, G,GOR, dem,demor);	
			*/
			//多点交叉  随机产生一串0-1序列  为0不交叉  为1交叉
			vector<vector<int> > onezero;
			for(unsigned int i=0;i<x.size();i++){
				vector<int> bit;
				for(int j=0;j<3;j++)   
					bit.push_back(rand()%2);
				onezero.push_back(bit); 
			}
			for(unsigned int n=0;n<x.size();n++)
				nx.push_back(x[n]);
			for(unsigned int i=0;i<onezero.size();i++)
					for(int j=0;j<3;j++){  
						if(onezero[i][j]==1)
							nx[i][j]=other.x[i][j];
					}
					return evoluDivbit(nx, G,GOR, dem,demor,other.mlubase,other.consider);	
		}
		//将二进制转为十进制  路径编号的具体值
		int decoding(vector<int> &bit){
			 int z=0;
		     z = bit[0]*1+bit[1]*2+bit[2]*4;
			 return z;
		}
		// *********  计算能力  *********//
		void calAbility(){
			GOR->reqPathNo.clear(); ////   GOR  Overlay as a leader
			for(unsigned int i=0;i<x.size();i++) 
				GOR->reqPathNo.push_back(decoding(x[i])); 
			ability = GAabilityNoCplex();	
		}

		void randNature(){
			for(unsigned int i = 0; i < x.size(); i++){
				vector<int> bit;
				for(int b=0;b<3;b++)        
					bit.push_back(rand()%2);
				x[i] = bit; 
			}
		}

		void mutation(){	
			int i=0;
			while(i<MUT){
			int row= rand()% demor->size();
			int col=rand()%3;
			if(x[row][col]==0) x[row][col]=1;
			else if(x[row][col]==1) x[row][col]=0; 
			i++;
			}
		}

		/////////学习历史最优解
		void culture(evoluDivbit hero){
		int num=rand()%HER+1;int n=0;
			while(n<num){
              int j=rand()%x.size();
			for(unsigned int i = 0; i < x.size(); i++){
				if(i==j) x[i] =  hero.x[i];
			}
			n++;
		}	
	}
};

 bool Cmp2(evoluDivbit a, evoluDivbit b){
	return a.ability > b.ability; //ability和适应度成反比，ability越小（最小化问题），适应度越大
  }

//种群 种群由个体组成
class evoluPopubit{
	private:
		static const int YEAR = 50;
		static const int YEARCUL = 40;
		static const int NOHERO = 50;
	    double pm;
		vector<evoluDivbit> popu;
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem;
		vector<demand> *demor;
		FILE *herofile2;
	public:
		evoluDivbit hero;
		// n 种群大小  m:ORreq数目 初始化种群
		evoluPopubit(int n, int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor,double delaybest,int con){
			popu.clear();
			pm=0.15;
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			for(int i = 0; i < n; i++){
				evoluDivbit divi(m, G, GOR,dem,demor,delaybest,con);
				popu.push_back(divi);
			}
			hero = evoluDivbit(m, G,GOR, dem,demor,delaybest,con);
			herofile2=fopen("outputFile//hero.txt","a");
		}
		///////////  轮盘赌和
		int wheelchose(double sum){
			double m=0;
			double r=rand()%1000*0.001;
			int res = rand()%popu.size();
			for(unsigned int i=0;i<popu.size();i++){
				m += popu[i].ability/sum;
				if(r<=m) return i;
			}
			return res;
		}
		/////////// 种群进化  下一代 //////////////////
		evoluDivbit evolution(){
			int nohero=0;//记录进化情况
			fprintf(herofile2,"Start:\n");
			vector<int> h0;
			for(int b=0;b<3;b++)   
				h0.push_back(0);
			for(unsigned int i = 0; i < hero.x.size(); i++)
				hero.x[i] = h0;
			hero.calAbility();
			//cout<< " year 0  hero " <<  hero.ability <<"  "<< hero.delay << "   "<< hero.mlu<<endl;
			//评价每个个体的能力
			for(unsigned int i = 0; i < popu.size(); i++)
				popu[i].calAbility();
			sort(popu.begin(), popu.end(), Cmp2);//按个体优劣排列以进行强强交配
			//繁殖的代数
			for(int curYear = 1; curYear <= YEAR; curYear++){		
			int n = popu.size(), getMore = 0;
			//轮盘赌和选择n个个体
			double sum=0;
		    for( int i=0;i<n;i++)
			    sum += popu[i].ability;
			vector<evoluDivbit> chosepopu;
			for(int i=0;i<n;i++) //n种群大小
			{
				int ch=wheelchose(sum);
				//cout<<"ch "<<ch<<" ";
				chosepopu.push_back(popu[ch]);
			}
			popu.clear();
			for(unsigned int i=0;i<chosepopu.size();i++)
				popu.push_back(chosepopu[i]);

			vector<evoluDivbit> sons;//子孙
			sons.clear();
			//杂交
			for(unsigned int i = 0; i+1 < popu.size(); i+=2){
				sons.push_back(popu[i].mate(popu[i+1]));
				sons.push_back(popu[i+1].mate(popu[i]));
				sons.push_back(popu[i].mate(popu[i+1]));
			}
			//  sons.push_back(hero);
			int m = sons.size();//子孙的个数
			for(int i = 0; i < m; i++){
				double p=rand()%100*0.01;
				if(p<pm) 
					sons[i].mutation();//个体变异以概率pm
				if(curYear > YEARCUL)
					sons[i].culture(hero); //向hero靠近
				sons[i].calAbility();
			}
			sort(sons.begin(), sons.end(), Cmp2);
			popu.clear(); //清空上一代种群
			//printf("Year %d\n ", curYear);
			for(int i = 0; i < n; i++){
				//printf("%.6f ", sons[i].ability);
				popu.push_back(sons[i]); //保持种群数量不变为n,将能力排在前n的放入下一代种群
				if(sons[i].ability > hero.ability){
					hero = sons[i]; //选出能力最好的作为hero
					getMore = 1;
				}
			}
			if(getMore){
				//printf("%f\t%f\t%f\n",hero.ability,hero.mlu,hero.delay);
				fprintf(herofile2, "Year %d: find hero \n", curYear);
				fprintf(herofile2,"%f\t%f\n", hero.mlu,hero.delay);
			}
			else nohero++;
			if(nohero> NOHERO){
				break;
			}
		}
			fprintf(herofile2,"\n\n");
			fclose(herofile2);
			return hero;
		}
};

void evoluDivbit::Caldelay(vector<demand>&eq){
	for(int ij=0;ij<G->m;ij++){
		if(G->Link[ij]->use>=G->Link[ij]->capacity) G->Link[ij]->latency = 1.0;
		else G->Link[ij]->latency=1.0/(G->Link[ij]->capacity-G->Link[ij]->use);  //传参数为单位延迟  
	}
	int totalnum = eq.size();
	for(int m=0;m<GOR->m;m++){	//计算OR边延迟
		bool flag=false;
		double dmin=0;
		for(int d=0;d<totalnum;d++){
			if(GOR->Link[m]->tail==eq[d].org && GOR->Link[m]->head==eq[d].des && (eq[d].flow>0) ){ 
				if(flag==false){
					flag=true;
					//cout<<G->reqPathID[d].size()<<"  ";
					for(unsigned int ij=0;ij<G->reqPathID[d].size();ij++){	
						dmin += G->Link[G->reqPathID[d][ij]]->latency;
					}
				}
			}
			if(flag==true){
				GOR->Link[m]->latency=dmin;	
				break;
			}
		} 
		
	} 

}

// OR在Overlay层未进行选路  在网络中以带宽利用率最小来进行选路  得到的带宽利用率和延迟
pair<double,double> Calability(CGraph *G,CGraph *GOR,vector<demand> &req){
	pair<double,double> res(INF,INF);
	G->clearOcc();
	double mlu = 0;
	int block = 0,success = 0;
	for(unsigned int i = 0; i < req.size(); i++){
		double ret = G->dijkstra(i,req[i].org, req[i].des, req[i].flow,1,0); //以带宽利用率最小来进行选路
		if(ret >= INF)
			block++;
		else{
			success++;
			if(mlu<ret)
				mlu = ret;
			//cout<< result <<"  ";
		}
	}

	double teobj = INF,orobj = INF,ilp = 0;
	if(success == req.size()){
		teobj = mlu;
		///// calculate delay
		for(int ij=0;ij<G->m;ij++){
			if(G->Link[ij]->use>=G->Link[ij]->capacity) 
				G->Link[ij]->latency = 1.0;
			else 
				G->Link[ij]->latency=1.0/(G->Link[ij]->capacity-G->Link[ij]->use);  //传参数为单位延迟  
	}

	int totalnum = req.size();
	for(int m = 0; m < GOR->m; m++){	//计算OR边延迟
		bool flag=false;
		double dmin=0;
		for(int d = 0; d < totalnum; d++){
			if(GOR->Link[m]->tail==req[d].org && GOR->Link[m]->head==req[d].des && (req[d].flow>0) ){ 
				if(flag==false){
					flag=true;
					//cout<<G->reqPathID[d].size()<<"  ";
					for(unsigned int ij=0;ij<G->reqPathID[d].size();ij++){	
						dmin += G->Link[G->reqPathID[d][ij]]->latency;
					}
				}
			}
			if(flag==true){
				GOR->Link[m]->latency=dmin;
				break;
			}
		} 		
	} 

	vector<demand> reqor;
	for(int i = 0;i < GOR->m ; ++i)
		reqor.push_back(req[i]);
	//orobj = GOR->ORcplex(&reqor); 
	double del = 0;
	for(unsigned int d = 0; d < reqor.size(); d++){
		del += GOR->dijkstraOR(reqor[d].org,reqor[d].des)*reqor[d].flow; //以延迟为权重的dijkstra
	}
    orobj = del ;
	}  // end of if
	res.first = teobj;
	res.second = orobj;
	return res;
}


//评价函数 知道每个req走哪条路之后（reqPathNo)  计算te的反应，反应完之后OR的目标值delay
double evoluDivbit::GAability(){
	//计算每条边上的负载
	this->GOR->clearOcc(); //清空每条边上负载 
	for (unsigned int d = 0; d<GOR->reqPathNo.size(); d++){
		int num = GOR->reqPathNo[d];
		//cout<<" d   "<<d<<"  "<<"num   "<<num<<"   ";
		vector<CEdge*> lsEg = GOR->reqlistPath[d][num]->listEdge;
		vector<CEdge*>::iterator it, end = lsEg.end();
		for (it = lsEg.begin(); it != end; it++){
			(*it)->load += (*demor)[d].flow;
		}
	}
	vector<demand> eq;
	for (int i = 0; i<GOR->m; i++)
		eq.push_back(demand(GOR->Link[i]->tail, GOR->Link[i]->head, GOR->Link[i]->load));
	int k = demor->size();//同GOR边一样多
	for (unsigned int d = k; d<dem->size(); d++) //dem的后面存eqTE
		eq.push_back((*dem)[d]);
	G->clearOcc();
	double teobj = G->TEcplex(eq, GOR);
	double orobj = GOR->ORcplex(demor);
	double ability;
	if (this->consider == 0)  ability = 1.0 / orobj; ///// 不考虑TE
	else if (this->consider == 1)
		ability = this->mlubase / teobj + 1.0 / orobj;
	else
		ability = this->mlubase / teobj + 1.0 / orobj;
	this->delay = orobj;//目标
	this->mlu = teobj;//目标

	return ability;
}

double evoluDivbit::GAabilityNoCplex(){ 
	this->GOR->clearOcc(); //清空每条边上负载 
	for(unsigned int d=0;d<GOR->reqPathNo.size();d++){
		int num=GOR->reqPathNo[d]; 
		//cout<<" d   "<<d<<"  "<<"num   "<<num<<"   ";
		vector<CEdge*> lsEg=GOR->reqlistPath[d][num]->listEdge;
		vector<CEdge*>::iterator it,end=lsEg.end();
		for(it=lsEg.begin();it!=end;it++){
			(*it)->load += (*demor)[d].flow;
		}
	}
	vector<demand> eq;
	for(int i=0;i<GOR->m;i++){
		if(GOR->Link[i]->load>0)
			eq.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,GOR->Link[i]->load));
	}
	int k = demor->size();
	for(unsigned int d = k;d<dem->size();d++) 
		eq.push_back((*dem)[d]);

	G->clearOcc();
	double result = 0;
	int block = 0,success = 0;
	for(unsigned int i = 0; i < eq.size(); i++){
		double ret = G->dijkstra(i,eq[i].org, eq[i].des, eq[i].flow,1,0);
		if(ret >= INF)
			block++;
		else{
			success++;
			if(result<ret)
				result = ret;
		}
	}
	double teobj = INF,orobj = INF,ilp = 0;
	double ab = MIN;
	if(success == eq.size()){
		teobj = result;	
		Caldelay(eq);	
		double del = 0;
		for(unsigned int d = 0; d < demor->size(); d++)
			del += GOR->dijkstraOR((*demor)[d].org, (*demor)[d].des)*(*demor)[d].flow;
		orobj = del ;	
		if(this->consider==0)  ab = 1.0/orobj; ///// 不考虑TE  
		else if(this->consider==1)
			ab= 1.2*this->mlubase/teobj+1.0/orobj;  ///// 考虑TE
		this->delay = orobj;//目标
		this->mlu = teobj;//目标
	}	
	return ab;
}

#endif
