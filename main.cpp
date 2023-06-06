#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include "SimStruct.h"
#include <fstream>
#include <cstring>
#include <unordered_set>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
//#include "alias.h"
//#include "evaluate.h"

void process_mem_usage(double& vm_usage, double& resident_set){
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}


void usage() {
    cerr << "./SetPush"<<endl
	 << "-d <dataset>"<<endl
	 << "-f <filelabel>"<<endl
	 << "-algo <algorithm>"<<endl
     << "[-alpha <alpha> (default 0.2)]"<<endl
	 << "[-er <epsilon> (default 0.5)]"<<endl
 	 << "[-c <damping factor> (default 0.5)]"<<endl
	 << "[-qn <# of query nodes> (default 10)]"<<endl;
}

long check_inc(long i, long max) {
    if (i == max) {
        usage();
        exit(1);
    }
    return i + 1;
}

bool cmp(const double& a, const double& b){
    return a > b;
}


int main(int argc, char **argv){
    long i = 1;
    char *endptr;
    string filename="./dataset/youtube/";
    string filelabel="youtube";
    string algo="SetPush";
    long querynum = 10;
    uint queryFlag = 1;
    double eps_r = 0.5;
    double pf=0.1;
    double alpha=0.2;
      
    while (i < argc) {
        if (!strcmp(argv[i], "-d")) {
            i = check_inc(i, argc);
            filename = argv[i];
        }
        else if (!strcmp(argv[i], "-f")) {
            i = check_inc(i, argc);
            filelabel = argv[i];
        }
        else if (!strcmp(argv[i], "-algo")) {
            i = check_inc(i, argc);
            algo = argv[i];
        } 
        else if (!strcmp(argv[i], "-er")) {
            i = check_inc(i, argc);
            eps_r = strtod(argv[i], &endptr);
            if ((eps_r == 0 || eps_r > 1) && endptr) {
                cerr << "Invalid eps argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-alpha")) {
            i = check_inc(i, argc);
            alpha = strtod(argv[i], &endptr);
            if ((alpha == 0 || alpha > 1) && endptr) {
                cerr << "Invalid t argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-pf")) {
            i = check_inc(i, argc);
            pf = strtod(argv[i], &endptr);
            if ((pf == 0 || pf > 1) && endptr) {
                cerr << "Invalid t argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-qn")) {
            i = check_inc(i, argc);
            querynum = strtod(argv[i], &endptr);
            if ((querynum < -2) && endptr) {
                cerr << "Invalid querynum argument" << endl;
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-qflag")) {
            i = check_inc(i, argc);
            queryFlag = strtod(argv[i], &endptr);
            if ((queryFlag !=1) && (queryFlag!=2) && endptr) {
                cerr << "Invalid querynum argument" << endl;
                exit(1);
            }
        }
        else {
            usage();
            exit(1);
        }
        i++;
    }

    SimStruct sim = SimStruct(filename, filelabel, eps_r, pf, alpha);
    
    if(querynum <0 || querynum > sim.vert){
        querynum=10;
        if(sim.vert<10){
            querynum=sim.vert;
        }
    }

    cout<<"========="<<endl;
    cout<<"Graph dataset: \t\t\t"<<filelabel<<endl;
    cout<<"Algorithm: \t\t\t"<<algo<<endl;
    cout<<"relative error c: \t\t"<<eps_r<<endl;
    cout<<"failure probability pf: \t"<<pf<<endl;
    cout<<"damping factor alpha: \t\t"<<alpha<<endl;
    cout<<"# of query nodes: \t\t"<<querynum<<endl;
    //cout<<"queryFlag="<<queryFlag<<endl;
    cout<<endl;
    
    
    if(algo == "SetPush"){
        string queryname;
        if(queryFlag==1){
            queryname = "query/" + filelabel + ".query";
            cout<<"Input query nodes in the Q1 query set. Every query node is uniformly sampled from G. "<<endl;
        }
        else if(queryFlag==2){
            queryname = "query/" + filelabel + "_degree.query";
            cout<<"Input query nodes in the Q2 query set. Every query node is sampled according to the degree distribution. "<<endl;
        }
        ifstream query(queryname);
        
        if(!query){
            uint* check=new uint[sim.vert]();
            ofstream data_idx(queryname);
            if(queryFlag==1){
                cout<<"Generate the Q1 query set first ... "<<endl;
                //uint* check=new uint[sim.vert]();
                //ofstream data_idx(queryname);
                for(uint i = 0; i < querynum; i++){
                    uint tmpnode=sim.R.generateRandom()%sim.vert;
                    while((sim.g.getInSize(tmpnode)==0)||(check[tmpnode]==1)){
                        tmpnode=sim.R.generateRandom()%sim.vert;
                        //cout<<"repeated tmpnode="<<tmpnode<<" outsize="<<sim.g.getOutSize(tmpnode)<<endl;
                    }
                    check[tmpnode]=1;
                    data_idx<<tmpnode<<"\n";
                }
                //data_idx.close();
            }
            else if(queryFlag==2){
                cout<<"Generate the Q2 query set first ... "<<endl;
                vector<pair<pair<uint,uint>, double > > aliasD;
                for(uint i=0; i<sim.vert;i++){
                    aliasD.push_back(make_pair(make_pair(i,i),(sim.g.getOutSize(i)/(double)sim.g.m)));
                }
                Alias alias = Alias(aliasD);
                for(uint i = 0; i < querynum; i++){
                    pair<uint,uint> tempPair = alias.generateRandom(sim.R);
                    uint tempnode=tempPair.first;
                    while((sim.g.getInSize(tempnode)==0)||(check[tempnode]==1)){
                        tempPair=alias.generateRandom(sim.R);
                        tempnode=tempPair.first;
                        //cout<<"repeated tmpnode="<<tempnode<<" outsize="<<sim.g.getOutSize(tempnode)<<endl;
                    }
                    check[tempnode]=1;
                    data_idx << tempnode<<"\n";
                    //cout<<tempnode<<" "<<sim.g.getOutSize(tempnode)<<endl;
                }
            }
            data_idx.close();
            ifstream query(queryname);
            if(!query){
                cout<<"ERROR: Fail to generate the query set. "<<endl;
            }
        }
        stringstream ss_dir,ss;
        ss_dir<<"result/"<<alpha<<"/"<<filelabel<<"/";
        mkpath(ss_dir.str());
	
        if(queryFlag==1){
            ss<<ss_dir.str()<<eps_r<<".txt";
        }
        else if(queryFlag==2){
            ss<<ss_dir.str()<<eps_r<<"_degree.txt";
        }
        
        cout<<"The output file is: "<<ss.str()<<endl;
        ofstream fout;
        fout.open(ss.str());
        fout.setf(ios::fixed,ios::floatfield);
        fout.precision(15);
        if(!fout){
            cout<<"Fail to open the output file"<<endl;
        }

        for(long i = 0; i < querynum; i++){
            long nodeId;
            if(querynum==sim.vert){
                nodeId=i;
            }
            else{
                query >> nodeId;
            }
            cout<<i<<": "<<nodeId<<endl;
            clock_t t0=clock();
            for(uint ite=0; ite<sim.median_ite; ite++){
                sim.SetPush(nodeId);
                sim.median_counter[ite]=sim.counter;
            }
            sort(sim.median_counter, (sim.median_counter+sim.median_ite));
            clock_t t1=clock();
            sim.avg_time+=(t1-t0)/(double)CLOCKS_PER_SEC;
            //cout<<"median_ite = "<<sim.median_ite<<endl;
            //for(uint ite=0; ite<sim.median_ite; ite++){
            //    cout<<"median_counter["<<ite<<"]="<<sim.median_counter[ite]<<endl;
            //}
            fout<<nodeId<<" "<<sim.median_counter[sim.median_ite/2]<<"\n";
            cout<<"PageRank: "<<sim.median_counter[sim.median_ite/2]<<endl;
            cout<<"query time: "<<(t1-t0)/(double)CLOCKS_PER_SEC<<" s"<<endl;
        }
        fout.close();
        
        cout << "Average query time: "<<sim.avg_time/ (double) querynum <<" s"<<endl;
        cout << "=== The single-node PageRank query on the "<<filelabel<<" dataset has been completed. ==="<<endl<<endl<<endl;
        query.close();
    }
    return 0;
}
