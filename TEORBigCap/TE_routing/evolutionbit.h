#ifndef EVOLUTIONBIT_H
#define EVOLUTIONBIT_H
#include"KSP.h"

// ���� individual
class evoluDivbit{
	private:
		static const int MUT = 4; //����λ��
		static const int HER = 20; //ѧϰλ��
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem; 
		vector<demand> *demor; 
	public:
		vector<vector<int>> x;//��  ����req������·����� �����Ʊ��� req*3
		double ability; //��������
		double delay,mlu1,mlu,euler;//��������ֵ������
		 double delaybase;
		// ���캯��
		evoluDivbit() {;}
		double GAability();
		//mΪreq������
		evoluDivbit(int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor,double delaybest){
			x.resize(m);//
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			delaybase=delaybest;
			randNature(); 
		}
		evoluDivbit(vector<vector<int> > &tx, CGraph *g,CGraph *gor, vector<demand> *d, vector<demand> *dor,double delaybest){
			x.clear();
			G = g;
			GOR=gor;
			dem = d;
			demor=dor;
			delaybase=delaybest;
			for(unsigned int i = 0; i < tx.size(); i++)
				x.push_back(tx[i]);
		}
		//////////////�����ӽ�  crossover
		evoluDivbit mate(evoluDivbit other){
			vector<vector<int> > nx;	
			/*
			//�ֳ����ػ��� ��num���ض�
			int n=dem->size();
			int num=rand()%(n/2);
			for(int i=0;i<num;i++) 
				nx.push_back(x[i]);
			for(unsigned int ij=num;ij<x.size();ij++)
			   nx.push_back(other.x[ij]);	
			return evoluDivbit(nx, G,GOR, dem,demor);	
			*/
			//��㽻��  �������һ��0-1����  Ϊ0������  Ϊ1����
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
			return evoluDivbit(nx, G,GOR, dem,demor,other.delaybase);	
		}
		//��������תΪʮ����  ·����ŵľ���ֵ
		int decoding(vector<int> &bit){
			 int z=0;
			 z = bit[0]*1+bit[1]*2+bit[2]*4;
			 return z;
		}
		// *********  ��������  *********//
		void calAbility(){
			ability = 0;
			G->reqPathNo.clear(); //�����һ��������·�����
			for(unsigned int i=0;i<x.size();i++) 
				G->reqPathNo.push_back(decoding(x[i])); 
			ability=max(ability,GAability());	
		}
		void randNature(){
			for(unsigned int i = 0; i < x.size(); i++){
				vector<int> bit;
				for(int b=0;b<3;b++)
					bit.push_back(rand()%2);//3λ���� �������0��1  ��ʾ·�����
				x[i] = bit; 
			}
		}
		void mutation(){	
			//����һλ  һ��req��·�����
			int row= rand()% dem->size();//������Ҫ�����req·�����
			int col=rand()%3;//������Ҫ�����λ��
			if(x[row][col]==0) x[row][col]==1;
			else if(x[row][col]==1) x[row][col]==0;  
		}
		/////////ѧϰ��ʷ���Ž�
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
	return a.ability > b.ability; //ability����Ӧ�ȳɷ��ȣ�abilityԽС����С�����⣩����Ӧ��Խ��
  }

//��Ⱥ ��Ⱥ�ɸ������
class evoluPopubit{
	private:
		static const int YEAR = 200;
		static const int YEARCUL = 120;
	    double pc,pm;
		vector<evoluDivbit> popu;
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem;
		vector<demand> *demor;
		FILE *herofile2;
	public:
		evoluDivbit hero;
		// n ��Ⱥ��С  m:req��Ŀ ��ʼ����Ⱥ
		evoluPopubit(int n, int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor,double delaybest){
			popu.clear();
			pc=0.85;
			pm=0.35;
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			for(int i = 0; i < n; i++){
				evoluDivbit divi(m, G, GOR,dem,demor,delaybest);
				popu.push_back(divi);
			}
			hero = evoluDivbit(m, G,GOR, dem,demor,delaybest);
			herofile2=fopen("outputFile//hero2.txt","a");
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
		evoluDivbit evolution(){
		fprintf(herofile2,"Start:\n");
			vector<int> h0;
			for(int b=0;b<3;b++)
				h0.push_back(0);
			for(unsigned int i = 0; i < hero.x.size(); i++)
				hero.x[i] = h0;//��ʼ��hero��Ϊÿ��req���߸���·�����ϵĵ�0��·
			hero.calAbility();
			if(hero.ability>MIN) fprintf(herofile2, "%f\t%f\t%f\n",  hero.mlu1,hero.mlu,hero.delay);
			//����ÿ�����������
			for(unsigned int i = 0; i < popu.size(); i++)
				popu[i].calAbility();
			//sort(popu.begin(), popu.end(), Cmp2);//���������������Խ���ǿǿ����
			//��ֳ�Ĵ���
			for(int curYear = 1; curYear <= YEAR; curYear++){
			//���̶ĺ�ѡ��n������
			int n = popu.size(), getMore = 0;
			double sum=0;
		    for(unsigned int i=0;i<n;i++)
			    sum += popu[i].ability;
			vector<evoluDivbit> chosepopu;
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
			vector<evoluDivbit> sons;//����
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
			    sons.push_back(hero);
				int m = sons.size();//����ĸ���
				for(int i = 0; i < m; i++){
					//double p=rand()%100*0.01;
					//if(p<pm) 
						sons[i].mutation();//��������Ը���pm
					if(curYear > YEARCUL)
						sons[i].culture(hero); //��hero����
					sons[i].calAbility();
				}
				sort(sons.begin(), sons.end(), Cmp2);
				popu.clear(); //�����һ����Ⱥ
				//printf("Year %d\n ", curYear);
				for(int i = 0; i < n; i++){
					//printf("%.6f ", sons[i].ability);
					popu.push_back(sons[i]); //������Ⱥ��������Ϊn,����������ǰn�ķ�����һ����Ⱥ
					if(sons[i].ability > hero.ability){
						hero = sons[i]; //ѡ��������õ���Ϊhero
						getMore = 1;
					}
				}
				if(getMore){
					fprintf(herofile2, "Year %d: find hero \n", curYear);
					fprintf(herofile2,"%f\t%f\t%f\n", hero.mlu1,hero.mlu,hero.delay);
				}
				printf("\n%f\t%f\n",hero.mlu,hero.delay);
			}
			fprintf(herofile2,"final hero\n%f\t%f\t%f\n", hero.mlu1,hero.mlu,hero.delay);
			fprintf(herofile2,"Ending\n\n\n\n");
			fclose(herofile2);
			return hero;
		}
};

//////////////////************** ���ۺ��� �ࣨevoluDiv)����ĳ�Ա����*************************////////////////////////
//֪��ÿ��req������·֮��reqPathNo)  ����te�ķ�Ӧ����Ӧ��֮���mlu������Ӧ��֮��OR��Ŀ��ֵdelay
double evoluDivbit::GAability(){ 
	//����ÿ�����ϵĸ���
	this->G->clearOcc(); //���ÿ�����ϸ��� 
	for(unsigned int d=0;d<G->reqPathNo.size();d++)
	{
		int num=G->reqPathNo[d]; 
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

	//if(te>=1) { cout<<endl<<"return 1 bit "<<endl;return MIN;}
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
							else dmin += (double)1.0/(double)((*lgi)->capacity-(*lgi)->load);//!!!!!!!!!!double	          
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
	for(unsigned int d=0;d<req2.size();++d){
		int num=G->reqPathNo[d]; //cout<<num<<" ";
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
	if(teobj>=1) { cout<<"return 2 bit"<<endl;return MIN;}
	// double obj=1.0/teobj;
	  double obj= 1.0/teobj+1.2*delaybase/orobj;
	 // cout<<"orobj  "<<orobj<<endl;
	 // cout<<"te  "<<te<<endl;
	 // cout<<"teobj  "<<teobj<<endl;
	 this->mlu1=te;
	 this->delay=orobj;
	 this->mlu=teobj;

	 return obj;
}

#endif
