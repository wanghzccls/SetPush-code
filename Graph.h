#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <cstring>
#include <unordered_set>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
//#include <cstring>

using namespace std;

typedef unsigned int uint;

int mkpath(string s, mode_t mode=0755){
    size_t pre=0, pos;
    string dir;
    int mdret;
    if(s[s.size()-1]!='/'){
        s+='/';
    }
    while((pos=s.find_first_of('/',pre))!=string::npos){
        dir=s.substr(0,pos++);
        pre=pos;
        if(dir.size()==0) continue;
        if((mdret=::mkdir(dir.c_str(),mode)) && errno!=EEXIST){
            return mdret;
        }
    }
    return mdret;
}

class Graph{
public:
	uint n;	//the number of graph vertices
	uint m;	//the number of graph edges

    uint* outEL;
    uint* outPL;
    uint* inEL;
    uint* inPL;
	
	Graph(){
		
	}

	~Graph(){

	}
    
    void csrGraphChange(string filedir, string filelabel){
        original_inputGraph(filedir,filelabel);
        csr_convert(filedir,filelabel);
    }

	void inputGraph(string filedir, string filelabel){
    		stringstream ss_dir,ss_attr,ss_outEL,ss_outPL,ss_inEL,ss_inPL;
    
    		ss_attr<<filedir<<filelabel<<".attribute";
    		ifstream in_attr;
    		in_attr.open(ss_attr.str());
    		if(!in_attr){
                //cout<<"attribute file open fail!"<<endl;
                //return 0;
                csrGraphChange(filedir,filelabel);
                in_attr.open(ss_attr.str());
                if(!in_attr){
                    cout<<"===ERROR: fail to read the graph structure. ==="<<endl;
                    return;
                }
    		}
    
    		//int n,m;
    		string tmp;
    		in_attr>>tmp>>n;
    		in_attr>>tmp>>m;
    		in_attr.close();

            clock_t st=clock();
    		ss_outEL<<filedir<<filelabel<<".outEdges";
    		ss_outPL<<filedir<<filelabel<<".outPtr";
    		ss_inEL<<filedir<<filelabel<<".inEdges";
    		ss_inPL<<filedir<<filelabel<<".inPtr";
    
    		outEL=new uint[m];
    		outPL=new uint[n+1];
    		inEL=new uint[m];
    		inPL=new uint[n+1];

    		ifstream outf(ss_outEL.str(),ios::in | ios::binary);
    		outf.read((char *)&outEL[0],sizeof(outEL[0])*m);

	    	ifstream outpf(ss_outPL.str(),ios::in | ios::binary);
            outpf.read((char *)&outPL[0],sizeof(outPL[0])*(n+1));

    		ifstream inf(ss_inEL.str(),ios::in | ios::binary);
    		inf.read((char *)&inEL[0],sizeof(inEL[0])*m);
 
    		ifstream inpf(ss_inPL.str(),ios::in | ios::binary);
    		inpf.read((char *)&inPL[0],sizeof(inPL[0])*(n+1));
 
            //cout<<inPL[0]<<" "<<inPL[1]<<" "<<inPL[2]<<endl;
    		outf.close();
    		outpf.close();
    		inf.close();
    		inpf.close();
	
            clock_t se=clock();
            cout<<"Graph input has been completed in "<<(se-st)/(double)CLOCKS_PER_SEC<<" s"<<endl;
            cout<<"# of vertices (n) = "<<n<<endl;
            cout<<"# of edges (m) = "<<m<<endl;
    		//cout<<outEL[0]<<" "<<outEL[1]<<" "<<outEL[2]<<" "<<outEL[3]<<endl;
    		//cout<<inEL[0]<<" "<<inEL[1]<<" "<<inEL[2]<<" "<<inEL[3]<<endl;
	}


	uint getInSize(uint vert){
		return (inPL[vert+1]-inPL[vert]);
	}
	uint getInVert(uint vert, uint pos){
		//return inAdjList[vert][pos];
		return inEL[(inPL[vert]+pos)];
	}
	uint getOutSize(uint vert){
		return (outPL[vert+1]-outPL[vert]);
	}
	uint getOutVert(uint vert, uint pos){
		//return outAdjList[vert][pos];
		return outEL[(outPL[vert]+pos)];
	}


private:
    uint** inAdjList;
    uint** outAdjList;
    uint* indegree;
    uint* outdegree;
    
    void original_inputGraph(string filedir, string filelabel){
        m=0;
        string filename="dataset/"+filelabel+".txt";
        ifstream infile;
        infile.open(filename);
        if(!infile){
            cout<<"ERROR: unable to open the raw graph dataset: "<<filename<<endl;
            return;
        }

        cout<<"Read the raw graph dataset..."<<endl;
        infile >> n;

        indegree=new uint[n];
        outdegree=new uint[n];
        for(uint i=0;i<n;i++){
            indegree[i]=0;
            outdegree[i]=0;
        }
        //read graph and get degree info
        uint from;
        uint to;
        while(infile>>from>>to){
            outdegree[from]++;
            indegree[to]++;
        }

        inAdjList=new uint*[n];
        outAdjList=new uint*[n];

        uint* pointer_in=new uint[n];
        uint* pointer_out=new uint[n];
        for(uint i=0;i<n;i++){
            inAdjList[i]=new uint[indegree[i]];
            outAdjList[i]=new uint[outdegree[i]];
            
            pointer_in[i]=0;
            pointer_out[i]=0;
        }
        infile.clear();
        infile.seekg(0);

        clock_t t1=clock();
        infile >> n;
        //cout<<"Vertice num="<<n<<endl;
        while(infile>>from>>to){
            outAdjList[from][pointer_out[from]]=to;
            pointer_out[from]++;
            inAdjList[to][pointer_in[to]]=from;
            pointer_in[to]++;

            m++;
        }
        infile.close();
        clock_t t2=clock();
        //cout<<"Edge num="<<m<<endl;
        cout<<"The graph input has been completed in "<<(t2-t1)/(1.0*CLOCKS_PER_SEC)<<" s."<<endl;

        delete[] pointer_in;
        delete[] pointer_out;
    }

    uint gettxtInSize(uint vert){
        return indegree[vert];
    }
    uint gettxtInVert(uint vert, uint pos){
        return inAdjList[vert][pos];
    }
    uint gettxtOutSize(uint vert){
        return outdegree[vert];
    }
    uint gettxtOutVert(uint vert, uint pos){
        return outAdjList[vert][pos];
    }

    void csr_convert(string filedir, string filelabel){
        cout<<"Convert the raw graph dataset into the CSR form..."<<endl;
        uint *coutEL=new uint[m];
        uint *coutPL=new uint[n+1];
        uint *cinEL=new uint[m];
        uint *cinPL=new uint[n+1];
        coutPL[0]=0;
        uint outid=0;
        uint out_curnum=0;
  
        cinPL[0]=0;
        uint inid=0;
        uint in_curnum=0;
     
        for(uint i=0;i<n;i++){
            outid+=gettxtOutSize(i);
            coutPL[i+1]=outid;
            for(uint j=0;j<gettxtOutSize(i);j++){
                coutEL[out_curnum]=gettxtOutVert(i,j);
                out_curnum+=1;
            }
            inid+=gettxtInSize(i);
            cinPL[i+1]=inid;
            for(uint j=0;j<gettxtInSize(i);j++){
                cinEL[in_curnum]=gettxtInVert(i,j);
                in_curnum+=1;
            }
        }

        stringstream ss_cdir,ss_cattr,ss_coutEL,ss_coutPL,ss_cinEL,ss_cinPL;
        ss_cdir<<filedir;
        mkpath(ss_cdir.str());

        ss_cattr<<ss_cdir.str()<<filelabel<<".attribute";
        ofstream cout_attr;
        cout_attr.open(ss_cattr.str());
        if(!cout_attr){
            cout<<"ERROR: unable to open the output file: "<<ss_cattr.str()<<endl;
            return;
        }

        cout_attr<<"n: "<<n<<"\n";
        cout_attr<<"m: "<<m<<"\n";
        cout_attr.close();
    
        ss_coutEL<<ss_cdir.str()<<filelabel<<".outEdges";
        ss_coutPL<<ss_cdir.str()<<filelabel<<".outPtr";
    
        ss_cinEL<<ss_cdir.str()<<filelabel<<".inEdges";
        ss_cinPL<<ss_cdir.str()<<filelabel<<".inPtr";
        
        ofstream foutEL(ss_coutEL.str(),ios::out | ios::binary);
        ofstream foutPL(ss_coutPL.str(),ios::out | ios::binary);
        ofstream finEL(ss_cinEL.str(),ios::out | ios::binary);
        ofstream finPL(ss_cinPL.str(),ios::out | ios::binary);
    
        foutEL.write((char *)&coutEL[0],sizeof(outEL[0])*m);
        foutPL.write((char *)&coutPL[0],sizeof(outPL[0])*(n+1));
        finEL.write((char *)&cinEL[0],sizeof(inEL[0])*m);
        finPL.write((char *)&cinPL[0],sizeof(inPL[0])*(n+1));
    
        foutEL.close();
        foutPL.close();
        finEL.close();
        finPL.close();
    
        delete[] coutPL;
        delete[] coutEL;
        delete[] cinPL;
        delete[] cinEL;

        return;
    }
    
};


