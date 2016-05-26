#ifndef NASH_H
#define NASH_H
#include "Common.h"
#include "CGraph.h"
#include <ilcplex/ilocplex.h>
////////////////////////  Nash  TE  this����G
double CGraph::TEcplex(vector<demand> &eq,CGraph *GOR){
    IloEnv env; //The environment handles memory management of the model and algorithm objects // �ڴ�й¶
    IloModel model(env);
    IloCplex TEsolver(model);

	int totalnum=eq.size();
    IloArray<IloIntVarArray> x(env, totalnum);	
    for(int d=0; d < totalnum; d++)
		x[d] = IloIntVarArray(env, this->m, 0, 1); 

    //Լ��1  �����غ�Լ��  ��ÿ������������غ�Լ��
   for(int d = 0; d < totalnum; d++)
		for( int i = 0; i < this->n; i++){    
			IloExpr constraint(env);
			for(unsigned int k = 0; k <adjL[i].size(); k++) // ���ȱ�
				constraint += x[d][adjL[i][k]->id];
			for(unsigned int k = 0; k <adjRL[i].size(); k++) // ��ȱ�
				constraint -= x[d][adjRL[i][k]->id];	

			if(i == (eq)[d].org) 
				model.add(constraint == 1);
			else if(i == (eq)[d].des)
				model.add(constraint == -1);
			else
				model.add(constraint == 0);
		}
//Լ��2 ����Լ��
  for(int i = 0; i < this->m; i++){
		IloExpr constraint(env);
		for(int d = 0; d <  totalnum; d++)
			constraint += eq[d].flow*x[d][i];
	model.add(constraint<=Link[i]->capacity);  
  }
	//�Ż�Ŀ�� ��С������������  
	IloNumVar z(env,0,1);
	for(int i = 0; i < this->m; i++){
		IloExpr load(env);
		for(int d = 0; d < totalnum; d++)
		  load += eq[d].flow*x[d][i];
		model.add(load<=z*Link[i]->capacity);	
	}
	model.add(IloMinimize(env,z));

    TEsolver.setOut(env.getNullStream());
	double obj = INF;
	TEsolver.solve();
	if(TEsolver.getStatus() == IloAlgorithm::Infeasible)
		 env.out() << "TE No Solution" << endl;
	else{
	   	obj=TEsolver.getObjValue();		
	//////////////////////////////////////   ��������Overlay	//////////////////////////////////////
	if(!CONSTANT){     //�ӳٷǳ���   1.0/(C-x)
		loadx.clear();
		for(int ij=0;ij<this->m;ij++){
			double la=0;
			for(int d=0;d<totalnum;d++)
				la += TEsolver.getValue(x[d][ij])*eq[d].flow; //link�ϵĸ���
			loadx.push_back(la);
			if(la>=Link[ij]->capacity) Link[ij]->latency=1.0;
			else Link[ij]->latency=1.0/(Link[ij]->capacity-la);  ////////////////������Ϊ��λ�ӳ�   2016-1-13   !!!!!!!
			}
		//����OR���ӳ�
		  for(int m=0;m<GOR->m;m++){	
			 bool flag=false;
			 double dmin=0;//�ӳ�
			for(int d=0;d<totalnum;d++){
				if(GOR->Link[m]->tail==eq[d].org && GOR->Link[m]->head==eq[d].des && (eq[d].flow>0) ){ //OR����������
					if(flag==false){
					//cout<<m<<" time "<<"flag"<<endl;
					flag=true;
					for(int ij=0;ij<this->m;ij++)
						dmin += TEsolver.getValue(x[d][ij])*Link[ij]->latency; 
			      }
				}
			  if(flag==true){
				 GOR->Link[m]->dist=dmin;
				 break;
			}
         }  
	}
}
	else{    //����OR���ӳ�   �����ӳ�
		  for(int m=0;m<GOR->m;m++){	
			 bool flag=false;
			 double dmin=0;//�ӳ�
			for(int d=0;d<totalnum;d++)
			{
				if(GOR->Link[m]->tail==eq[d].org && GOR->Link[m]->head==eq[d].des && (eq[d].flow>0) ){ //OR����������
					if(flag==false){
					//cout<<m<<" time "<<"flag"<<endl;
					flag=true;
					for(int ij=0;ij<this->m;ij++)
						dmin += TEsolver.getValue(x[d][ij])*Link[ij]->dist; 
			      }
				}
			 if(flag==true){
				 GOR->Link[m]->dist=dmin;
				 break;
			}
         }	
		}
    }
}
   env.end();
    return obj;
}

/////////////////////////////Nash Overlay 
//�����Ա����double ORcplex(vector<demand> &eqO)
double CGraph::ORcplex(vector<demand> *eqO){
    IloEnv env;
    IloModel model(env);
    IloCplex ORsolver(model);

	int num=eqO->size();
    IloArray<IloIntVarArray> x(env, num); //IloNumVarArray ���ԣ����з��������ߺ�ΪС��
    for(int d = 0; d < num; d++)
		x[d] = IloIntVarArray(env, this->m, 0, 1); 

    //�Ż�Ŀ��  ��С���ӳ�֮��  
    IloExpr delay(env);
	for(int ij = 0; ij  < this->m; ij++){		
		IloNumExpr ep1(env);
		for(int d = 0; d < num; d++){
			if(CONSTANT) ep1 += x[d][ij] * this->Link[ij]->dist;
			else
				ep1 += x[d][ij] * this->Link[ij]->dist*(*eqO)[d].flow;  //d*f֮��		
		}	
			delay += ep1;		 		
	}
	model.add(IloMinimize(env, delay)); 

    //Լ��2  �����غ�Լ��  ��ÿ������������غ�Լ��
    for(int d = 0; d < num; d++)
		for(unsigned int i = 0; i < ver.size(); i++){    // ver.size()Ϊ������,ver[i]Ϊ������
			IloExpr constraint(env);
			for(unsigned int k = 0; k < adjL[ver[i]].size(); k++) // ���ȱ�
				constraint += x[d][adjL[ver[i]][k]->id];
			for(unsigned int k = 0; k < adjRL[ver[i]].size(); k++) // ��ȱ�
				constraint -= x[d][adjRL[ver[i]][k]->id];		

			if(ver[i] == (*eqO)[d].org) 
				model.add(constraint == 1);
			else if(ver[i] == (*eqO)[d].des)
				model.add(constraint == -1);
			else
				model.add(constraint == 0);
		}

	ORsolver.setOut(env.getNullStream());
	double obj = INF;
	ORsolver.solve();
	if(ORsolver.getStatus() == IloAlgorithm::Infeasible)
		 env.out() << "OR No Solution" << endl;
	else{
		obj = ORsolver.getObjValue();
		//****************************//
		//*********��������TE*********//
		eqfo.clear();
	    loadx.clear(); //ÿ�����ϵĸ���
		for(int i=0;i<this->m;i++){
			double de=0;
			for(int d=0;d<num;d++){	
					de += ORsolver.getValue(x[d][i])*(*eqO)[d].flow;
			}
			loadx.push_back(de);   // for  GAS calAbility
			//if(de>0)  // for nash	  ����Ҳ������nash�ж��м���eq[d].flow>0  ��95  ��73
			eqfo.push_back(demand(Link[i]->tail,Link[i]->head,de));	
		}	
   }
   env.end();
   return obj;
}

//�����ȫ�ֺ���
// Bargaining  TE-aware   Network Friendly
// from paper  P7
// Preemptive Strategies to Improve Routing Performance of Native and Overlay Layers
double ORBcplex(CGraph *GOR, vector<demand> &eqO){
    IloEnv env;
    IloModel model(env);
    IloCplex ORBsolver(model);

    //���� X[d][m] demand d �ڵ�m ������
	int num=eqO.size();
    IloArray<IloNumVarArray> x(env, num); //IloNumVarArray ���ԣ����з��������ߺ�ΪС��
    for(int d = 0; d < num; d++)
		x[d] = IloNumVarArray(env, GOR->m, 0, 1); 
	
   //�Ż�Ŀ��1  ��С���ӳ�֮��  
    //�Ż�Ŀ��  ��С���ӳ�֮��  
    IloExpr delay(env);
	for(int ij = 0; ij  < GOR->m; ij++){		
		IloNumExpr ep1(env);
		for(int d = 0; d < num; d++){
			if(CONSTANT) ep1 += x[d][ij] * GOR->Link[ij]->dist;
			else
				ep1 += x[d][ij] * GOR->Link[ij]->dist*(eqO)[d].flow;  //d*f֮��		
		}	
			delay += ep1;		 		
	}
	model.add(IloMinimize(env, delay)); 
	
   // friendly ���ӵ�Լ��
	for(int i=0;i<GOR->m;i++){
		IloExpr load(env);
	for(int d =0;d< num;d++)
		load += x[d][i]*eqO[d].flow;
	model.add(load<=1.3*GOR->Link[i]->load); //��һ��loadΪ����
	}

    //Լ��  �����غ�Լ��  ��ÿ������������غ�Լ��
    for(int d = 0; d < num; d++)
		for(int i = 0; i < GOR->ver.size(); i++){    // ver.size()Ϊ������,ver[i]Ϊ������
			IloExpr constraint(env);
			for(unsigned int k = 0; k < GOR->adjL[GOR->ver[i]].size(); k++) // ���ȱ�
				constraint += x[d][GOR->adjL[GOR->ver[i]][k]->id];
			for(unsigned int k = 0; k < GOR->adjRL[GOR->ver[i]].size(); k++) // ��ȱ�
				constraint -= x[d][GOR->adjRL[GOR->ver[i]][k]->id];
			
			if(GOR->ver[i] == eqO[d].org)
				model.add(constraint == 1);
			else if(GOR->ver[i] == eqO[d].des)
				model.add(constraint == -1);
			else
				model.add(constraint == 0);
		}
	
	ORBsolver.setOut(env.getNullStream());
	double obj = INF;
	ORBsolver.solve();
	if(ORBsolver.getStatus() == IloAlgorithm::Infeasible)
		 env.out() << "ORB No Solution" << endl;
	else{	
		obj = ORBsolver.getObjValue();
		//****************************//
		//*********��������TE*********//
		GOR->eqfo.clear();
		for(int i=0;i<GOR->m;i++){
			double de=0;
			for(int d=0;d<num;d++){
					de += ORBsolver.getValue(x[d][i])*eqO[d].flow;
			}
			GOR->eqfo.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,de));
			if(de>0){
			  //********��GOR link�ϵ�load *********//
		    GOR->Link[i]->load=de; //����link�Ϸ�����������·����һ���������������Ϊ��ֵ
			}
		}	
    }
	env.end();
    return obj;
}

#endif
