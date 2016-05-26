#ifndef NASH_H
#define NASH_H
#include "Common.h"
#include "CGraph.h"
#include <ilcplex/ilocplex.h>
////////////////////////  Nash  TE  this代表G
double CGraph::TEcplex(vector<demand> &eq,CGraph *GOR){
    IloEnv env; //The environment handles memory management of the model and algorithm objects // 内存泄露
    IloModel model(env);
    IloCplex TEsolver(model);

	int totalnum=eq.size();
    IloArray<IloIntVarArray> x(env, totalnum);	
    for(int d=0; d < totalnum; d++)
		x[d] = IloIntVarArray(env, this->m, 0, 1); 

    //约束1  流量守恒约束  对每个点进行流量守恒约束
   for(int d = 0; d < totalnum; d++)
		for( int i = 0; i < this->n; i++){    
			IloExpr constraint(env);
			for(unsigned int k = 0; k <adjL[i].size(); k++) // 出度边
				constraint += x[d][adjL[i][k]->id];
			for(unsigned int k = 0; k <adjRL[i].size(); k++) // 入度边
				constraint -= x[d][adjRL[i][k]->id];	

			if(i == (eq)[d].org) 
				model.add(constraint == 1);
			else if(i == (eq)[d].des)
				model.add(constraint == -1);
			else
				model.add(constraint == 0);
		}
//约束2 带宽约束
  for(int i = 0; i < this->m; i++){
		IloExpr constraint(env);
		for(int d = 0; d <  totalnum; d++)
			constraint += eq[d].flow*x[d][i];
	model.add(constraint<=Link[i]->capacity);  
  }
	//优化目标 最小化带宽利用率  
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
	//////////////////////////////////////   传参数给Overlay	//////////////////////////////////////
	if(!CONSTANT){     //延迟非常数   1.0/(C-x)
		loadx.clear();
		for(int ij=0;ij<this->m;ij++){
			double la=0;
			for(int d=0;d<totalnum;d++)
				la += TEsolver.getValue(x[d][ij])*eq[d].flow; //link上的负载
			loadx.push_back(la);
			if(la>=Link[ij]->capacity) Link[ij]->latency=1.0;
			else Link[ij]->latency=1.0/(Link[ij]->capacity-la);  ////////////////传参数为单位延迟   2016-1-13   !!!!!!!
			}
		//计算OR边延迟
		  for(int m=0;m<GOR->m;m++){	
			 bool flag=false;
			 double dmin=0;//延迟
			for(int d=0;d<totalnum;d++){
				if(GOR->Link[m]->tail==eq[d].org && GOR->Link[m]->head==eq[d].des && (eq[d].flow>0) ){ //OR边在需求中
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
	else{    //计算OR边延迟   常数延迟
		  for(int m=0;m<GOR->m;m++){	
			 bool flag=false;
			 double dmin=0;//延迟
			for(int d=0;d<totalnum;d++)
			{
				if(GOR->Link[m]->tail==eq[d].org && GOR->Link[m]->head==eq[d].des && (eq[d].flow>0) ){ //OR边在需求中
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
//用类成员函数double ORcplex(vector<demand> &eqO)
double CGraph::ORcplex(vector<demand> *eqO){
    IloEnv env;
    IloModel model(env);
    IloCplex ORsolver(model);

	int num=eqO->size();
    IloArray<IloIntVarArray> x(env, num); //IloNumVarArray 可以，进行分流，决策后为小数
    for(int d = 0; d < num; d++)
		x[d] = IloIntVarArray(env, this->m, 0, 1); 

    //优化目标  最小化延迟之和  
    IloExpr delay(env);
	for(int ij = 0; ij  < this->m; ij++){		
		IloNumExpr ep1(env);
		for(int d = 0; d < num; d++){
			if(CONSTANT) ep1 += x[d][ij] * this->Link[ij]->dist;
			else
				ep1 += x[d][ij] * this->Link[ij]->dist*(*eqO)[d].flow;  //d*f之和		
		}	
			delay += ep1;		 		
	}
	model.add(IloMinimize(env, delay)); 

    //约束2  流量守恒约束  对每个点进行流量守恒约束
    for(int d = 0; d < num; d++)
		for(unsigned int i = 0; i < ver.size(); i++){    // ver.size()为顶点数,ver[i]为顶点编号
			IloExpr constraint(env);
			for(unsigned int k = 0; k < adjL[ver[i]].size(); k++) // 出度边
				constraint += x[d][adjL[ver[i]][k]->id];
			for(unsigned int k = 0; k < adjRL[ver[i]].size(); k++) // 入度边
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
		//*********传参数给TE*********//
		eqfo.clear();
	    loadx.clear(); //每条边上的负载
		for(int i=0;i<this->m;i++){
			double de=0;
			for(int d=0;d<num;d++){	
					de += ORsolver.getValue(x[d][i])*(*eqO)[d].flow;
			}
			loadx.push_back(de);   // for  GAS calAbility
			//if(de>0)  // for nash	  但是也可以在nash判断中加入eq[d].flow>0  行95  行73
			eqfo.push_back(demand(Link[i]->tail,Link[i]->head,de));	
		}	
   }
   env.end();
   return obj;
}

//这个是全局函数
// Bargaining  TE-aware   Network Friendly
// from paper  P7
// Preemptive Strategies to Improve Routing Performance of Native and Overlay Layers
double ORBcplex(CGraph *GOR, vector<demand> &eqO){
    IloEnv env;
    IloModel model(env);
    IloCplex ORBsolver(model);

    //变量 X[d][m] demand d 在第m 条边上
	int num=eqO.size();
    IloArray<IloNumVarArray> x(env, num); //IloNumVarArray 可以，进行分流，决策后为小数
    for(int d = 0; d < num; d++)
		x[d] = IloNumVarArray(env, GOR->m, 0, 1); 
	
   //优化目标1  最小化延迟之和  
    //优化目标  最小化延迟之和  
    IloExpr delay(env);
	for(int ij = 0; ij  < GOR->m; ij++){		
		IloNumExpr ep1(env);
		for(int d = 0; d < num; d++){
			if(CONSTANT) ep1 += x[d][ij] * GOR->Link[ij]->dist;
			else
				ep1 += x[d][ij] * GOR->Link[ij]->dist*(eqO)[d].flow;  //d*f之和		
		}	
			delay += ep1;		 		
	}
	model.add(IloMinimize(env, delay)); 
	
   // friendly 增加的约束
	for(int i=0;i<GOR->m;i++){
		IloExpr load(env);
	for(int d =0;d< num;d++)
		load += x[d][i]*eqO[d].flow;
	model.add(load<=1.3*GOR->Link[i]->load); //第一轮load为无穷
	}

    //约束  流量守恒约束  对每个点进行流量守恒约束
    for(int d = 0; d < num; d++)
		for(int i = 0; i < GOR->ver.size(); i++){    // ver.size()为顶点数,ver[i]为顶点编号
			IloExpr constraint(env);
			for(unsigned int k = 0; k < GOR->adjL[GOR->ver[i]].size(); k++) // 出度边
				constraint += x[d][GOR->adjL[GOR->ver[i]][k]->id];
			for(unsigned int k = 0; k < GOR->adjRL[GOR->ver[i]].size(); k++) // 入度边
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
		//*********传参数给TE*********//
		GOR->eqfo.clear();
		for(int i=0;i<GOR->m;i++){
			double de=0;
			for(int d=0;d<num;d++){
					de += ORBsolver.getValue(x[d][i])*eqO[d].flow;
			}
			GOR->eqfo.push_back(demand(GOR->Link[i]->tail,GOR->Link[i]->head,de));
			if(de>0){
			  //********改GOR link上的load *********//
		    GOR->Link[i]->load=de; //对于link上分配了流的链路，下一轮最大允许负载设置为此值
			}
		}	
    }
	env.end();
    return obj;
}

#endif
