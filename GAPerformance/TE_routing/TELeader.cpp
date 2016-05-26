#define COMMOM_H
#include"evolutionbit.h"
#include"Nash.h"
#include<vld.h> ////Visual Leak Detector �ڴ�й¶��⹤��



int main(){
	
	srand((unsigned)time(NULL));	
	CGraph *G = new CGraph("inputFile//graphATT.txt");
	CGraph *GOR = new CGraph("inputFile//graphATTOR.txt");	
	vector<demand> eqOR;
	vector<demand> eqTE;
	vector<demand> eqbase;
	FILE *out = fopen("outputFile//GAF.txt", "a");
	int CASEnum= 3;
	fprintf(out, "strat\n"); 
	for(int casenum = 0; casenum < CASEnum; casenum++){	
	eqbase.clear();//�������TE��demand,�൱��background�� //�б�����
	for(int i = 0; i < NUMDEMAND; i++){
		int s = rand()%G->n, t;
		do{
			t = rand()%G->n;
		}while(s == t);
		eqbase.push_back(demand(s, t, rand()%(MAXDEMAND)+2));
			}	
	////Overlay   ����demand
	eqOR.clear(); //����Overlay�ڵ��֮�����������
	for(int i=0;i<GOR->m;i++)
		eqOR.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,rand()%(MAXDEMAND)+3));

	/////////////////////�ڢڢڢڢ�  ����  ////////////////////
	////////////TE����  ��Overlayѡ·����Ҳ��TE����	
	eqTE.clear(); //eqTEǰ�����װOverlay����  Ϊ�˱�̷���
	for(unsigned int i=0;i<eqOR.size();i++)
	   eqTE.push_back(eqOR[i]);			
	for(unsigned int i=0;i<eqbase.size();i++)
		eqTE.push_back(eqbase[i]);
	G->clearOcc();
	fprintf(out, "TE dictator\n"); 
	double tedic=0,teor=0;
	tedic = TEdictor(G,eqTE,GOR);
	teor = G->delay;
	fprintf(out, "%f\t%f\n\n",tedic,teor); 
	printf("%f\t%f\n",tedic,teor); 

	/////////OR����  ��TEѡ·����Ҳ��TE����
	fprintf(out, "Overlay dictator\n");
	G->clearOcc();
	double ordic=0,orte=0;
	ordic = ORdictor(G,eqTE,GOR->m);
	orte = G->mlu;
	fprintf(out, "%f\t%f\n\n",orte,ordic); 
	printf("%f\t%f\n",orte,ordic); 
	fprintf(out, "\n"); 

	int Stime = 10;		
	G->clearOcc();
	G->GAinit(eqTE);
	int consider = 0; //������Overlay
	for(int i=0;i<Stime;++i){
		int n2=200;//��Ⱥ������Ŀ
		int m2=eqTE.size();
		evoluPopubit *popubit=new evoluPopubit(n2,m2,G,GOR,&eqTE,&eqOR,ordic,consider);
		(popubit)->evolution();
		fprintf(out, "�Ŵ��㷨\n%f\t%f\n\n",popubit->hero.mlu,popubit->hero.delay);
		printf("�Ŵ��㷨\n%f\t%f\n\n",popubit->hero.mlu,popubit->hero.delay); 
		delete popubit;
	}
	fprintf(out, "\n\n");

	//// ����
	int consider2 = 1;
	for(int i=0;i<Stime;++i){
		int n3=200;//��Ⱥ������Ŀ
		int m3=eqTE.size();
		evoluPopubit popubit(n3,m3,G,GOR,&eqTE,&eqOR,ordic,consider2);
		(popubit).evolution();
		fprintf(out, "�Ŵ��㷨2\n%f\t%f\n\n",popubit.hero.mlu,popubit.hero.delay); 
		printf("�Ŵ��㷨2\n%f\t%f\n\n",popubit.hero.mlu,popubit.hero.delay); 
	}
	fprintf(out, "\n\n"); 	

	double scale = G->LoadScaleFactor(eqTE); // Load Scale Factor
	fprintf(out, " Load Scale Factor\n%f\n\n",scale); 
	printf("Load Scale Factor\n%f\n\n",scale); 

} 
   fclose(out);
	delete G;
	delete GOR;
	cout<< "TE routing " <<endl;
	system("pause");
	return 0;	

}