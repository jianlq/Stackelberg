#ifndef EVOLUTION2_H
#define EVOLUTION2_H
#include"KSP.h"

// 个体 individual
class evoluDiv2{
	private:
		static const int MUT = 4; //变异位数
		static const int HER = 8; //学习位数
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem; //
		vector<demand> *demor; //
	public:
		vector<int> x;//解  各个req经过的路径编号
		double ability; //个体能力
		double delay,mlu,euler;//个体能力值的体现
		// 构造函数
		evoluDiv2() {;}
		double GAability();
		//m为req的数量
		evoluDiv2(int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor){
			x.resize(m);
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			randNature(); ////////
		}
		evoluDiv2(vector<int> &tx, CGraph *g,CGraph *gor, vector<demand> *d, vector<demand> *dor){
			x.clear();
			G = g;
			GOR=gor;
			dem = d;
			demor=dor;
			for(unsigned int i = 0; i < tx.size(); i++)
				x.push_back(tx[i]);
		}
		//////////////交配杂交  crossover
		evoluDiv2 mate(evoluDiv2 other){
			vector<int> nx;
			int n=dem->size();
			int num=rand()%(n/2);
			//分成两截互换 在num处截断
			for(int i=0;i<num;i++) 
				nx.push_back(x[i]);
			for(unsigned int ij=num;ij<x.size();ij++)
			   nx.push_back(other.x[ij]);	
			return evoluDiv2(nx, G,GOR, dem,demor);	
			/*
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
			return evoluDiv2(nx, G,GOR, dem,demor);
			*/
		}
		// *********  计算能力  *********//
		void calAbility(){
			ability = 0;
			GOR->reqPathNo.clear(); //清空上一个个体解的路径编号
			for(unsigned int i=0;i<x.size();i++) 
				GOR->reqPathNo.push_back(x[i]); /////GOR  2016-1-5
			ability=max(ability,GAability());	
		}
		//OD对之间没有路 pathnum为0 但是不存在这种情况
		void randNature(){
			for(unsigned int i = 0; i < x.size(); i++){
				x[i] =   rand() % GOR->pathnum[i]; //pathnum[i]表示req的路径数目
			}
		}
		void mutation(){	
				//选择几位突变
				int num= rand()% MUT+1;
				for(int i=0;i<num;i++)
				{
					x[i]=rand()%GOR->pathnum[i];
				}	
		}
		/////////靠近最优解  随机选num位相同  选的位置又是随机x数组下标
		void culture(evoluDiv2 hero){
			int num=rand()%HER+1;
			int n=0;
			while(n<num){
              int j=rand()%x.size();
			for(unsigned int i = 0; i < x.size(); i++){
				if(i==j) x[i] =  hero.x[i];
			}
			n++;
		}
	}
};

 bool Cmp2(evoluDiv2 a, evoluDiv2 b){
	return a.ability < b.ability; //ability和适应度成反比，ability越小（最小化问题），适应度越大
  }

//种群 种群由个体组成
class evoluPopu2{
	private:
		static const int YEAR = 100;
		static const int YEARCUL = 60;
	    double pc,pm;
		vector<evoluDiv2> popu;
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem;
		vector<demand> *demor;
		FILE *herofile;
	public:
		evoluDiv2 hero;
		// n 种群大小  m:req数目 初始化种群
		evoluPopu2(int n, int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor){
			popu.clear();
			pc=0.95;
			pm=0.10;
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			for(int i = 0; i < n; i++){
				evoluDiv2 divi(m, G, GOR,dem,demor);
				popu.push_back(divi);
			}
			hero = evoluDiv2(m, G,GOR, dem,demor);
			herofile=fopen("outputFile//hero2.txt","a");
		}
		//种群进化  下一代
		evoluDiv2 evolution(){
			for(unsigned int i = 0; i < hero.x.size(); i++)
				hero.x[i] = 0;//开始的hero设为每个req都走各自路径集合的第0条路
			hero.calAbility();
			fprintf(herofile, "%f\t%f\t%f\n", hero.mlu,hero.delay,hero.euler);
			//评价每个个体的能力
			for(unsigned int i = 0; i < popu.size(); i++)
			{	popu[i].calAbility();
			   fprintf(herofile, "%f\t%f\t%f\n", popu[i].mlu,popu[i].delay,popu[i].euler);
			}
			sort(popu.begin(), popu.end(), Cmp2);//按个体优劣排列以进行强强交配

			//繁殖的代数
			for(int curYear = 1; curYear <= YEAR; curYear++){
				int n = popu.size(), getMore = 0;
				vector<evoluDiv2> sons;//子孙
				//杂交
				for(int i = 0; i+1 < n; i+=2){
					double p=rand()%100*0.01 ;
					if(p>pc) {  //pc 杂交概率
						sons.push_back(popu[i]);
						sons.push_back(popu[i+1]);
					}
					else{
						sons.push_back(popu[i].mate(popu[i+1]));
					    sons.push_back(popu[i+1].mate(popu[i]));
					    sons.push_back(popu[i].mate(popu[i+1]));
					}
				}
				int m = sons.size();//子孙的个数
				for(int i = 0; i < m; i++){
					double p=rand()%100*0.01;
					if(p<pm) sons[i].mutation();//个体变异以概率pm
					if(curYear > YEARCUL)
						sons[i].culture(hero); //向hero靠近
					sons[i].calAbility();
				}
				sort(sons.begin(), sons.end(), Cmp2);
				popu.clear(); //清空上一代种群
				for(int i = 0; i < n; i++){
					//printf("%.6f ", sons[i].ability);
					popu.push_back(sons[i]); //保持种群数量不变为n,将能力排在前n的放入下一代种群
					if(sons[i].ability < hero.ability){
						hero = sons[i]; //选出能力最好的作为hero
						getMore = 1;
					}
				}
				if(getMore){
					fprintf(herofile, "Year %d: find hero \n", curYear);
					fprintf(herofile,"%f\t%f\t%f\n", hero.mlu,hero.delay,hero.euler);
				}
			}
			fprintf(herofile,"\n\n\n");
			return hero;
		}
		void clearfile(){
		fclose(herofile);}
};

//////////////////************** 评价函数 类（evoluDiv)个体的成员函数*************************////////////////////////
//知道每个req走哪条路之后（reqPathNo)  计算te的反应（反应完之后的mlu），反应完之后OR的目标值delay
double evoluDiv2::GAability(){ 
	//计算每条边上的负载
	this->GOR->clearOcc(); //清空Overlay每条边上负载 
	for(unsigned int d=0;d<GOR->reqPathNo.size();d++)
	{
		int num=GOR->reqPathNo[d];// cout<<num<<" ";
		vector<CEdge*> lsEg=GOR->reqlistPath[d][num]->listEdge;
		vector<CEdge*>::iterator it,end=lsEg.end();
		for(it=lsEg.begin();it!=end;it++){
			(*it)->load += (*dem)[d].flow;
		}
	}	
	vector<double> x1;
	x1.clear();//x1的长度为边的数目
	GOR->eqfo.clear();
	for(int i=0;i<GOR->m;i++){	
		//cout<<GOR->Link[i]->load<<" "<<endl;
		x1.push_back(GOR->Link[i]->load); //将当次解放到x1里面 每条边上的负载
		if(GOR->Link[i]->load>0) GOR->eqfo.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,GOR->Link[i]->load));
	}
	 //计算TE的反应
	 vector<demand> req2;
	 req2.clear();
	 for(unsigned int d=0;d<GOR->eqfo.size();d++)
	    req2.push_back(GOR->eqfo[d]);
	 for(unsigned int d=NUMDEMAND;d<(*dem).size();d++) //dem的前NUMDEMAND个为TE的req
		 req2.push_back((*dem)[d]);
	 double teobj=G->TEcplex(req2,GOR);
	 if(teobj>=INF) return INF; //TE无解
	 //TE反应完之后 计算OR的反应
	 double orobj=GOR->ORcplex(demor);  
	 double or_euler=0;
	 for(unsigned int i=0;i<x1.size();i++)
	 {	or_euler += (x1[i]-GOR->loadx[i])*(x1[i]-GOR->loadx[i]);//x[1]是当前的个体（解）对应的每条边上的负载  loadx是当前解在OR反应完之后TE决策后的边上负载
	   //cout<< GOR->loadx[i]<<" ";
	 }
	 or_euler = sqrt(or_euler); //求得欧拉距离	
	 double obj= teobj;
	  //double obj=or_euler;
	  cout<<"orobj  "<<orobj<<endl;
	  cout<<"teobj  "<<teobj<<endl;
	  cout<<"euler "<<or_euler<<endl;
	  //cout<<"obj  "<<obj<<endl<<endl;
	 this->delay=orobj;
	 this->mlu=teobj;
	 this->euler=or_euler;
	 return obj;
}

#endif
