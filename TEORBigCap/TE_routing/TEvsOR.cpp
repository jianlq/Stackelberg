#include"evolution.h"
#include"evolutionbit.h"
#include"Nash.h"

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
	CGraph *G=new CGraph("inputFile//graph52.txt");
	CGraph *GOR=new CGraph("inputFile//graph52OR.txt");
	
	/*//test KSP
	G->KSP(9, 1, 5);
	for(int i=0;i<G->listPath.size();i++)
	{
		//reverse(G->listPath[i]->listEdge.begin(),G->listPath[i]->listEdge.end());
		vector<CEdge*>::iterator it=G->listPath[i]->listEdge.begin(),end=G->listPath[i]->listEdge.end();
		for(it;it!=end;it++)
			cout<<(*it)->tail<<" "<<(*it)->head<<" ";
		cout<<endl;
	}*/

	 vector<demand> eqOR;
	 vector<demand> eqTE;
	 vector<demand> eqbase;
	 FILE *out = fopen("outputFile//objcapmm1.txt", "a");
	int CASEnum=5;
	for(int casenum=0;casenum<CASEnum;casenum++){

	eqbase.clear();//随机产生TE的demand,相当于background流 //有背景流
	for(int i = 0; i < NUMDEMAND; i++){
		int s = rand()%G->n, t;
		do{
			t = rand()%G->n;
		}while(s == t);
		eqbase.push_back(demand(s, t, rand()%(MAXDEMAND)+1));
	}
	////Overlay   产生demand
	eqOR.clear(); //产生Overlay节点对之间的流量需求
	for(int i=0;i<GOR->m;i++)
		eqOR.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,rand()%(MAXDEMAND)+1));

	//////////////////// ①①①①① Nash 博弈  ////////////////////////
	eqTE.clear(); 
	for(unsigned int i=0;i<eqOR.size();i++)
	   eqTE.push_back(eqOR[i]);			
	for(unsigned int i=0;i<eqbase.size();i++)
		eqTE.push_back(eqbase[i]);

	//for(unsigned int i=0;i<eqTE.size();i++)
	//	cout<<eqTE[i].org<<" "<<eqTE[i].des<<" "<<eqTE[i].flow<<endl;

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


	//////////////////// 增大capacity  ////////////////////////
	eqTE.clear(); 
	for(unsigned int i=0;i<eqOR.size();i++)
	   eqTE.push_back(eqOR[i]);			
	for(unsigned int i=0;i<eqbase.size();i++)
		eqTE.push_back(eqbase[i]);

	for(int ij=0;ij<G->m;ij++)
		G->Link[ij]->capacity += 100;

		fprintf(out, "TEvsOR 2    博弈:\n");
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

	/*
	/////////////// ③③③③③  Network Friendly Overlay Routing  /////////////////////////
	eqTE.clear(); //eqTE前面必须装Overlay需求  为了编程方便
	for(unsigned int i=0;i<eqOR.size();i++)
	   eqTE.push_back(eqOR[i]);			
	for(unsigned int i=0;i<eqbase.size();i++)
		eqTE.push_back(eqbase[i]);
	G->clearOcc();
	 fprintf(out, "Network Friendly Overlay Routing\n");
	for(int ij=0;ij<GOR->m;ij++)
		GOR->Link[ij]->load=INF;
	for(int demo =1; demo <= 100; demo++){
			double te=0,or=0;
			te +=G->TEcplex(eqTE,GOR);
			if(te>=INF) { fprintf(out, "TE unfeasible\n");break;}
			or += ORBcplex(GOR,eqOR);
			if(or>=INF) { fprintf(out, "OR unfeasible\n");break;}
			eqTE.clear();
			for(unsigned int i=0;i<GOR->eqfo.size();i++)
				eqTE.push_back(GOR->eqfo[i]);				
			for(unsigned int i=0;i<eqbase.size();i++)
				eqTE.push_back(eqbase[i]);	
			 fprintf(out, "%f\t%f\n", te,or);
		}
	 	fprintf(out, "\n\n"); 
		*/

/*
	///////////////////////②②②②②  独裁  ////////////////////
	//////////////TE独裁  对Overlay选路安排也归TE所有
	eqTE.clear(); //eqTE前面必须装Overlay需求  为了编程方便
	for(unsigned int i=0;i<eqOR.size();i++)
	   eqTE.push_back(eqOR[i]);			
	for(unsigned int i=0;i<eqbase.size();i++)
		eqTE.push_back(eqbase[i]);
	G->clearOcc();
	fprintf(out, "TE dictator\n"); 
	double tedic=0,teor=0;
	tedic +=TEdictor(G,eqTE,GOR);
	teor=G->delay;
	fprintf(out, "%f\t%f\n\n",tedic,teor); 

	
	/////////////OR独裁  对TE选路安排也归TE所有
	fprintf(out, "Overlay dictator\n");
	G->clearOcc();
	double ordic=0,orte=0;
	ordic +=ORdictor(G,eqTE,GOR->m);
	orte=G->mlu;
	fprintf(out, "%f\t%f\n\n",ordic,orte); 
	fprintf(out, "\n\n"); 
	
	
  /////////// ④④④④④④ 遗传算法 ///////////
////////////////遗传算法1  搜索TE的解空间 十进制编码
	G->clearOcc();
	int Stime=2;
	G->GAinit(eqTE);	
	int n=150;//种群个体数目
	int m=eqTE.size();
	for(int i=0;i<Stime;++i){    //随机搜索time次
	evoluPopu popu(n,m,G,GOR,&eqTE,&eqOR);
		(popu).evolution();
	fprintf(out, "遗传算法1\n%f\t%f\t%f\n\n",popu.hero.mlu1,popu.hero.mlu,popu.hero.delay); 
	}

////////////////遗传算法2  搜索TE的解空间 二进制编码
	G->clearOcc();
	G->GAinit(eqTE);
	int n2=150;//种群个体数目
	int m2=eqTE.size();
	for(int i=0;i<Stime;++i){
	evoluPopubit popubit(n2,m2,G,GOR,&eqTE,&eqOR,ordic);
		(popubit).evolution();
	fprintf(out, "遗传算法2\n%f\t%f\t%f\n\n",popubit.hero.mlu1,popubit.hero.mlu,popubit.hero.delay); 
	}
	*/
	
}
 fclose(out);
	delete G;
	delete GOR;
	system("pause");
	return 0;
	
}


/* graph_ATT

0 1 2 54
1 0 2 54
0 2 1 68
2 0 2 68
0 6 2 55
6 0 3 55
0 7 1 69
7 0 1 69
1 6 1 67
6 1 1 67
2 3 1 68
3 2 1 68
2 6 1 89
6 2 1 89
2 9 2 65
9 2 2 65
2 15 1 61
15 2 2 61
2 16 1 97
16 2 1 97
2 17 1 85
17 2 1 85 
2 20 1 95
20 2 1 95
2 21 1 81
21 2 1 81
3 8 2 52
8 3 2 52
3 9 1 71
9 3 1 71
3 6 1 73
6 3 1 72
4 5 1 61
5 4 2 61
4 7 1 92
7 4 1 92
5 7 1 96
7 5 1 96
5 8 1 93
8 5 1 93
5 9 1 97
9 5 1 97
5 13 2 59
13 5 2 59
5 14 1 59
14 5 2 59
6 7 1 97
7 6 1 97
8 9 1 66
9 8 2 66
8 13 1 74
13 8 1 74
9 13 1 84
13 9 1 84
9 16 1 58
16 9 1 58
9 22 1 61
22 9 1 61
10 11 1 81
11 10 1 81
10 13 1 84
13 10 1 84
10 14 1 77
14 10 1 77
11 12 1 85
12 11 1 85 
11 13 1 61
13 11 1 61
11 14 1 50
14 11 2 50
12 24 1 99
24 12 1 99
12 13 1 87
13 12 1 87
13 15 1 78
15 13 1 78
13 16 1 85
16 13 1 85
13 17 1 90
17 13 1 90
13 22 1 79
22 13 1 79
15 16 1 59
16 15 2 59
15 17 1 86
17 15 1 86
15 21 1 93
21 15 1 93
16 17 1 69
17 16 1 69
17 18 1 85
18 17 1 85
17 19 1 86
19 17 1 86
17 20 1 94
20 17 1 94
17 21 1 76
21 17 1 76
17 22 2 58
22 17 2 58
18 21 1 53
21 18 1 53
19 20 1 94
20 19 1 94
21 22 1 76
22 21 1 76
22 23 2 51
23 22 2 51
22 24 1 98
24 22 1 98
23 24 1 98
24 23 1 98
*/