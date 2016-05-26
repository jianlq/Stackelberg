#ifndef KSP_H
#define KSP_H
#include"CGraph.h"
#include <ilcplex/ilocplex.h>

void CGraph::KSP(int s, int t, unsigned int k)
{
    listSolu.clear();
    listTemp.clear();
   // cout<<"Start!"<<endl;
    Status *beginning = new Status(s,0);
    beginning->passby.push_back(s);
    listTemp.push_back(beginning);
    while(listSolu.size() < k && listTemp.size() != 0)
    {
        listTemp.sort(pStatusComp);///在更新序列里进行排序
        int id = (**listTemp.begin()).ID;///拿出节点序列的第一个点的ID
        if(id == t)
            listSolu.push_back(*listTemp.begin());///如果这个点就是终点，那么把这个状态丢到解集合里头去。
        double d = (**listTemp.begin()).d;///拿出存储的d值
        list<int> passby = (**listTemp.begin()).passby;
        /*cout<<endl;
        cout<<"we chose the situ that id is "<<id<<" d is "<< d<<endl;*/
        vector<CEdge*>::iterator it;
        for(it = mapVID_listEdge[id].begin(); it != mapVID_listEdge[id].end(); it++)///遍历该点的出度边
        {
            int head = (**it).getHead();///拿出头结点ID
            if(find(passby.begin(),passby.end(),head) == passby.end())
            {
                double weight = (**it).getWeight() + d;///拿出该边的重量和d相加得到路径长度
                list<int> templist = passby;
                templist.push_back(head);
                Status *newstatus = new Status(head,weight,*listTemp.begin(),templist);///形成某个状态
                listTemp.push_back(newstatus);///把状态丢进更新序列里
            }
        }
        listTemp.pop_front();
    }

	listPath.clear(); ////////每个OD对之间算的路径集合  必须清空
    list<Status*>::iterator Sit;
    for(Sit = listSolu.begin(); Sit != listSolu.end(); Sit++)
    {
        listPath.push_back(new CPath(beginning, *Sit, mapVID_Vertex, mapVID_listEdge));
    }	    
}


double ORdictor(CGraph *G,vector<demand> &eq,int ornum){    /////  eq的前ornum(GOR->m)个先装Overlay的需求  ////////
    IloEnv env;
    IloModel model(env);
    IloCplex ORsolver(model);

	int num=eq.size();
    IloArray<IloIntVarArray> x(env, num); 
    for(int d = 0; d < num; d++)
		x[d] = IloIntVarArray(env, G->m, 0, 1); // num * G->m 的二维矩阵
	
	IloNumVarArray cost(env,G->m,0.0 ,IloInfinity); 
	if(!CONSTANT){
	IloExpr res(env);
	for(int i=0;i<G->m;i++){
		IloNumExpr load(env);
	     for(int d=0;d<ornum;d++) 
			 load += eq[d].flow*x[d][i];
	model.add(cost[i]>=load/(G->Link[i]->capacity-3.0));   // [1,1/3]
	model.add(cost[i]>=3.0*load/(G->Link[i]->capacity-3.0)-(2.0/3.0));  //[1/3,2/3]
	model.add(cost[i]>=10.0*load/(G->Link[i]->capacity-3.0)-(14.0/3.0)); // [2/3,9/10]
	model.add(cost[i]>=70.0*load/(G->Link[i]->capacity-3.0)-(163.0/3.0));  //[9/10,1]
	model.add(cost[i]>=500*load/G->Link[i]->capacity-(double)(1368/3)); // [1,11/10]
	//model.add(cost[i]>=5000*load/G->Link[i]->capacity-(double)(14548/3)); [11/10,...]
	res += cost[i];
	}
	model.add(IloMinimize(env,res));
	}
	else{
	IloExpr delay(env);
	for(int ij = 0; ij  < G->m; ij++){		
		IloNumExpr ep1(env);
		for(int d = 0; d < ornum; d++){
			ep1 += x[d][ij]*G->Link[ij]->dist;  //d之和			
		}
			delay += ep1;		 
	}
    model.add(IloMinimize(env, delay)); 
	}

    //约束  流量守恒约束  对每个点进行流量守恒约束  
	//这里为eq的所有需求选路  包括TE的背景流
    for(int d = 0; d < num; d++)
		for(int i = 0; i < G->n; i++){    // n为顶点数
			IloExpr constraint(env);
			for(unsigned int k = 0; k < G->adjL[i].size(); k++) // 出度边
				constraint += x[d][G->adjL[i][k]->id];
			for(unsigned int k = 0; k < G->adjRL[i].size(); k++) // 入度边
				constraint -= x[d][G->adjRL[i][k]->id];
			// 出 - 入
			if(i == eq[d].org)
				model.add(constraint == 1);
			else if(i == eq[d].des)
				model.add(constraint == -1);
			else
				model.add(constraint == 0);
		}
	//带宽约束
    for(int i = 0; i < G->m; i++){
		IloExpr constraint(env);
	for(int d = 0; d <  num; d++)
			constraint += eq[d].flow*x[d][i];
	model.add(constraint<=G->Link[i]->capacity);  
     }

	ORsolver.setOut(env.getNullStream());
	double obj = INF;
	ORsolver.solve();
	if(ORsolver.getStatus() == IloAlgorithm::Infeasible)
		 env.out() << "OR Dictator No Solution" << endl;
	else{
		obj = ORsolver.getObjValue();
		//****************************// 计算TE的链路利用率
		double util=0;
		for(int i=0;i<G->m;i++){
			double de=0;
			for(int d=0;d<num;d++)
				de += ORsolver.getValue(x[d][i])*eq[d].flow;
			if(util<de/G->Link[i]->capacity)
				util=de/G->Link[i]->capacity;
		}
		G->mlu=util;
    }
	env.end();
    return obj;
}

double TEdictor(CGraph *G,vector<demand> &eq,CGraph *GOR){    ////////   eq的前面装eqOR  //////////
    IloEnv env;
    IloModel model(env);
    IloCplex TEsolver(model);
	
	int num=eq.size();	
	IloArray<IloIntVarArray> x(env, num); 
	for(int d = 0; d < num; d++)
		x[d] = IloIntVarArray(env, G->m, 0, 1); 	

    //约束  流量守恒约束  对每个点进行流量守恒约束  
	//这里为eq的所有需求选路  包括OR的流
    for(int d = 0; d < num; d++)
		for(int i = 0; i < G->n; i++){    // n为顶点数
			IloExpr constraint(env);
			for(unsigned int k = 0; k < G->adjL[i].size(); k++) // 出度边
				constraint += x[d][G->adjL[i][k]->id];
			for(unsigned int k = 0; k < G->adjRL[i].size(); k++) // 入度边
				constraint -= x[d][G->adjRL[i][k]->id];
			
			if(i == eq[d].org)
				model.add(constraint == 1);
			else if(i == eq[d].des)
				model.add(constraint == -1);
			else
				model.add(constraint == 0);
		}

	for(int i = 0; i < G->m; i++){
		IloExpr constraint(env);
	for(int d = 0; d <  num; d++)
			constraint += eq[d].flow*x[d][i];
	model.add(constraint<=G->Link[i]->capacity);  
     }

	//优化目标 最小化带宽利用率  
	IloNumVar z(env,0,1);
	for(int i = 0; i < G->m; i++){
		IloExpr load(env);
		for(int d = 0; d < num; d++)
		  load += eq[d].flow*x[d][i];
		model.add(load<=z*G->Link[i]->capacity);	
	}
	model.add(IloMinimize(env,z));

	TEsolver.setOut(env.getNullStream());
	double obj = INF;
	TEsolver.solve();
	if(TEsolver.getStatus() == IloAlgorithm::Infeasible)
		 env.out() << "TE Dictator No Solution" << endl;
	else{
		obj = TEsolver.getObjValue();
		
		for(int i=0;i<G->m;i++){ //先计算每条边上的负载
			double de=0;
			for(int d=0;d<num;d++)
				de += TEsolver.getValue(x[d][i])*eq[d].flow;
			G->Link[i]->load=de;
		  }

		if(!CONSTANT){ //排队延迟
		////////直接在Network上计算
		//****************************// 计算Overlay的流量延迟
		double lat=0;
		for(int d=0;d<GOR->m;d++){   ////GOR->m个Overlay的需求
			for(int ij=0;ij<G->m;ij++){
				double del;
				if(G->Link[ij]->load>=G->Link[ij]->capacity) del=1.0;
				else del=1.0/(G->Link[ij]->capacity-G->Link[ij]->load);
				lat += TEsolver.getValue(x[d][ij])*del*eq[d].flow; // 1/(C-x)  * x1  // 其中x1表示Overlay的流需求
			}
		}
		G->delay=lat;
    }
	////////常数延迟
	else{  //计算OR边延迟   常数延迟
		//在Network算
		double lat=0;
		for(int d=0;d<GOR->m;d++){  
			for(int ij=0;ij<G->m;ij++){
			   if(G->Link[ij]->load>=G->Link[ij]->capacity) lat += 100.0;
               else lat += TEsolver.getValue(x[d][ij])*G->Link[ij]->dist;
			}
		}
		G->delay=lat;
    }
}
	env.end();
    return obj;
}

double TEdictorNum(CGraph *G,vector<demand> &eq,CGraph *GOR){    ////////   eq的前面装eqOR  //////////
    IloEnv env;
    IloModel model(env);
    IloCplex TEsolver(model);
	
	int num=eq.size();
	IloArray<IloNumVarArray> x(env, num); 
	for(int d = 0; d < num; d++)
		x[d] = IloNumVarArray(env, G->m, 0, 1); //小数规划

    //约束  流量守恒约束  对每个点进行流量守恒约束  
	//这里为eq的所有需求选路  包括OR的流
    for(int d = 0; d < num; d++)
		for(int i = 0; i < G->n; i++){    // n为顶点数
			IloExpr constraint(env);
			for(unsigned int k = 0; k < G->adjL[i].size(); k++) // 出度边
				constraint += x[d][G->adjL[i][k]->id];
			for(unsigned int k = 0; k < G->adjRL[i].size(); k++) // 入度边
				constraint -= x[d][G->adjRL[i][k]->id];
			
			if(i == eq[d].org)
				model.add(constraint == 1);
			else if(i == eq[d].des)
				model.add(constraint == -1);
			else
				model.add(constraint == 0);
		}

	for(int i = 0; i < G->m; i++){
		IloExpr constraint(env);
	for(int d = 0; d <  num; d++)
			constraint += eq[d].flow*x[d][i];
	model.add(constraint<=G->Link[i]->capacity);  
     }

	//优化目标 最小化带宽利用率  
	IloNumVar z(env,0,1);
	for(int i = 0; i < G->m; i++){
		IloExpr load(env);
		for(int d = 0; d < num; d++)
		  load += eq[d].flow*x[d][i];
		model.add(load<=z*G->Link[i]->capacity);	
	}
	model.add(IloMinimize(env,z));

	TEsolver.setOut(env.getNullStream());
	double obj = INF;
	TEsolver.solve();
	if(TEsolver.getStatus() == IloAlgorithm::Infeasible)
		 env.out() << "TE Dictator No Solution" << endl;
	else{
		obj = TEsolver.getObjValue();
		
		for(int i=0;i<G->m;i++){ //先计算每条边上的负载
			double de=0;
			for(int d=0;d<num;d++)
				de += TEsolver.getValue(x[d][i])*eq[d].flow;
			G->Link[i]->load=de;
		  }

		if(!CONSTANT){ //排队延迟
		////////直接在Network上计算
		//****************************// 计算Overlay的流量延迟
		double lat=0;
		for(int d=0;d<GOR->m;d++){   ////GOR->m个Overlay的需求
			for(int ij=0;ij<G->m;ij++){
				double del;
				if(G->Link[ij]->load>=G->Link[ij]->capacity) del=1.0;
				else del=1.0/(G->Link[ij]->capacity-G->Link[ij]->load);
				lat += TEsolver.getValue(x[d][ij])*del*eq[d].flow; // 1/(C-x)  * x1  // 其中x1表示Overlay的流需求
			}
		}
		G->delay=lat;
    }
	////////常数延迟
	else{  //计算OR边延迟   常数延迟
		//在Network算
		double lat=0;
		for(int d=0;d<GOR->m;d++){  
			for(int ij=0;ij<G->m;ij++){
			   if(G->Link[ij]->load>=G->Link[ij]->capacity) lat += 100.0;
               else lat += TEsolver.getValue(x[d][ij])*G->Link[ij]->dist;
			}
		}
		G->delay=lat;
    }
}
	env.end();
    return obj;
}

#endif