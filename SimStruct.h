#ifndef SIMSTRUCT_H
#define SIMSTRUCT_H

#define INT_MAX 32767

#include <vector>
#include <algorithm>
#include <queue>
#include <functional>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <fstream>
#include "Graph.h"
#include "Random.h"
#include "alias.h"
#include <unordered_map>
#include <unordered_set>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <queue>
#include <cmath>
#include <random>
#include <ctime>


bool comp1(const uint &a, const uint &b) {
	return a < b;
}

bool maxScoreCmp(const pair<uint, double>& a, const pair<uint, double>& b){
    return a.second > b.second;
}

class SimStruct{
    public:
        Graph g;//Class Graph
        Random R;//Class Random
        uint vert;//# of vertice
        string filelabel;
        double alpha;
        double eps_r;
        double pf;
        double avg_time;
        //double *H[2];//hash map,the same as U,C,UC
        uint *H[2];
        uint * U[2];
        uint *candidate_set[2];
        uint candidate_count[2];
        double *residue[2];
        double *finalReserve;
        //bool* isInArray;
        uint finalreserve_count;
        double counter;
        uint median_ite;
        double *median_counter;
        uint seed;

    SimStruct(string name, string file_label, double epsilon_r, double para_pf, double para_alpha) {
        filelabel = file_label;
        g.inputGraph(name,file_label);
        R = Random();
        vert = g.n;
        alpha=para_alpha;
        eps_r = epsilon_r;
        pf=para_pf;
        avg_time = 0;
        candidate_count[0]=0;
        candidate_count[1]=0;
        H[0] = new uint[vert];
        H[1] = new uint[vert];
        U[0] = new uint[vert];
        U[1] = new uint[vert];
        candidate_set[0] = new uint[vert];
        candidate_set[1] = new uint[vert];
        residue[0]=new double[vert];
        residue[1]=new double[vert];
        finalReserve = new double[vert];
        //isInArray = new bool[vert];
        finalreserve_count=0;
        for(uint i = 0; i < vert; i++){
            //isInArray[i] = false;
            residue[0][i]=0;
            residue[1][i]=0;
            finalReserve[i]=0;
            H[0][i] = 0;
            H[1][i] = 0;
            U[0][i] = 0;
            U[1][i] = 0;
            candidate_set[0][i]=0;
            candidate_set[1][i]=0;
        }
        median_ite=((uint)log(1.0/para_pf) > 1? (uint)log(1.0/para_pf):1);
        //median_ite=1;
        median_counter=new double[median_ite];
        counter=0;
        for(uint i=0; i<median_ite; i++){
            median_counter[i]=0;
        }
        //cout << "====init done!====" << endl;
    }
    
    ~SimStruct() {
        delete[] H[0];
        delete[] H[1];
        delete[] U[0];
        delete[] U[1];
        delete[] residue[0];
        delete[] residue[1];
        delete[] finalReserve;
        //delete[] isInArray;
    	delete[] candidate_set[0];
        delete[] candidate_set[1];
        delete[] median_counter;
    }


    void SetPush(uint u){
        uint tempLevel = 0;
        double dt=g.getOutSize(u);
        seed=(uint)time(0);
        std::default_random_engine generator(seed);

        residue[0][u] = 1;
        candidate_set[0][0]=u;
        candidate_count[0]=1;

        uint L=(uint)ceil(log(eps_r*alpha/(double)vert/2.0)/log(1-alpha))+1;
        //uint L=50;
        //cout<<"L="<<L<<endl;

        double theta_1=alpha*eps_r*eps_r/12.0/L/dt;
        double theta_2=alpha*eps_r*eps_r*sqrt(2.0*(1-alpha)/g.m)/12.0/L;
        double tun_eps = theta_1>theta_2 ? theta_1:theta_2;
        //cout<<"theta_1="<<theta_1<<" theta_2="<<theta_2<<" tun_eps="<<tun_eps<<endl;

        while(tempLevel<=L){
            uint tempLevelID=tempLevel%2;
            uint newLevelID=(tempLevel+1)%2;
            uint candidateCnt=candidate_count[tempLevelID];
            //cout<<"Iteration "<<tempLevel<<": candidateCnt="<<candidateCnt<<endl;
            if(candidateCnt==0){
                //cout<<"candidateCnt=0 tempLevel="<<tempLevel<<endl;
                break;
            }
            candidate_count[tempLevelID]=0;
            //cout<<"tempLevelID="<<tempLevelID<<" newLevelID="<<newLevelID<<endl;
            for(uint j = 0; j < candidateCnt; j++){
                uint tempNode = candidate_set[tempLevelID][j];
                double tempR = residue[tempLevelID][tempNode];
                U[tempLevelID][tempNode]=0;
                residue[tempLevelID][tempNode] = 0;
                if(H[1][tempNode] == 0){
                    H[0][finalreserve_count++] = tempNode;
                    H[1][tempNode] = 1;
                }
                finalReserve[tempNode]+=alpha*tempR;
	        
                if(tempLevel==L){
                    continue;
                }
                uint outSize = g.getOutSize(tempNode);
		
                double incre=tempR*(1-alpha)/outSize;

                if(incre>=tun_eps){
                    for(uint k = 0; k < outSize; k++){
                        uint newNode = g.getOutVert(tempNode, k);
                        residue[newLevelID][newNode] += incre;
                        if(U[newLevelID][newNode] == 0){
                            U[newLevelID][newNode] = 1;
                            candidate_set[newLevelID][candidate_count[newLevelID]++]=newNode;
                        }
                    }
                }
                else{
                    double sampling_pr=incre/tun_eps;
                    std::binomial_distribution<uint> distribution(outSize,(double)sampling_pr);
                    uint expn=distribution(generator);
			
                    if(expn==outSize){
                        for(uint k = 0; k < outSize; k++){
                            uint newNode = g.getOutVert(tempNode, k);
                            residue[newLevelID][newNode] += incre;
                            if(U[newLevelID][newNode] == 0){
                                U[newLevelID][newNode] = 1;
                                candidate_set[newLevelID][candidate_count[newLevelID]++]=newNode;
                            }
                        }
                    }
                    else{
                        for(uint ri=0;ri<expn;ri++){
                            uint tmpran=(uint)(R.drand()*(outSize-ri));
                            uint idchangefar=g.outPL[tempNode]+ri+tmpran;
                            uint idchangenear=g.outPL[tempNode]+ri;
                            uint tmpchange=g.outEL[idchangefar];
                            g.outEL[idchangefar]=g.outEL[idchangenear];
                            g.outEL[idchangenear]=tmpchange;
                            uint newNode=g.getOutVert(tempNode,ri);
                            residue[newLevelID][newNode] += tun_eps;
                            if(U[newLevelID][newNode] == 0){
                                U[newLevelID][newNode] = 1;
                                candidate_set[newLevelID][candidate_count[newLevelID]++]=newNode;
                            }
                        }
                    }
                }
            }
            tempLevel++;
        }
	
        counter=0;
        for(uint j = 0; j < finalreserve_count; j++){
            uint tempS=H[0][j];
            counter+=dt*finalReserve[tempS]/(double)g.getOutSize(tempS);
            finalReserve[H[0][j]] = 0;
            H[1][H[0][j]] = 0;
        }
        finalreserve_count=0;
        counter/=(double)vert;
    }
};


#endif
