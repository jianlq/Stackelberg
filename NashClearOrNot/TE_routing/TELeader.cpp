#define COMMOM_H
#include"evolution.h"
#include"evolutionbit.h"
#include"Nash.h"
#include<vld.h> ////Visual Leak Detector 内存泄露检测工具

void genGraph(int n, int m, int maxWeight, char route[]){ //也会产生重复的边
    FILE *out = fopen(route, "w");
	fprintf(out,"%d\n",n);
    for(int i = 1; i < min(n, m+1); i++){
        int t = rand()%i, w = rand()%maxWeight+1;
        fprintf(out, "%d %d %d \n", i, t, w);
        fprintf(out, "%d %d %d \n", t, i, w);
    }
    for(int i = 0; i < m-n+1; i++){
        int s = rand()%n, t = rand()%n, w = rand()%maxWeight+1;
        while(t == s)
            t = rand()%n;
        fprintf(out, "%d %d %d \n", s, t, w);
        fprintf(out, "%d %d %d \n", t, s, w);
    }
    fclose(out);
}

void genORGraph(int n1,unsigned int n2,int m,int maxWeight,char route[]){ //会产生重复的边
	FILE *out=fopen(route,"w");
	fprintf(out,"%d\n",n2);
	set<int> ver; //不存重复元素
	while(ver.size()<n2)
	{
		int s=rand()%n1;
		ver.insert(s);
	}

vector<int> ver2;
set<int>::iterator i=ver.begin();
for(;i!=ver.end();i++)
	ver2.push_back(*i);

 for(unsigned int i = 1; i < n2; i++){
        int t = rand()%i, w = rand()%maxWeight+1;
        fprintf(out, "%d %d %d \n", ver2[i], ver2[t], w);
        fprintf(out, "%d %d %d \n",ver2[t], ver2[i], w);
    }

  for(unsigned int i = 0; i < m-n2+1; i++){
        int s = rand()%n2, t = rand()%n2, w = rand()%maxWeight+1;
        while(t == s)
            t = rand()%n2;
        fprintf(out, "%d %d %d \n", ver2[s], ver2[t], w);
        fprintf(out, "%d %d %d \n", ver2[t], ver2[s], w);
    }
    fclose(out);
}


int main(){
	srand((unsigned)time(NULL));	
	CGraph *G=new CGraph("inputFile//graphATT.txt");
	CGraph *GOR=new CGraph("inputFile//graphATTOR.txt");	

	vector<demand> eqOR;
	vector<demand> eqTE;
	vector<demand> eqbase;
	FILE *out = fopen("outputFile//clear.txt", "a");
	int CASEnum= 1;
	for(int casenum=0;casenum<CASEnum;casenum++){
		eqbase.clear();//随机产生TE的demand,相当于background流 //有背景流
		for(int i = 0; i < NUMDEMAND; i++){
			int s = rand()%G->n, t;
			do{
				t = rand()%G->n;
			}while(s == t);
			eqbase.push_back(demand(s, t, rand()%(MAXDEMAND)+2));
		}	
		////Overlay   产生demand
		eqOR.clear(); //产生Overlay节点对之间的流量需求
		for(int i=0;i<GOR->m;i++)
			eqOR.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,rand()%(MAXDEMAND)+1));

		//////////////// ①①①①① Nash 博弈  ////////////////////////
		eqTE.clear(); 
		for(unsigned int i=0;i<eqOR.size();i++)
			eqTE.push_back(eqOR[i]);			
		for(unsigned int i=0;i<eqbase.size();i++)
			eqTE.push_back(eqbase[i]);
		fprintf(out, "TEvsOR    博弈:\n");
		G->clearOcc();
		for(int demo =1; demo <= 50; demo++){
			double te=0,or=0;
			te +=G->TEcplex(eqTE,GOR);
			if(te>=INF) { fprintf(out, "TE unfeasible\n");break;}
			or += GOR->ORcplex(&eqOR);
			if(or>=INF) { fprintf(out, "OR unfeasible\n");break;}
			//GOR->clearOcc();
			eqTE.clear();
			
			for(unsigned int i=0;i<GOR->eqfo.size();i++){
				eqTE.push_back(GOR->eqfo[i]);	
			}
			for(unsigned int i=0;i<eqbase.size();i++)
				eqTE.push_back(eqbase[i]);
			fprintf(out, "%f\t%f\n", te,or);
		}
		fprintf(out, "\n\n"	); 

		//////////////// ①①①①① Nash 博弈  clear////////////////////////
		eqTE.clear(); 
		for(unsigned int i=0;i<eqOR.size();i++)
			eqTE.push_back(eqOR[i]);			
		for(unsigned int i=0;i<eqbase.size();i++)
			eqTE.push_back(eqbase[i]);
		fprintf(out, "TEvsOR clear    博弈:\n");
		G->clearOcc();
		for(int demo =1; demo <= 50; demo++){
			double te=0,or=0;
			te +=G->TEcplex(eqTE,GOR);
			if(te>=INF) { fprintf(out, "TE unfeasible\n");break;}
			or += GOR->ORcplex(&eqOR);
			if(or>=INF) { fprintf(out, "OR unfeasible\n");break;}
			GOR->clearOcc();// 重置边上延迟   2016-5-17 有区别  重置和不重置
			eqTE.clear();
			for(unsigned int i=0;i<GOR->eqfo.size();i++){
				eqTE.push_back(GOR->eqfo[i]);		
			}
			for(unsigned int i=0;i<eqbase.size();i++)
				eqTE.push_back(eqbase[i]);
			fprintf(out, "%f\t%f\n", te,or);
		}
		fprintf(out, "\n\n"	); 
	}
	fclose(out);
	delete G;
	delete GOR;
	cout<< " TE routing " <<endl;
	system("pause");
	return 0;
}