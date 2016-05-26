#ifndef EVOLUTION_H
#define EVOLUTION_H
#include"KSP.h"

// ���� individual
class evoluDiv{
	private:
		static const int MAT = 5; //����λ��
		static const int MUT = 4; //����λ��
		static const int HER = 30; //ѧϰλ��
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem; //
		vector<demand> *demor; //
	public:
		vector<int> x;//��  ����req������·�����
		double ability; //��������
		double delay,mlu1,mlu,euler;//��������ֵ������
		// ���캯��
		evoluDiv() {;}
		double GAability();
		//mΪreq������
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
		//////////////�����ӽ�  crossover
		evoluDiv mate(evoluDiv other){
			vector<int> nx;
			int n=dem->size();
			int num=rand()%MAT+1;
			/*
			//�ֳ����ػ��� ��num���ض�  Ч����
			for(int i=0;i<num;i++) 
				nx.push_back(x[i]);
			for(unsigned int ij=num;ij<x.size();ij++)
			   nx.push_back(other.x[ij]);	
			return evoluDiv(nx, G,GOR, dem,demor);
			*/
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
			return evoluDiv(nx, G,GOR, dem,demor);
			
		}
		// *********  ��������  *********//
		void calAbility(){
			ability = 0;
			G->reqPathNo.clear(); //�����һ��������·�����
			for(unsigned int i=0;i<x.size();i++)  ////////////////////////////////////// 
				G->reqPathNo.push_back(x[i]);
			ability=max(ability,GAability());	
		}
		void randNature(){
			for(unsigned int i = 0; i < x.size(); i++){
				x[i] = rand() % G->pathnum[i]; //pathnum[i]��ʾreq��·����Ŀ
			}
		}
		void mutation(){	
				//ѡ��λͻ��
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
		/////////�������Ž�  ���ѡHERλ��ͬ  ѡ��λ���������x�����±�
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
	return a.ability > b.ability; //abilityԽ����Ӧ��Խ��
}

//��Ⱥ ��Ⱥ�ɸ������
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
		// n ��Ⱥ��С  m:req��Ŀ ��ʼ����Ⱥ
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
		//��Ⱥ����  ��һ��
		evoluDiv evolution(){
			int nohero=0;
			fprintf(herofile,"Start:\n");
			for(unsigned int i = 0; i < hero.x.size(); i++)
				hero.x[i] = 0;//��ʼ��hero��Ϊÿ��req���߸���·�����ϵĵ�0��·
			hero.calAbility();
			if(hero.ability>MIN) fprintf(herofile, "%f\t%f\t%f\n", hero.mlu1,hero.mlu,hero.delay);
			//����ÿ�����������
			for(unsigned int i = 0; i < popu.size(); i++)
				popu[i].calAbility();		
			//2016-1-5 sort(popu.begin(), popu.end(), evoluCmp);//���������������Խ���ǿǿ����
			//��ֳ�Ĵ���
			for(int curYear = 1; curYear <= YEAR; curYear++){
			//���̶ĺ�ѡ��n������
			int n = popu.size(), getMore = 0;
			double sum=0;
		    for(int i=0;i<n;i++)
			    sum += popu[i].ability;
			vector<evoluDiv> chosepopu;
			chosepopu.clear();
			for(int i=0;i<n;i++) //n��Ⱥ��С
			{
				int ch=wheelchose(sum);
				//cout<<"ch "<<ch<<" ";
				chosepopu.push_back(popu[ch]);
			}
			popu.clear();
			for(unsigned int i=0;i<chosepopu.size();i++)
				popu.push_back(chosepopu[i]);
			vector<evoluDiv> sons;//����
			sons.clear();
			//�ӽ�
			for(unsigned int i = 0; i+1 < popu.size(); i+=2){
			   double p=rand()%100*0.01 ;
			   if(p>pc){  //pc �ӽ�����
						sons.push_back(popu[i]);
						sons.push_back(popu[i+1]);
					}
				else{
						sons.push_back(popu[i].mate(popu[i+1]));
					    sons.push_back(popu[i+1].mate(popu[i]));
					    sons.push_back(popu[i].mate(popu[i+1]));
					}
				}
			    sons.push_back(hero); /////��Ӣ����  2016-1-14  ���ܻᵼ�¾ֲ����� �ܿ�����  ��ͨ���ʵ��ı������
				int m = sons.size();//����ĸ���
				for(int i = 0; i < m; i++){
					//double p=rand()%100*0.01;
					//if(p<pm)       //��������Ը���pm
					sons[i].mutation();
					if(curYear > YEARCUL)
						sons[i].culture(hero); //��hero����
					sons[i].calAbility();
				}
				sort(sons.begin(), sons.end(), evoluCmp);
				popu.clear(); //�����һ����Ⱥ
				//printf("Year %d\n",curYear);
				for(int i = 0; i < n; i++){
					//printf("%.6f ",sons[i].ability);
					popu.push_back(sons[i]); //������Ⱥ��������Ϊn,����������ǰn�ķ�����һ����Ⱥ
					if(sons[i].ability > hero.ability){
						hero = sons[i]; //ѡ��������õ���Ϊhero
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
////  ����ÿ�� OD��֮���·��   ��ʼ��·����Ŀ�Ȳ���
void CGraph::GAinit(vector<demand> &req){
	pathnum.clear();//���req��ÿ��OD�Ե�·����Ŀ
	reqlistPath.clear();
 for(unsigned int i=0;i<req.size();i++)
 {  
	 KSP(req[i].org,req[i].des,K); //����õ�һ��OD�Ե�listPath
	 reqlistPath.push_back(listPath); //��listPath�浽reqlistPath��Ӧλ��   vector<vector<CPath*> > 
	 pathnum.push_back(listPath.size());
	// cout<<pathnum[i]<<" "<<endl;
	
 }
 //// �������
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
//////////////////************** ���ۺ��� �ࣨevoluDiv)����ĳ�Ա����*************************////////////////////////
//֪��ÿ��req������·֮��reqPathNo)  ����OR�ķ�Ӧ����Ӧ��֮����ӳ٣�����Ӧ��֮��TE��Ŀ��ֵ
double evoluDiv::GAability(){ 
	//����ÿ�����ϵĸ���
	this->G->clearOcc(); //���ÿ�����ϸ��� 
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

 //����Overlay�ı����ӳ�
 if(!CONSTANT){
	 for(int m=0;m<GOR->m;m++){	
			 bool flag=false;
			 double dmin=0;//�ӳ�
			for(unsigned int d=0;d<(*dem).size();d++){
				if(GOR->Link[m]->tail==(*dem)[d].org && GOR->Link[m]->head==(*dem)[d].des){ //OR����������
					if(flag==false){
					   	flag=true;
						//�����req��������·��   ��·���ϵ��ӳټ�����
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
			 double dmin=0;//�ӳ�
			for(unsigned int d=0;d<(*dem).size();d++){
				if(GOR->Link[m]->tail==(*dem)[d].org && GOR->Link[m]->head==(*dem)[d].des){ //OR����������
					if(flag==false){
					   	flag=true;
						//�����req��������·��   ��·���ϵ��ӳټ�����
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
	//OR����֮��TE�ľ���  eqfo  
	 vector<demand> req2;
	 req2.clear();
	 int num1=GOR->eqfo.size();
	 int num2=(*dem).size();
	 for(int d=0;d<num1;++d) 
		 req2.push_back(GOR->eqfo[d]); 
	 for(int d=num1;d<num2;++d)
		 req2.push_back((*dem)[d]);

	this->G->clearOcc(); //���ÿ�����ϸ��� 
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
