#include <iostream>

#include "headers.h"
#include "M.h"
#include "nodeInformation.h"
#include "functions.h"
#include "helperClass.h"

using namespace std;

//the  Node information 
NodeInformation::NodeInformation(){
	fingerTable = vector< pair< pair<string,int> , lli > >(M+1);//fingers //lli the hash code
	successorList = vector< pair< pair<string,int> , lli > >(R+1);//successors //string : successor.first.first 
	// int sucessor.first.second // lli the hash code (node id on the ring)
	isInRing = false; //the Node state that it is not in the ring 
}

void NodeInformation::setStatus(){
	isInRing = true; // node is in the ring
}

//set the successor for the node
void NodeInformation::setSuccessor(string ip,int port,lli hash){
	successor.first.first = ip; //successor ip
	successor.first.second = port; //successor port
	successor.second = hash; //hash code (node id on the ring)
}

void NodeInformation::setSuccessorList(string ip,int port,lli hash){
	//R=10 from M.h file
	for(int i=1;i<=R;i++){
		successorList[i] = make_pair(make_pair(ip,port),hash);
	}
}

void NodeInformation::setPredecessor(string ip,int port,lli hash){
	predecessor.first.first = ip; //predecessor ip
	predecessor.first.second = port; //predecessor port
	predecessor.second = hash;	//the hash code (node id on the ring)
}

void NodeInformation::setId(lli nodeId){
	id = nodeId; //(node id on the ring)
}

void NodeInformation::setFingerTable(string ip,int port,lli hash){
	//M=48 from M.h file
	for(int i=1;i<=M;i++){
		fingerTable[i] = make_pair(make_pair(ip,port),hash);
	}
}

/*this function is for storing key in the key dictionary*/
void NodeInformation::storeKey(lli key,string val){
	dictionary[key] = val;
}

/*this function is for printing all the keys */
void NodeInformation::printKeys(){
	map<lli,string>::iterator it;// (it) is just like i in the normal for loop 

	for(it = dictionary.begin(); it != dictionary.end() ; it++){
		cout<<it->first<<" "<<it->second<<endl;
	}
}

/*this function is for updating the successor information */
void NodeInformation::updateSuccessorList(){

	HelperFunctions help;

	vector< pair<string,int> > list = help.getSuccessorListFromNode(successor.first.first,successor.first.second);
	/*successor ip , successor port*/ 

	if(list.size() != R)
		return;

	successorList[1] = successor;

	for(int i=2;i<=R;i++){
		successorList[i].first.first = list[i-2].first;//i=2 so the element will be put in the list[0] //successor ip 
		successorList[i].first.second = list[i-2].second;//i=3 so the element will be put in the list[1] //successor port
		successorList[i].second = help.getHash(list[i-2].first + ":" + to_string(list[i-2].second));// the hash code 
	}

}


/* send all keys of this node to it's successor after it leaves the ring */
vector< pair<lli , string> > NodeInformation::getAllKeysForSuccessor(){
	map<lli,string>::iterator it;
	vector< pair<lli , string> > res;

	for(it = dictionary.begin(); it != dictionary.end() ; it++){
		res.push_back(make_pair(it->first , it->second));
		dictionary.erase(it);
	}

	return res;
}

/*this function is for sending all the keys of this node to it's predecessor*/
vector< pair<lli , string> > NodeInformation::getKeysForPredecessor(lli nodeId){
	map<lli,string>::iterator it;

	vector< pair<lli , string> > res; 
	for(it = dictionary.begin(); it != dictionary.end() ; it++){
		lli keyId = it->first;

		/* if predecessor's id is more than current node's id */
		if(id < nodeId){
			if(keyId > id && keyId <= nodeId){
				res.push_back(make_pair(keyId , it->second));
				dictionary.erase(it);
			}
		}

		/* if predecessor's id is less than current node's id */
		else{
			if(keyId <= nodeId || keyId > id){
				res.push_back(make_pair(keyId , it->second));
				dictionary.erase(it);
			}
		}
	}

	return res;
}

/*this function for finding a successor for the node */
pair< pair<string,int> , lli > NodeInformation::findSuccessor(lli nodeId){

	pair < pair<string,int> , lli > self; //ip , port, id 
	self.first.first = sp.getIpAddress(); //the successor ip
	self.first.second = sp.getPortNumber();  //the successor port
	self.second = id; // the successor id

	if(nodeId > id && nodeId <= successor.second){
		return successor;
	}

	/* if there is no node else on the ring so the successor will be the node it self */
	else if(id == successor.second || nodeId == id){
		return self;
	}
    
	else if(successor.second == predecessor.second){
		if(successor.second >= id){
			if(nodeId > successor.second || nodeId < id)
				return self;
		}
		else
		 /*if there is another node on the ring so it will be the successor else the node itself will be the successor 
		so if the node id was grater than the given id and the succID or less of them this condition will return the successor 
		else it will return the node itself as the successor*/
		{
			if((nodeId > id && nodeId > successor.second) || (nodeId < id && nodeId < successor.second))
				return successor;
			else
				return self;
		}
	}

	else{

		pair < pair<string,int> , lli > node = closestPrecedingNode(nodeId);// find the closest preceding node 
		if(node.second == id) //if node id = id return the successor
		{
			return successor;
		}
		else{

			/*if not  connect to node which will now find the successor */
			struct sockaddr_in serverToConnectTo; 
			socklen_t len = sizeof(serverToConnectTo);

			string ip;
			int port;

			/* if this node couldn't find closest preciding node for given node id then now ask it's successor to do so */
			if(node.second == -1) //if the node id = -1 so the node will be the successor 
			{
				node = successor;
			}

			HelperFunctions help;

			help.setServerDetails(serverToConnectTo,node.first.first,node.first.second);

			/* set timer on this socket */
    		struct timeval timer;
    		help.setTimer(timer);


			int sockT = socket(AF_INET,SOCK_DGRAM,0);

			setsockopt(sockT,SOL_SOCKET,SO_RCVTIMEO,(char*)&timer,sizeof(struct timeval));

			if(sockT < 0){
				cout<<"socket cre error";
				perror("error");
				exit(-1);
			}

			/* send the node's id to the other node */
			char nodeIdChar[40];
			strcpy(nodeIdChar,to_string(nodeId).c_str());
			sendto(sockT, nodeIdChar, strlen(nodeIdChar), 0, (struct sockaddr*) &serverToConnectTo, len);

			/* receive ip and port of node's successor as ip:port*/
			char ipAndPort[40];

			int l = recvfrom(sockT, ipAndPort, 1024, 0, (struct sockaddr *) &serverToConnectTo, &len);

			close(sockT);

			if(l < 0){
				pair < pair<string,int> , lli > node;
				node.first.first = "";
				node.second = -1;
				node.first.second = -1;
				return node;
			}

			ipAndPort[l] = '\0';

			/* set ip,port and hash for this node and return it */
			string key = ipAndPort;
    		lli hash = help.getHash(ipAndPort);
    		pair<string,int> ipAndPortPair = help.getIpAndPort(key);
    		node.first.first = ipAndPortPair.first;
    		node.first.second = ipAndPortPair.second;
    		node.second = hash;

    		return node;
		}
	}
}


/*this function for knowing the the closest preceding node */
pair< pair<string,int> , lli > NodeInformation::closestPrecedingNode(lli nodeId){
	HelperFunctions help;

	for(int i=M;i>=1;i--) /*if ip empty and port and id =-1 contiinue */
					{
		if(fingerTable[i].first.first == "" || fingerTable[i].first.second == -1 || fingerTable[i].second == -1){
			continue;
		}
			//if the id of the finger[i] greater than the given id and les than the node id return the finger[i]
		if(fingerTable[i].second > id && fingerTable[i].second < nodeId){
			return fingerTable[i];
		}
		else{

			lli successorId = help.getSuccessorId(fingerTable[i].first.first,fingerTable[i].first.second);

			if(successorId == -1)
				continue;

			if(fingerTable[i].second > successorId){
				if((nodeId <= fingerTable[i].second && nodeId <= successorId) || (nodeId >= fingerTable[i].second && nodeId >= successorId)){
					return fingerTable[i];
				}
			}
			else if(fingerTable[i].second < successorId && nodeId > fingerTable[i].second && nodeId < successorId){
				return fingerTable[i];
			}
			
			pair< pair<string,int> , lli > predNode = help.getPredecessorNode(fingerTable[i].first.first,fingerTable[i].first.second,"",-1,false);
			lli predecessorId = predNode.second;

			if(predecessorId != -1 && fingerTable[i].second < predecessorId){
				if((nodeId <= fingerTable[i].second && nodeId <= predecessorId) || (nodeId >= fingerTable[i].second && nodeId >= predecessorId)){
					return predNode;
				}		
			}
			if(predecessorId != -1 && fingerTable[i].second > predecessorId && nodeId >= predecessorId && nodeId <= fingerTable[i].second){
				return predNode;
			}
		}
	}

	/* */
	pair< pair<string,int> , lli > node;
	node.first.first = "";
	node.first.second = -1;
	node.second = -1;
	return node;
}
//thid function is for checking if the nodes are alive 
void NodeInformation::stabilize(){

	/* get predecessor of successor */

	HelperFunctions help;

	string ownIp = sp.getIpAddress();
	int ownPort = sp.getPortNumber();

	if(help.isNodeAlive(successor.first.first,successor.first.second) == false)
		return;

	/* get predecessor of successor */
	pair< pair<string,int> , lli > predNode = help.getPredecessorNode(successor.first.first,successor.first.second,ownIp,ownPort,true);

	lli predecessorHash = predNode.second;

	if(predecessorHash == -1 || predecessor.second == -1)
		return;

	if(predecessorHash > id || (predecessorHash > id && predecessorHash < successor.second) || (predecessorHash < id && predecessorHash < successor.second)){
		successor = predNode;
	}

	
}

/* check if current node's predecessor is still alive */
void NodeInformation::checkPredecessor(){
	if(predecessor.second == -1)
		return;

	HelperFunctions help;
	string ip = predecessor.first.first; //the predecessor ip
	int port = predecessor.first.second; //the predecessor port

	if(help.isNodeAlive(ip,port) == false){
		/* if node has same successor and predecessor then set node as it's successor itself */
		if(predecessor.second == successor.second){
			successor.first.first = sp.getIpAddress();
			successor.first.second = sp.getPortNumber();
			successor.second = id;
			setSuccessorList(successor.first.first,successor.first.second,id);
		}
		predecessor.first.first = "";
		predecessor.first.second = -1;
		predecessor.second = -1;
	}

}

/* check if current node's successor is still alive */
void NodeInformation::checkSuccessor(){
	if(successor.second == id)
		return;

	HelperFunctions help;
	string ip = successor.first.first; //successor ip
	int port = successor.first.second; //successor port 

	if(help.isNodeAlive(ip,port) == false){
		successor = successorList[2];
		updateSuccessorList();
	}

}

void NodeInformation::notify(pair< pair<string,int> , lli > node){

	/* get id of node and predecessor */
	lli predecessorHash = predecessor.second;
	lli nodeHash = node.second;

	predecessor = node;

	/* if node's successor is node itself then set it's successor to this node */
	if(successor.second == id){
		successor = node;
	}
}

//this function is fixing the fingers for a node 
void NodeInformation::fixFingers(){

	HelperFunctions help;

	//if(help.isNodeAlive(successor.first.first,successor.first.second) == false)
		//return;
	//cout<<"in fix fingers - "<<successor.second<<endl;
	
	int next = 1;
	lli mod = pow(2,M);// 2 to the power of 48

	while(next <= M){
		if(help.isNodeAlive(successor.first.first,successor.first.second) == false)
			return;
		
		lli newId = id + pow(2,next-1);
		newId = newId % mod;
		pair< pair<string,int> , lli > node = findSuccessor(newId);
		if(node.first.first == "" || node.second == -1 || node.first.second == -1 )
			break;
		fingerTable[next] = node;
		next++;	
	}

}

//this function return the finger table 
vector< pair< pair<string,int> , lli > > NodeInformation::getFingerTable(){
	return fingerTable;
}

//this function is to return the id 
lli NodeInformation::getId(){
	return id;
}

//this function is to return the successor
pair< pair<string,int> , lli > NodeInformation::getSuccessor(){
	return successor;
}
//this function is to return the predecessor
pair< pair<string,int> , lli > NodeInformation::getPredecessor(){
	return predecessor;
}
////this function is to return the value of the key 
string NodeInformation::getValue(lli key){
	if(dictionary.find(key) != dictionary.end()){
		return dictionary[key];
	}
	else
		return "";
}
//this function is to return the successor list
vector< pair< pair<string,int> , lli > > NodeInformation::getSuccessorList(){
	return successorList;
}
//this function check if the node is in the ring 
bool NodeInformation::getStatus(){
	return isInRing;
}