#ifndef nodeInfo_h
#define nodeInfo_h

#include <iostream>
#include <vector>
#include <map>

#include "port.h"
#include "M.h"

using namespace std;

typedef long long int lli; //define a spicific datatype 

class NodeInformation{
	private:
		lli id; 
		pair< pair<string,int> , lli > predecessor;/*in pair<string,int> the first item is ip,second item is port ,
													lii is the predeid */
		pair< pair<string,int> , lli > successor;/*in pair<string,int> the first item is ip,second item is port ,
													lii is the succid */

		vector< pair< pair<string,int> , lli > > fingerTable; /*the finger table contain the ip ,id ,port of the node on the ring */ 
		map<lli,string> dictionary; // this dictionary is for setkeys 
		vector< pair< pair<string,int> , lli > > successorList; //this list contain the successor ip, id and port number

		bool isInRing; // for checking the existance of a node in the ring 

	public:
		SocketAndPort sp;
		
		NodeInformation();
		
		pair< pair<string,int> , lli > findSuccessor(lli nodeId);
		pair< pair<string,int> , lli > closestPrecedingNode(lli nodeId);
		void fixFingers();
		void stabilize();
		void notify(pair< pair<string,int> , lli > node);
		void checkPredecessor();
		void checkSuccessor();
		void updateSuccessorList();

		void printKeys();
		void storeKey(lli key,string val);
		vector< pair<lli , string> > getAllKeysForSuccessor();
		vector< pair<lli , string> > getKeysForPredecessor(lli nodeId);

		void setSuccessor(string ip,int port,lli hash);
		void setSuccessorList(string ip,int port,lli hash);
		void setPredecessor(string ip,int port,lli hash);
		void setFingerTable(string ip,int port,lli hash);
		void setId(lli id);
		void setStatus();
		
		lli getId();
		string getValue(lli key);
		vector< pair< pair<string,int> , lli > > getFingerTable();
		pair< pair<string,int> , lli > getSuccessor();
		pair< pair<string,int> , lli > getPredecessor(); 
		vector< pair< pair<string,int> , lli > > getSuccessorList();
		bool getStatus();
};

#endif