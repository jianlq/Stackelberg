#ifndef EVOLUTION2_H
#define EVOLUTION2_H
#include"KSP.h"

// ���� individual
class evoluDiv2{
	private:
		static const int MUT = 4; //����λ��
		static const int HER = 8; //ѧϰλ��
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem; //
		vector<demand> *demor; //
	public:
		vector<int> x;//��  ����req������·�����
		double ability; //��������
		double delay,mlu,euler;//��������ֵ������
		// ���캯��
		evoluDiv2() {;}
		double GAability();
		//mΪreq������
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
		//////////////�����ӽ�  crossover
		evoluDiv2 mate(evoluDiv2 other){
			vector<int> nx;
			int n=dem->size();
			int num=rand()%(n/2);
			//�ֳ����ػ��� ��num���ض�
			for(int i=0;i<num;i++) 
				nx.push_back(x[i]);
			for(unsigned int ij=num;ij<x.size();ij++)
			   nx.push_back(other.x[ij]);	
			return evoluDiv2(nx, G,GOR, dem,demor);	
			/*
			//�������ѡnumλ����
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
		// *********  ��������  *********//
		void calAbility(){
			ability = 0;
			GOR->reqPathNo.clear(); //�����һ��������·�����
			for(unsigned int i=0;i<x.size();i++) 
				GOR->reqPathNo.push_back(x[i]); /////GOR  2016-1-5
			ability=max(ability,GAability());	
		}
		//OD��֮��û��· pathnumΪ0 ���ǲ������������
		void randNature(){
			for(unsigned int i = 0; i < x.size(); i++){
				x[i] =   rand() % GOR->pathnum[i]; //pathnum[i]��ʾreq��·����Ŀ
			}
		}
		void mutation(){	
				//ѡ��λͻ��
				int num= rand()% MUT+1;
				for(int i=0;i<num;i++)
				{
					x[i]=rand()%GOR->pathnum[i];
				}	
		}
		/////////�������Ž�  ���ѡnumλ��ͬ  ѡ��λ���������x�����±�
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
	return a.ability < b.ability; //ability����Ӧ�ȳɷ��ȣ�abilityԽС����С�����⣩����Ӧ��Խ��
  }

//��Ⱥ ��Ⱥ�ɸ������
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
		// n ��Ⱥ��С  m:req��Ŀ ��ʼ����Ⱥ
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
		//��Ⱥ����  ��һ��
		evoluDiv2 evolution(){
			for(unsigned int i = 0; i < hero.x.size(); i++)
				hero.x[i] = 0;//��ʼ��hero��Ϊÿ��req���߸���·�����ϵĵ�0��·
			hero.calAbility();
			fprintf(herofile, "%f\t%f\t%f\n", hero.mlu,hero.delay,hero.euler);
			//����ÿ�����������
			for(unsigned int i = 0; i < popu.size(); i++)
			{	popu[i].calAbility();
			   fprintf(herofile, "%f\t%f\t%f\n", popu[i].mlu,popu[i].delay,popu[i].euler);
			}
			sort(popu.begin(), popu.end(), Cmp2);//���������������Խ���ǿǿ����

			//��ֳ�Ĵ���
			for(int curYear = 1; curYear <= YEAR; curYear++){
				int n = popu.size(), getMore = 0;
				vector<evoluDiv2> sons;//����
				//�ӽ�
				for(int i = 0; i+1 < n; i+=2){
					double p=rand()%100*0.01 ;
					if(p>pc) {  //pc �ӽ�����
						sons.push_back(popu[i]);
						sons.push_back(popu[i+1]);
					}
					else{
						sons.push_back(popu[i].mate(popu[i+1]));
					    sons.push_back(popu[i+1].mate(popu[i]));
					    sons.push_back(popu[i].mate(popu[i+1]));
					}
				}
				int m = sons.size();//����ĸ���
				for(int i = 0; i < m; i++){
					double p=rand()%100*0.01;
					if(p<pm) sons[i].mutation();//��������Ը���pm
					if(curYear > YEARCUL)
						sons[i].culture(hero); //��hero����
					sons[i].calAbility();
				}
				sort(sons.begin(), sons.end(), Cmp2);
				popu.clear(); //�����һ����Ⱥ
				for(int i = 0; i < n; i++){
					//printf("%.6f ", sons[i].ability);
					popu.push_back(sons[i]); //������Ⱥ��������Ϊn,����������ǰn�ķ�����һ����Ⱥ
					if(sons[i].ability < hero.ability){
						hero = sons[i]; //ѡ��������õ���Ϊhero
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

//////////////////************** ���ۺ��� �ࣨevoluDiv)����ĳ�Ա����*************************////////////////////////
//֪��ÿ��req������·֮��reqPathNo)  ����te�ķ�Ӧ����Ӧ��֮���mlu������Ӧ��֮��OR��Ŀ��ֵdelay
double evoluDiv2::GAability(){ 
	//����ÿ�����ϵĸ���
	this->GOR->clearOcc(); //���Overlayÿ�����ϸ��� 
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
	x1.clear();//x1�ĳ���Ϊ�ߵ���Ŀ
	GOR->eqfo.clear();
	for(int i=0;i<GOR->m;i++){	
		//cout<<GOR->Link[i]->load<<" "<<endl;
		x1.push_back(GOR->Link[i]->load); //�����ν�ŵ�x1���� ÿ�����ϵĸ���
		if(GOR->Link[i]->load>0) GOR->eqfo.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,GOR->Link[i]->load));
	}
	 //����TE�ķ�Ӧ
	 vector<demand> req2;
	 req2.clear();
	 for(unsigned int d=0;d<GOR->eqfo.size();d++)
	    req2.push_back(GOR->eqfo[d]);
	 for(unsigned int d=NUMDEMAND;d<(*dem).size();d++) //dem��ǰNUMDEMAND��ΪTE��req
		 req2.push_back((*dem)[d]);
	 double teobj=G->TEcplex(req2,GOR);
	 if(teobj>=INF) return INF; //TE�޽�
	 //TE��Ӧ��֮�� ����OR�ķ�Ӧ
	 double orobj=GOR->ORcplex(demor);  
	 double or_euler=0;
	 for(unsigned int i=0;i<x1.size();i++)
	 {	or_euler += (x1[i]-GOR->loadx[i])*(x1[i]-GOR->loadx[i]);//x[1]�ǵ�ǰ�ĸ��壨�⣩��Ӧ��ÿ�����ϵĸ���  loadx�ǵ�ǰ����OR��Ӧ��֮��TE���ߺ�ı��ϸ���
	   //cout<< GOR->loadx[i]<<" ";
	 }
	 or_euler = sqrt(or_euler); //���ŷ������	
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
