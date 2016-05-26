#ifndef EVOLUTIONBIT_H
#define EVOLUTIONBIT_H
#include"KSP.h"

// 个体 individual
class evoluDivbit{
	private:
		static const int MUT = 5; //变异位数
		static const int HER = 15; //学习位数
		int consider;
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem; 
		vector<demand> *demor; 
	public:
		vector<vector<int>> x;//解  各个req经过的路径编号 二进制编码 req*4
		double ability; //个体能力
		double delay, mlu;//个体能力值的体现
		double delaybase;
		// 构造函数
		evoluDivbit() {;}
		double GAability();
		double GAabilityNoCplex();
		void Caldelay();
		void Init(){
			ability = MIN;
			delay = 0;
			mlu = 0;
		}
		//m为req的数量
		evoluDivbit(int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor,double delaybest, int con){
			Init();
			x.resize(m);//
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			delaybase=delaybest;
			consider = con;
			randNature(); 
		}

		evoluDivbit(vector<vector<int> > &tx, CGraph *g,CGraph *gor, vector<demand> *d, vector<demand> *dor,double delaybest,int con){
			Init();
			x.clear();
			G = g;
			GOR=gor;
			dem = d;
			demor=dor;
			delaybase=delaybest;
			consider = con;
			for(unsigned int i = 0; i < tx.size(); i++)
				x.push_back(tx[i]);
			 // 也可以用  x = tx ; 代替上面的操作
		}

		/////////////交配杂交  crossover
		evoluDivbit crossover(evoluDivbit other){
			vector<vector<int> > nx;	
			/*
			//分成两截互换 在num处截断  即是单点杂交
			int n=dem->size();
			int num=rand()%(n/2);
			for(int i=0;i<num;i++) 
				nx.push_back(x[i]);
			for(unsigned int ij=num;ij<x.size();ij++)
			   nx.push_back(other.x[ij]);	
			return evoluDivbit(nx, G,GOR, dem,demor);	
			*/			
			vector<vector<int> > onezero;   //多点交叉  随机产生一串0-1序列  为0不交叉  为1交叉
			for(unsigned int i=0;i<x.size();i++){
				vector<int> bit;
				for(int j=0;j<4;j++)  
					bit.push_back(rand()%2);
				onezero.push_back(bit); 
			}
			for(unsigned int n=0;n<x.size();n++)
				nx.push_back(x[n]);
			for(unsigned int i=0;i<onezero.size();i++)
					for(int j=0;j<4;j++){  
						if(onezero[i][j]==1)
							nx[i][j]=other.x[i][j];
					}
					return evoluDivbit(nx, G,GOR, dem,demor,other.delaybase,other.consider);	
		}

		//将二进制转为十进制  路径编号的具体值
		int Decoding(vector<int> &bit){
			 int z=0;
			 z = bit[0]*1+bit[1]*2+bit[2]*4+bit[2]*8;//// 16条路
			 //z = bit[0]*1+bit[1]*2+bit[2]*4;//// 8条路
			 return z;
		}

		// *********  计算能力  *********//
		void calAbility(){
			G->reqPathNo.clear(); //清空上一个个体解的路径编号
			for(unsigned int i=0;i<x.size();i++) 
				G->reqPathNo.push_back(Decoding(x[i])); 
			ability = GAabilityNoCplex();	
		}

		void randNature(){
			for(unsigned int i = 0; i < x.size(); i++){
				vector<int> bit;
				for(int b=0;b<4;b++)        ///////////////////////   16 ： 4      8 ： 
					bit.push_back(rand()%2);// 4位编码 随机产生0或1  表示路径编号
				x[i] = bit; 
			}
		}

		void mutation(){	//变异
			int i=0;
			while(i<MUT){
			int row= rand()% dem->size();//产生需要变异的req路径编号
			int col=rand()%4;//产生需要变异的位置   ///////////////////////   16 ： 4      8 ： 3
			if(x[row][col]==0) x[row][col]=1;
			else if(x[row][col]==1) x[row][col]=0; 
			i++;
			}
		}

		/////////学习历史最优解
		void culture(evoluDivbit hero){
			int n=0;
			while(n<HER){
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
		static const int YEAR = 100;
		static const int YEARCUL = 80;
		static const int NOHERO = 60;
	    double pm;
		vector<evoluDivbit> popu;
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem;
		vector<demand> *demor;
		FILE *herofile;
	public:
		evoluDivbit hero;
		// n 种群大小  m:req数目 初始化种群
		evoluPopubit(int n, int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor,double delaybest,int con){
			popu.clear();
			pm = 0.20;
			G = g;
			GOR = gor;
			dem = d;
			demor = dor;
			for(int i = 0; i < n; i++){
				evoluDivbit divi(m, G, GOR,dem,demor,delaybest,con);
				popu.push_back(divi);
			}
			hero = evoluDivbit(m, G,GOR, dem,demor,delaybest,con);
			herofile = fopen("outputFile//hero.txt","a");
		}
		///////////  轮盘赌和
		int wheelchose(double sum){
			double m=0;
			double r=rand()%1000*0.001;
			int res = rand()%popu.size();
			for(unsigned int i=0;i<popu.size();i++){
				m += popu[i].ability/sum;
				if(r<=m) {
				//	cout << i << "   ";
					return i;
				}
			}
			return res; //如果for循环中没有返回   // 解决 ：“evoluPopubit::wheelchose”: 不是所有的控件路径都返回
		}

		/////////// 种群进化 //////////////////
		evoluDivbit evolution(){
		int nohero=0;//记录进化情况
		fprintf(herofile,"Start:\n ");
			vector<int> h0;
			for(int b=0;b<4;b++)  
				h0.push_back(0);
			for(unsigned int i = 0; i < hero.x.size(); i++)
				hero.x[i] = h0;//开始的hero设为每个req都走各自路径集合的第0条路
			hero.calAbility();
			if(hero.ability>MIN) fprintf(herofile, "%f\t%f\t%f\n", hero.mlu,hero.delay,hero.ability);
			//评价每个个体的能力
			for(unsigned int i = 0; i < popu.size(); i++)
				popu[i].calAbility();
			//sort(popu.begin(), popu.end(), Cmp2);//按个体优劣排列以进行强强交配
			//繁殖的代数
			for(int curYear = 1; curYear <= YEAR; curYear++){
			//轮盘赌和选择n个个体
			int n = popu.size(), getMore = 0;
			double sum=0;
		    for( int i=0;i<n;i++)
			    sum += popu[i].ability;
			vector<evoluDivbit> chosepopu;
			chosepopu.clear();
			//cout<< " ch   " <<endl;
			for(int i=0;i<n;i++) //n种群大小
			{
				int ch=wheelchose(sum);
				// cout<<"  "<<ch<<"  ";
				chosepopu.push_back(popu[ch]);
			}
			popu.clear();
			for(unsigned int i=0;i<chosepopu.size();i++)
				popu.push_back(chosepopu[i]);
			vector<evoluDivbit> sons;//子孙
			sons.clear();
			//杂交
			for(unsigned int i = 0; i+1 < popu.size(); i+=2){
				sons.push_back(popu[i].crossover(popu[i+1]));
				sons.push_back(popu[i+1].crossover(popu[i]));
				sons.push_back(popu[i].crossover(popu[i+1]));
			}
			   // sons.push_back(hero);  //精英保留策略
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
					if(abs(sons[i].ability - hero.ability) < 1e-4 ){	//判断相等
						//cout<< sons[i].ability <<" ====== "<< hero.ability;
						continue;
					}
					else if(sons[i].ability > hero.ability){
						//cout<< sons[i].ability <<"  >>>>>>  "<< hero.ability;
						hero = sons[i]; //选出能力最好的作为hero
						getMore = 1;
					}
				}
				if(getMore){
					fprintf(herofile, "Year %d: find hero \n", curYear);
					fprintf(herofile,"%f\t%f\t%f\n", hero.mlu,hero.delay,hero.ability);
				}
				else nohero++;
				if(nohero> NOHERO){
					break;
				}
			}
			fprintf(herofile,"end\n\n\n\n");
			fclose(herofile);
			return hero;
		}

};

//////////////////************** 评价函数 类（evoluDiv)个体的成员函数*************************////////////////////////
//知道每个req走哪条路之后（reqPathNo)  计算te的反应（反应完之后的mlu），反应完之后OR的目标值delay
double evoluDivbit::GAability(){ 
	//计算每条边上的负载
	this->G->clearOcc(); //清空每条边上负载 
	for(unsigned int d=0;d<G->reqPathNo.size();d++)
	{
		int num=G->reqPathNo[d]; 
		vector<CEdge*> lsEg=G->reqlistPath[d][num]->listEdge;
		//cout<< lsEg.size() << "   ";
		//cout << endl;
		vector<CEdge*>::iterator it,end=lsEg.end();
		for(it=lsEg.begin();it!=end;it++){
			(*it)->load += (*dem)[d].flow;
		}
	}
	//double te=0; // 按这个路由方案 Overlay还没反应之前TE的链路利用率
	//for(int i=0;i<G->m;i++){
	//	if(te<G->Link[i]->load/G->Link[i]->capacity)
	//		te=G->Link[i]->load/G->Link[i]->capacity;
	//}
	////if(te>=1) { cout<<endl<<"return 1 bit "<<endl;return MIN;}
	//double or=0; //按这个路由方案Overlay的延迟
	////cout<<"te < 1  "<< te <<"    and demor size is "<<demor->size()<<endl;
	//for(int i=0;i<demor->size();++i){   
	//	
	//	int num=G->reqPathNo[i]; 
	//	vector<CEdge*> lsEg=G->reqlistPath[i][num]->listEdge;
	//	vector<CEdge*>::iterator it,end=lsEg.end();
	//	for(it=lsEg.begin();it!=end;it++){
	//		or += (*demor)[i].flow/( (*it)->capacity-(*it)->load ); // x1/(C-x)
	//	}
	//	// cout<<i<<" "<< or << endl;
	//}
	GOR->clearOcc();
	if(!CONSTANT){  //非常数  //计算Overlay的边上延迟
	 for(int m=0;m<GOR->m;m++){	
			 bool flag=false;
			 double dmin=0;//延迟
			for(unsigned int d=0;d<(*dem).size();d++){
				if(GOR->Link[m]->tail==(*dem)[d].org && GOR->Link[m]->head==(*dem)[d].des){ //OR边在需求中
					if(flag==false){
					   	flag=true;
						//则看这个req经过哪条路径   把路径上的延迟加起来
						vector<CEdge*> lg=G->reqlistPath[d][G->reqPathNo[d]]->listEdge; 
						for(vector<CEdge*>::iterator lgi=lg.begin();lgi!=lg.end();lgi++){ 		
							if((*lgi)->load>=(*lgi)->capacity) dmin += 1.0;				
							else dmin += (double)1.0/(double)((*lgi)->capacity-(*lgi)->load);//!!!!!!!!!!double	          
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
else{  // CONSTANT
  for(int m=0;m<GOR->m;m++){	
			 bool flag=false;
			 double dmin=0;//延迟
			for(unsigned int d=0;d<(*dem).size();d++){
				if(GOR->Link[m]->tail==(*dem)[d].org && GOR->Link[m]->head==(*dem)[d].des){ //OR边在需求中
					if(flag==false){
					   	flag=true;
						//则看这个req经过哪条路径   把路径上的延迟加起来
						vector<CEdge*> lg=G->reqlistPath[d][G->reqPathNo[d]]->listEdge; 
						for(vector<CEdge*>::iterator lgi=lg.begin();lgi!=lg.end();lgi++){ 
							if((*lgi)->load>=(*lgi)->capacity) dmin += 5.0;
							else dmin += (*lgi)->dist;		          
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

//for(int i = 0; i<GOR->m;i++)
//	cout<< GOR->Link[i]->latency << "   ";
double orobj=GOR->ORcplex(demor); 

	//OR决策之后TE的决策  eqfo  
	 vector<demand> req2;
	 int num1=GOR->eqfo.size();
	 int num2=(*dem).size();
	 for(int d=0;d<num1;++d) 
		 req2.push_back(GOR->eqfo[d]); 
	 int k = (*demor).size();
	 for(int d = k; d < num2; ++d)
		 req2.push_back((*dem)[d]);
	//this->G->clearOcc(); //清空每条边上负载 
	//for(unsigned int d=0;d<req2.size();++d){   // overlay需求装在前面
	//	int num=G->reqPathNo[d]; //cout<<num<<" ";
	//	vector<CEdge*> lsEg=G->reqlistPath[d][num]->listEdge;
	//	vector<CEdge*>::iterator it,end=lsEg.end();
	//	for(it=lsEg.begin();it!=end;++it){
	//		(*it)->load += req2[d].flow;
	//	}
	//}
	//double teobj=0; //如果下层路由不变 则TE的目标值
	//for(int i=0;i<G->m;i++){
	//	if(teobj<G->Link[i]->load/G->Link[i]->capacity)
	//		teobj=G->Link[i]->load/G->Link[i]->capacity;
	//}

	double teobj2 = G->TEcplex(req2,GOR,0);
	if(teobj2>=INF){ cout<<" return te2  "<<endl;return MIN;}
	//if(teobj>=1) { cout<<"return te "<<endl;return MIN;}	
	double obj = MIN;
	// te是当前决策给TE带来的目标值   teobj是考虑到Overlay决策后对TE的影响
	if(this->consider==0)   obj = 1.0/teobj2; ///// 不考虑overlay
	else if(this->consider==1)
		obj= 1.0/teobj2+1.2*delaybase/orobj;  // consider = 1 ,考虑overlay 
	else
		obj= 1.0/teobj2+1.5*delaybase/orobj;
	this->delay=orobj;//目标
	this->mlu = teobj2;//目标

	return obj;
}

// 计算能力值的时候不用cplex
double evoluDivbit::GAabilityNoCplex(){ 
	//计算每条边上的负载
	this->G->clearOcc(); //清空每条边上负载 
	for(unsigned int d=0;d<G->reqPathNo.size();d++)
	{
		int num=G->reqPathNo[d]; 
		vector<CEdge*> lsEg=G->reqlistPath[d][num]->listEdge;
		vector<CEdge*>::iterator it,end=lsEg.end();
		for(it=lsEg.begin();it!=end;it++){
			(*it)->use += (*dem)[d].flow;
		}
	}
	// 计算链路延迟
	this->GOR->clearOcc();
	Caldelay();
	//Overlay 计算路由
	double del = 0;
	for (unsigned int d = 0; d < demor->size(); d++)
		del += GOR->dijkstraOR((*demor)[d].org, (*demor)[d].des, (*demor)[d].flow)*(*demor)[d].flow;
	
	//double del2 = GOR->ORcplex(demor);
	////cout<< del << "  delay compare    " << del2<< "    ";  // 对比和线性规划的目标,是一样的
	//cout<<endl;
	//for(int i =0;i<GOR->m;i++)
	//	cout << GOR->Link[i]->use<< "   ";
	//cout<<endl;
	//for( int d =0;d<GOR->eqfo.size();d++)
	//	cout<< GOR->eqfo[d].flow<< "   ";
	
	// 将GOR->Link->use 加到reqTE中得到新的流量矩阵计算TE的目标
	vector<demand> req;
	for (int i = 0; i < GOR->m; i++){
		if(GOR->Link[i]->use > 0 )
		req.push_back(demand(GOR->Link[i]->tail, GOR->Link[i]->head, GOR->Link[i]->use));
	}
	for (unsigned int i = demor->size(); i < dem->size(); i++){
		req.push_back((*dem)[i]);
	}
	// 计算TE的目标值
	G->clearOcc(); //清空link上的use
	double result = 0;
	int block = 0, success = 0;
	int n = req.size();
	for (int i = 0; i < n; i++){
		double ret = 0;
		if(req[i].flow>0) 
			ret = G->dijkstra(i,req[i].org, req[i].des, req[i].flow,1,0);		
		if ((INF - ret) < 1e-4)  // ret == INF
			block++;
		else{
			success++;
			if (result<ret)
				result = ret;
		}
	}
	//cout<< "  " << n << "  "<<success<< "   ";
	double ab = MIN;
	if (success >= n){
		this->mlu = result;
		this->delay = del;
		//cout << result << "   " << del <<"     "<<endl;
		if (this->consider == 0)  ab = 1.0 / result; ///// 不考虑OR
		else if (this->consider == 1)
			ab = 1.2/result + 1.0*this->delaybase / del;  ///// 考虑OR
		else
			ab = 1.0 / result + 1.2*this->delaybase / del;
	}
	return ab;
}


void evoluDivbit::Caldelay(){
	for(int ij=0;ij<G->m;ij++){
		if(G->Link[ij]->use>=G->Link[ij]->capacity) G->Link[ij]->latency = 1.0;
		else G->Link[ij]->latency=1.0/(G->Link[ij]->capacity-G->Link[ij]->use);  //传参数为单位延迟  
	}
	int totalnum = (*dem).size();
	for(int m=0;m<GOR->m;m++){	//计算OR边延迟
		bool flag=false;
		double dmin=0;
		for(int d=0;d<totalnum;d++){
			if (GOR->Link[m]->tail == (*dem)[d].org && GOR->Link[m]->head == (*dem)[d].des && ((*dem)[d].flow>0)){
				if(flag==false){
					flag=true;
					//则看这个req经过哪条路径   把路径上的延迟加起来
					vector<CEdge*> lg=G->reqlistPath[d][G->reqPathNo[d]]->listEdge; 
					for(vector<CEdge*>::iterator lgi=lg.begin();lgi!=lg.end();lgi++){ 		
						if((*lgi)->load>=(*lgi)->capacity) dmin += 1.0;				
						else dmin += (double)1.0/(double)((*lgi)->capacity-(*lgi)->load);//!!!!!!!!!!double	          
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


//   在网络中以带宽利用率最小来进行选路  得到的带宽利用率和延迟  OR在Overlay层未进行选路
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
	double ability = 0.1;
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
		del += GOR->dijkstraOR(reqor[d].org,reqor[d].des,reqor[d].flow)*reqor[d].flow; //以延迟为权重的dijkstra
	}
    orobj = del ;
	}  // end of if
	res.first = teobj;
	res.second = orobj;
	return res;
}


#endif
