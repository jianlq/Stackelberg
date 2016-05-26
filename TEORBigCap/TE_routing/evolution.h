#ifndef EVOLUTION_H
#define EVOLUTION_H
#include"KSP.h"

// 个体 individual
class evoluDiv{
	private:
		static const int MAT = 5; //交叉位数
		static const int MUT = 4; //变异位数
		static const int HER = 30; //学习位数
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem; //
		vector<demand> *demor; //
	public:
		vector<int> x;//解  各个req经过的路径编号
		double ability; //个体能力
		double delay,mlu1,mlu,euler;//个体能力值的体现
		// 构造函数
		evoluDiv() {;}
		double GAability();
		//m为req的数量
		evoluDiv(int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor){
			x.resize(m);
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			randNature(); ////////
		}
		evoluDiv(vector<int> &tx, CGraph *g,CGraph *gor, vector<demand> *d, vector<demand> *dor){
			x.clear();
			G = g;
			GOR=gor;
			dem = d;
			demor=dor;
			for(unsigned int i = 0; i < tx.size(); i++)
				x.push_back(tx[i]);
		}
		//////////////交配杂交  crossover
		evoluDiv mate(evoluDiv other){
			vector<int> nx;
			int n=dem->size();
			int num=rand()%MAT+1;
			/*
			//分成两截互换 在num处截断  效果差
			for(int i=0;i<num;i++) 
				nx.push_back(x[i]);
			for(unsigned int ij=num;ij<x.size();ij++)
			   nx.push_back(other.x[ij]);	
			return evoluDiv(nx, G,GOR, dem,demor);
			*/
			//可以随机选num位互换
			for(unsigned int i=0;i<x.size();i++)
				nx.push_back(x[i]);
			int j=0;
			while(j<num)
			{
               int i=rand()%n;
			   nx[i]=other.x[i];
			   j++;
			}
			return evoluDiv(nx, G,GOR, dem,demor);
			
		}
		// *********  计算能力  *********//
		void calAbility(){
			ability = 0;
			G->reqPathNo.clear(); //清空上一个个体解的路径编号
			for(unsigned int i=0;i<x.size();i++)  ////////////////////////////////////// 
				G->reqPathNo.push_back(x[i]);
			ability=max(ability,GAability());	
		}
		void randNature(){
			for(unsigned int i = 0; i < x.size(); i++){
				x[i] = rand() % G->pathnum[i]; //pathnum[i]表示req的路径数目
			}
		}
		void mutation(){	
				//选择几位突变
				int num= rand()% MUT+1;
				for(int i=0;i<num;i++)
				{
					int j=rand()%G->pathnum.size();
					int pa;
					do{
						pa=rand()%G->pathnum[i];
					}while(pa==x[j]);
					x[j]=pa;
				}	
		}
		/////////靠近最优解  随机选HER位相同  选的位置又是随机x数组下标
		void culture(evoluDiv hero){
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

bool evoluCmp(evoluDiv a, evoluDiv b){
	return a.ability > b.ability; //ability越大，适应度越大
}

//种群 种群由个体组成
class evoluPopu{
	private:
		static const int YEAR =100;
		static const int YEARCUL = 60;
		static const int NOHERO = 40;
	    double pc,pm;
		vector<evoluDiv> popu;
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem;
		vector<demand> *demor;
		FILE *herofile;
		
	public:
		evoluDiv hero;
		// n 种群大小  m:req数目 初始化种群
		evoluPopu(int n, int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor){
			popu.clear();
			pc=0.90;
			pm=0.35;
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			for(int i = 0; i < n; i++){
				evoluDiv divi(m, G, GOR,dem,demor);
				popu.push_back(divi);
			}
			hero = evoluDiv(m, G,GOR, dem,demor);
			herofile=fopen("outputFile//hero.txt","a");
		}

		int wheelchose(double sum){
			double m=0;
			double r=rand()%1000*0.001;
			for(unsigned int i=0;i<popu.size();i++){
				m += popu[i].ability/sum;
				if(r<=m) return i;
			}
		}
		//种群进化  下一代
		evoluDiv evolution(){
			int nohero=0;
			fprintf(herofile,"Start:\n");
			for(unsigned int i = 0; i < hero.x.size(); i++)
				hero.x[i] = 0;//开始的hero设为每个req都走各自路径集合的第0条路
			hero.calAbility();
			if(hero.ability>MIN) fprintf(herofile, "%f\t%f\t%f\n", hero.mlu1,hero.mlu,hero.delay);
			//评价每个个体的能力
			for(unsigned int i = 0; i < popu.size(); i++)
				popu[i].calAbility();		
			//2016-1-5 sort(popu.begin(), popu.end(), evoluCmp);//按个体优劣排列以进行强强交配
			//繁殖的代数
			for(int curYear = 1; curYear <= YEAR; curYear++){
			//轮盘赌和选择n个个体
			int n = popu.size(), getMore = 0;
			double sum=0;
		    for(int i=0;i<n;i++)
			    sum += popu[i].ability;
			vector<evoluDiv> chosepopu;
			chosepopu.clear();
			for(int i=0;i<n;i++) //n种群大小
			{
				int ch=wheelchose(sum);
				//cout<<"ch "<<ch<<" ";
				chosepopu.push_back(popu[ch]);
			}
			popu.clear();
			for(unsigned int i=0;i<chosepopu.size();i++)
				popu.push_back(chosepopu[i]);
			vector<evoluDiv> sons;//子孙
			sons.clear();
			//杂交
			for(unsigned int i = 0; i+1 < popu.size(); i+=2){
			   double p=rand()%100*0.01 ;
			   if(p>pc){  //pc 杂交概率
						sons.push_back(popu[i]);
						sons.push_back(popu[i+1]);
					}
				else{
						sons.push_back(popu[i].mate(popu[i+1]));
					    sons.push_back(popu[i+1].mate(popu[i]));
					    sons.push_back(popu[i].mate(popu[i+1]));
					}
				}
			    sons.push_back(hero); /////精英保留  2016-1-14  可能会导致局部最优 很快收敛  可通过适当的变异操作
				int m = sons.size();//子孙的个数
				for(int i = 0; i < m; i++){
					//double p=rand()%100*0.01;
					//if(p<pm)       //个体变异以概率pm
					sons[i].mutation();
					if(curYear > YEARCUL)
						sons[i].culture(hero); //向hero靠近
					sons[i].calAbility();
				}
				sort(sons.begin(), sons.end(), evoluCmp);
				popu.clear(); //清空上一代种群
				//printf("Year %d\n",curYear);
				for(int i = 0; i < n; i++){
					//printf("%.6f ",sons[i].ability);
					popu.push_back(sons[i]); //保持种群数量不变为n,将能力排在前n的放入下一代种群
					if(sons[i].ability > hero.ability){
						hero = sons[i]; //选出能力最好的作为hero
						getMore = 1;
					}
				}
				if(getMore){
					fprintf(herofile, "Year %d: find hero \n", curYear);
					fprintf(herofile,"%f\t%f\t%f\n", hero.mlu1,hero.mlu,hero.delay);
				}
				else nohero++;
				if(nohero>NOHERO) break;
				//printf("\n%f\t%f\n",hero.mlu,hero.delay);
			}
			fprintf(herofile,"final hero\n%f\t%f\t%f\n", hero.mlu1,hero.mlu,hero.delay);
			fprintf(herofile,"Ending\n\n\n");
			fclose(herofile);
			return hero;
		}
		
		
};
////  计算每个 OD对之间的路径   初始化路径数目等参数
void CGraph::GAinit(vector<demand> &req){
	pathnum.clear();//这次req的每个OD对的路径数目
	reqlistPath.clear();
 for(unsigned int i=0;i<req.size();i++)
 {  
	 KSP(req[i].org,req[i].des,K); //计算得到一个OD对的listPath
	 reqlistPath.push_back(listPath); //将listPath存到reqlistPath对应位置   vector<vector<CPath*> > 
	 pathnum.push_back(listPath.size());
	// cout<<pathnum[i]<<" "<<endl;
	
 }
 //// 输出测试
 // vector<vector<CPath*>>::iterator it;
 // for(it = reqlistPath.begin();it != reqlistPath.end();it++){
	//  for(int k = 0; k < 16; k++){
	//  vector<CEdge*>::iterator i;
	//  cout << k << " : ";
	//  reverse((*it)[k]->listEdge.begin(),(*it)[k]->listEdge.end());
	//  for( i = (*it)[k]->listEdge.begin(); i!= (*it)[k]->listEdge.end();i++){
	//	     cout << (*i)->tail << "  " << (*i)->head <<"  ";
	//  }
	//  cout<<endl;
 // }
 // }
 // exit(0);

}
//////////////////************** 评价函数 类（evoluDiv)个体的成员函数*************************////////////////////////
//知道每个req走哪条路之后（reqPathNo)  计算OR的反应（反应完之后的延迟），反应完之后TE的目标值
double evoluDiv::GAability(){ 
	//计算每条边上的负载
	this->G->clearOcc(); //清空每条边上负载 
	//cout<<" 111111111111111 ";
	for(unsigned int d=0;d<G->reqPathNo.size();d++)
	{
		int num=G->reqPathNo[d];  // cout<<num<<" ";
		vector<CEdge*> lsEg=G->reqlistPath[d][num]->listEdge;
		vector<CEdge*>::iterator it,end=lsEg.end();
		for(it=lsEg.begin();it!=end;it++){
			(*it)->load += (*dem)[d].flow;
		}
	}
	double te=0;
	for(int i=0;i<G->m;i++){
		if(te<G->Link[i]->load/G->Link[i]->capacity)
			te=G->Link[i]->load/G->Link[i]->capacity;
	}
 //if(te>=1) { cout<<endl<<"return 1 "<<endl;return MIN;}

 //计算Overlay的边上延迟
 if(!CONSTANT){
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
							else dmin += (double)1.0/(double)((*lgi)->capacity-(*lgi)->load);//!!!!!!!!!!double  1/(C-x+1)
						}		          
				     }
			      }	
			if(flag==true){ 
				GOR->Link[m]->dist=dmin;	
				break;
			}	
        }
    }
 }
 else{
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
							if((*lgi)->load>=(*lgi)->capacity) dmin += 100.0;
							else dmin += (*lgi)->dist;		          
				      }
			      }
				}
			 if(flag==true){    
				 GOR->Link[m]->dist=dmin;
				 break;
			}
         }
	 }
 }
double orobj=GOR->ORcplex(demor); 
	//OR决策之后TE的决策  eqfo  
	 vector<demand> req2;
	 req2.clear();
	 int num1=GOR->eqfo.size();
	 int num2=(*dem).size();
	 for(int d=0;d<num1;++d) 
		 req2.push_back(GOR->eqfo[d]); 
	 for(int d=num1;d<num2;++d)
		 req2.push_back((*dem)[d]);

	this->G->clearOcc(); //清空每条边上负载 
	//cout<<" 222222222222222";
	for(unsigned int d=0;d<req2.size();++d)
	{
		int num=G->reqPathNo[d];//cout<<num<<" ";
		vector<CEdge*> lsEg=G->reqlistPath[d][num]->listEdge;
		vector<CEdge*>::iterator it,end=lsEg.end();
		for(it=lsEg.begin();it!=end;++it){
			(*it)->load += req2[d].flow;
		}
	}
	double teobj=0;
	for(int i=0;i<G->m;i++){
		if(teobj<G->Link[i]->load/G->Link[i]->capacity)
			teobj=G->Link[i]->load/G->Link[i]->capacity;
	}
	if(teobj>=1) { cout<<"return 2 "<<endl;return MIN;}
	  double obj= 1.0/teobj;
	//  cout<<"orobj  "<<orobj<<endl;
	//  cout<<"teobj1  "<<te<<endl;
	//  cout<<"teobj  "<<teobj<<endl;
	 this->delay=orobj;
	 this->mlu=teobj;
	 this->mlu1=te;

	 return obj;
}

#endif
