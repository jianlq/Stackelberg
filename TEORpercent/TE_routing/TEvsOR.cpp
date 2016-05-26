#include"evolutionbit.h"
#include"Nash.h"

int main(){
	srand((unsigned)time(NULL));	
	CGraph *G=new CGraph("inputFile//graphATT.txt");
	CGraph *GOR=new CGraph("inputFile//graphATTOR.txt");

	vector<demand> eqOR;
	vector<demand> eqTE;
	vector<demand> eqbase;
	FILE *out = fopen("outputFile//obj90.txt", "a");
	int CASEnum=3;
	for(int casenum=0;casenum<CASEnum;casenum++){

		eqbase.clear();//随机产生TE的demand,相当于background流 //有背景流
		for(int i = 0; i < NUMDEMAND; i++){
			int s = rand()%G->n, t;
			do{
				t = rand()%G->n;
			}while(s == t);
			eqbase.push_back(demand(s, t, rand()%(MAXDEMAND)+1));
		}

		//百分数 测量百分之20 10的时候用到
		if(TEN){
			eqOR.clear();
			int denum=3;
			for(int i=0;i<denum;i++){
				int j=rand()%GOR->m;
				eqOR.push_back(demand(GOR->Link[j]->tail,GOR->Link[j]->head,rand()%MAXDEMAND+2));
			}
		}
		else{
			////Overlay   产生demand
			eqOR.clear(); //产生Overlay节点对之间的流量需求
			for(int i=0;i<GOR->m;i++)
				eqOR.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,rand()%(MAXDEMAND)+1));
		}


		//////////////////// ①①①①① Nash 博弈  ////////////////////////
		eqTE.clear(); 
		for(unsigned int i=0;i<eqOR.size();i++)
			eqTE.push_back(eqOR[i]);			
		for(unsigned int i=0;i<eqbase.size();i++)
			eqTE.push_back(eqbase[i]);

		fprintf(out, "TEvsOR    博弈:\n");
		G->clearOcc();
		for(int demo =1; demo <= 100; demo++){
			double te=0,or=0;
			te +=G->TEcplex(eqTE,GOR);
			if(te>=INF) { fprintf(out, "TE unfeasible\n");break;}
			//for(int i=0;i<GOR->m;i++)
			//	cout<< GOR->Link[i]->dist<< "  ";
			or += GOR->ORcplex(&eqOR);
			if(or>=INF) { fprintf(out, "OR unfeasible\n");break;}
			eqTE.clear();
			for(unsigned int i=0;i<GOR->eqfo.size();i++)
				eqTE.push_back(GOR->eqfo[i]);				
			for(unsigned int i=0;i<eqbase.size();i++)
				eqTE.push_back(eqbase[i]);
			fprintf(out, "%f\t%f\n", te,or);
		}
		fprintf(out, "\n\n"); 

	}
	fclose(out);
	delete G;
	delete GOR;
	system("pause");
	return 0;

}


