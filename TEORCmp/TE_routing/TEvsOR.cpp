#include"evolution.h"
#include"evolutionbit.h"
#include"Nash.h"

int main(){
	srand((unsigned)time(NULL));	
	CGraph *G=new CGraph("inputFile//graph52.txt");
	CGraph *GOR=new CGraph("inputFile//graph52OR.txt");

	 vector<demand> eqOR;
	 vector<demand> eqTE;
	 vector<demand> eqbase;
	 FILE *out = fopen("outputFile//objcmp.txt", "a");
	int CASEnum=5;
	for(int casenum=0;casenum<CASEnum;casenum++){

	eqbase.clear();//�������TE��demand,�൱��background�� //�б�����
	for(int i = 0; i < NUMDEMAND; i++){
		int s = rand()%G->n, t;
		do{
			t = rand()%G->n;
		}while(s == t);
		eqbase.push_back(demand(s, t, rand()%(MAXDEMAND)+1));
			}
	////Overlay   ����demand
	eqOR.clear(); //����Overlay�ڵ��֮�����������
	for(int i=0;i<GOR->m;i++)
		eqOR.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,rand()%(MAXDEMAND)+1));


	//////////////////// �٢٢٢٢� Nash ����  ////////////////////////
	eqTE.clear(); 
	for(unsigned int i=0;i<eqOR.size();i++)
	   eqTE.push_back(eqOR[i]);			
	for(unsigned int i=0;i<eqbase.size();i++)
		eqTE.push_back(eqbase[i]);
		fprintf(out, "TEvsOR    ����:\n");
		G->clearOcc();
		for(int demo =1; demo <= 100; demo++){
			double te=0,or=0;
		    te +=G->TEcplex(eqTE,GOR);
			if(te>=INF) { fprintf(out, "TE unfeasible\n");break;}
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

	
	/////////////// �ۢۢۢۢ�  Network Friendly Overlay Routing  /////////////////////////
	eqTE.clear(); //eqTEǰ�����װOverlay����  Ϊ�˱�̷���
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
			
}
    fclose(out);
	delete G;
	delete GOR;
	system("pause");
	return 0;
	
}
