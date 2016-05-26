#ifndef EVOLUTIONBIT_H
#define EVOLUTIONBIT_H
#include"KSP.h"

// ���� individual
class evoluDivbit{
	private:
		static const int MUT = 3; //����λ��
		static const int HER = 20; //ѧϰλ��
		int consider;
		CGraph *G;
		CGraph *GOR;
		vector<demand> *dem; 
		vector<demand> *demor; 
	public:
		vector<vector<int>> x;//��  ����req������·����� �����Ʊ��� req*3
		double ability; //��������
		double delay1,delay,mlu1,mlu,euler;//��������ֵ������
		double delaybase;
		// ���캯��
		evoluDivbit() {;}
		double GAability();
		double GAabilityNoCplex();
		void Init(){
			ability = MIN;
			delay1 = 0;
			delay = 0;
			mlu1 = 0;
			mlu = 0;
			euler = 0;
		}
		//mΪreq������
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
			 // Ҳ������  x = tx ; ��������Ĳ���
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
				for(int j=0;j<4;j++)   ///////////////////////   16 �� 4      8 �� 3
					bit.push_back(rand()%2);
				onezero.push_back(bit); 
			}
			for(unsigned int n=0;n<x.size();n++)
				nx.push_back(x[n]);
			for(unsigned int i=0;i<onezero.size();i++)
					for(int j=0;j<4;j++){  ///////////////////////   16 �� 4      8 �� 3
						if(onezero[i][j]==1)
							nx[i][j]=other.x[i][j];
					}
					return evoluDivbit(nx, G,GOR, dem,demor,other.delaybase,other.consider);	
		}
		//��������תΪʮ����  ·����ŵľ���ֵ
		int decoding(vector<int> &bit){
			 int z=0;
			 z = bit[0]*1+bit[1]*2+bit[2]*4+bit[2]*8;//// 16��·
			 //  z = bit[0]*1+bit[1]*2+bit[2]*4;//// 8��·
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
				for(int b=0;b<4;b++)        ///////////////////////   16 �� 4      8 �� 
					bit.push_back(rand()%2);// 4/3λ���� �������0��1  ��ʾ·�����
				x[i] = bit; 
			}
		}
		void mutation(){	
			int i=0;
			while(i<MUT){
			int row= rand()% dem->size();//������Ҫ�����req·�����
			int col=rand()%4;//������Ҫ�����λ��   ///////////////////////   16 �� 4      8 �� 3
			if(x[row][col]==0) x[row][col]=1;
			else if(x[row][col]==1) x[row][col]=0; 
			i++;
			}
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
		static const int YEAR = 50;
		static const int YEARCUL = 1000;
		static const int NOHERO = 20;
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
		evoluPopubit(int n, int m, CGraph *g, CGraph *gor,vector<demand> *d,vector<demand> *dor,double delaybest,int con){
			popu.clear();
			pm=0.25;
			G = g;
			GOR=gor;
			dem = d;
			demor = dor;
			for(int i = 0; i < n; i++){
				evoluDivbit divi(m, G, GOR,dem,demor,delaybest,con);
				popu.push_back(divi);
			}
			hero = evoluDivbit(m, G,GOR, dem,demor,delaybest,con);
			herofile2=fopen("outputFile//hero2.txt","a");
		}
		///////////  ���̶ĺ�
		int wheelchose(double sum){
			double m=0;
			double r=rand()%1000*0.001;
			int res = rand()%popu.size();
			for(unsigned int i=0;i<popu.size();i++){
				m += popu[i].ability/sum;
				if(r<=m) {
					cout << i << "   ";
					return i;
				}
			}
			return res; //���forѭ����û�з���   // ��� ����evoluPopubit::wheelchose��: �������еĿؼ�·��������
		}
		/////////// ��Ⱥ����  ��һ�� //////////////////
		evoluDivbit evolution(){
		int nohero=0;//��¼�������
		fprintf(herofile2,"Start:\n ");
			vector<int> h0;
			for(int b=0;b<4;b++)   ///////////////////////   16 �� 4      8 �� 3
				h0.push_back(0);
			for(unsigned int i = 0; i < hero.x.size(); i++)
				hero.x[i] = h0;//��ʼ��hero��Ϊÿ��req���߸���·�����ϵĵ�0��·
			hero.calAbility();
			if(hero.ability>MIN) fprintf(herofile2, "%f\t%f\t%f\n", hero.mlu1,hero.delay,hero.ability);
			//����ÿ�����������
			for(unsigned int i = 0; i < popu.size(); i++)
				popu[i].calAbility();
			//sort(popu.begin(), popu.end(), Cmp2);//���������������Խ���ǿǿ����
			//��ֳ�Ĵ���
			for(int curYear = 1; curYear <= YEAR; curYear++){
			//���̶ĺ�ѡ��n������
			int n = popu.size(), getMore = 0;
			double sum=0;
		    for( int i=0;i<n;i++)
			    sum += popu[i].ability;
			vector<evoluDivbit> chosepopu;
			chosepopu.clear();
			cout<< " ch   " <<endl;
			for(int i=0;i<n;i++) //n��Ⱥ��С
			{
				int ch=wheelchose(sum);
				// cout<<"  "<<ch<<"  ";
				chosepopu.push_back(popu[ch]);
			}
			popu.clear();
			for(unsigned int i=0;i<chosepopu.size();i++)
				popu.push_back(chosepopu[i]);
			vector<evoluDivbit> sons;//����
			sons.clear();
			//�ӽ�
			for(unsigned int i = 0; i+1 < popu.size(); i+=2){
				sons.push_back(popu[i].mate(popu[i+1]));
				sons.push_back(popu[i+1].mate(popu[i]));
				sons.push_back(popu[i].mate(popu[i+1]));
			}
			    sons.push_back(hero);
				int m = sons.size();//����ĸ���
				for(int i = 0; i < m; i++){
					double p=rand()%100*0.01;
					if(p<pm) 
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
					
					if(abs(sons[i].ability - hero.ability) < 1e-4 ){	
						//cout<< sons[i].ability <<" ====== "<< hero.ability;
						continue;
					}
					else if(sons[i].ability > hero.ability){
						//cout<< sons[i].ability <<"  >>>>>>  "<< hero.ability;
						hero = sons[i]; //ѡ��������õ���Ϊhero
						getMore = 1;
					}
				}
				if(getMore){
					fprintf(herofile2, "Year %d: find hero \n", curYear);
					fprintf(herofile2,"%f\t%f\t%f\n", hero.mlu1,hero.delay,hero.ability);
					//printf("%f\t%f\t%f\n", hero.mlu,hero.mlu1,hero.delay);

				}
				else nohero++;
				if(nohero> NOHERO){
					break;}
				//printf("\n%f\t%f\n",hero.mlu,hero.delay);
			}
			//fprintf(herofile2,"\nfinal hero\n%f\t%f\n", hero.mlu1,hero.delay1);
			fprintf(herofile2,"\n\n\n\n");
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
		//cout<< lsEg.size() << "   ";
		//cout << endl;
		vector<CEdge*>::iterator it,end=lsEg.end();
		for(it=lsEg.begin();it!=end;it++){
			(*it)->load += (*dem)[d].flow;
		}
	}
	//double te=0; // �����·�ɷ��� Overlay��û��Ӧ֮ǰTE����·������
	//for(int i=0;i<G->m;i++){
	//	if(te<G->Link[i]->load/G->Link[i]->capacity)
	//		te=G->Link[i]->load/G->Link[i]->capacity;
	//}
	////if(te>=1) { cout<<endl<<"return 1 bit "<<endl;return MIN;}
	//double or=0; //�����·�ɷ���Overlay���ӳ�
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
	if(!CONSTANT){  //�ǳ���  //����Overlay�ı����ӳ�
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
				GOR->Link[m]->latency=dmin;
				 break;
			}
         }
	 }	
}
else{  // CONSTANT
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

	//OR����֮��TE�ľ���  eqfo  
	 vector<demand> req2;
	 int num1=GOR->eqfo.size();
	 int num2=(*dem).size();
	 for(int d=0;d<num1;++d) 
		 req2.push_back(GOR->eqfo[d]); 
	 int k = (*demor).size();
	 for(int d = k; d < num2; ++d)
		 req2.push_back((*dem)[d]);
	//this->G->clearOcc(); //���ÿ�����ϸ��� 
	//for(unsigned int d=0;d<req2.size();++d){   // overlay����װ��ǰ��
	//	int num=G->reqPathNo[d]; //cout<<num<<" ";
	//	vector<CEdge*> lsEg=G->reqlistPath[d][num]->listEdge;
	//	vector<CEdge*>::iterator it,end=lsEg.end();
	//	for(it=lsEg.begin();it!=end;++it){
	//		(*it)->load += req2[d].flow;
	//	}
	//}
	//double teobj=0; //����²�·�ɲ��� ��TE��Ŀ��ֵ
	//for(int i=0;i<G->m;i++){
	//	if(teobj<G->Link[i]->load/G->Link[i]->capacity)
	//		teobj=G->Link[i]->load/G->Link[i]->capacity;
	//}

	double teobj2 = G->TEbase(req2);
	if(teobj2>=INF){ cout<<" return te2  "<<endl;return MIN;}
	//if(teobj>=1) { cout<<"return te "<<endl;return MIN;}	
	double obj = MIN;
	// te�ǵ�ǰ���߸�TE������Ŀ��ֵ   teobj�ǿ��ǵ�Overlay���ߺ��TE��Ӱ��
	if(this->consider==0)   obj = 1.0/teobj2; ///// ������overlay
	else if(this->consider==1)
		obj= 1.0/teobj2+1.2*delaybase/orobj;  // consider = 1 ,����overlay  1.0/teobj+1.5*delaybase/orobj
	else
		obj= 1.0/teobj2+1.5*delaybase/orobj;
	this->delay=orobj;//Ŀ��
	this->mlu1 = teobj2;//Ŀ��

	return obj;
}

double evoluDivbit::GAabilityNoCplex(){ 
	//����ÿ�����ϵĸ���
	this->G->clearOcc(); //���ÿ�����ϸ��� 
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

	

}

//void evoluDivbit::Caldelay(){
//	for(int ij=0;ij<G->m;ij++){
//		if(G->Link[ij]->use>=G->Link[ij]->capacity) G->Link[ij]->latency = 1.0;
//		else G->Link[ij]->latency=1.0/(G->Link[ij]->capacity-G->Link[ij]->use);  //������Ϊ��λ�ӳ�  
//	}
//	int totalnum = (*dem).size();
//	for(int m=0;m<GOR->m;m++){	//����OR���ӳ�
//		bool flag=false;
//		double dmin=0;
//		for(int d=0;d<totalnum;d++){
//			if(GOR->Link[m]->tail==eq[d].org && GOR->Link[m]->head==eq[d].des && (eq[d].flow>0) ){ 
//				if(flag==false){
//					flag=true;
//					//cout<<G->reqPathID[d].size()<<"  ";
//					for(int ij=0;ij<G->reqPathID[d].size();ij++){	
//						dmin += G->Link[G->reqPathID[d][ij]]->latency;
//					}
//				}
//			}
//			if(flag==true){
//				GOR->Link[m]->latency=dmin;	
//				break;
//			}
//		} 		
//	} 
//}

// OR��Overlay��δ����ѡ·  ���������Դ�����������С������ѡ·  �õ��Ĵ��������ʺ��ӳ�
pair<double,double> Calability(CGraph *G,CGraph *GOR,vector<demand> &req){
	pair<double,double> res(INF,INF);
	G->clearOcc();
	double mlu = 0;
	int block = 0,success = 0;
	for(int i = 0; i < req.size(); i++){
		double ret = G->dijkstra(i,req[i].org, req[i].des, req[i].flow,1); //�Դ�����������С������ѡ·
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
		//ilp += G->TEcplex(eq,GOR);	
		///// calculate delay
		for(int ij=0;ij<G->m;ij++){
			if(G->Link[ij]->use>=G->Link[ij]->capacity) 
				G->Link[ij]->latency = 1.0;
			else 
				G->Link[ij]->latency=1.0/(G->Link[ij]->capacity-G->Link[ij]->use);  //������Ϊ��λ�ӳ�  
	}

	int totalnum = req.size();
	for(int m = 0; m < GOR->m; m++){	//����OR���ӳ�
		bool flag=false;
		double dmin=0;
		for(int d = 0; d < totalnum; d++){
			if(GOR->Link[m]->tail==req[d].org && GOR->Link[m]->head==req[d].des && (req[d].flow>0) ){ 
				if(flag==false){
					flag=true;
					//cout<<G->reqPathID[d].size()<<"  ";
					for(int ij=0;ij<G->reqPathID[d].size();ij++){	
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
	for(int d = 0; d < reqor.size(); d++){
		del += GOR->dijkstraOR(reqor[d].org,reqor[d].des)*reqor[d].flow; //���ӳ�ΪȨ�ص�dijkstra
	}
    orobj = del ;
	}  // end of if
	res.first = teobj;
	res.second = orobj;
	return res;
}


#endif
