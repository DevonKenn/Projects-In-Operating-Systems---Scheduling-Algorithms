
#include <string>
#include <vector>
#include <sstream>      // std::stringstream
#include <fstream>
#include <algorithm>    // std::remove_if
#include <iomanip>

#define _CRT_SECURE_NO_DEPRECATE

#include <direct.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <errno.h>  

#include <cstring>
#include <windows.h>



#include <iostream>

using namespace std;

const string FIFO = "FIFO";
const string SJF = "SJF";
const string SRT = "SRT";
const string MLF = "MLF";

typedef struct process{
	int arrivalTime;
	int totalTime;
	int timeLeft;
	int time;

	int index;

} process;

typedef struct myTime{
	int time;
	int placing;
} myTime;


bool invalidCharacter(char c){
	return !(c >= 0 && c < 128);
}

vector<int> split(string input){
	stringstream ss;
	ss.str(input);
	string item;
	vector<int> result;
	while (getline(ss, item, ' ')) {
		item.erase(remove_if(item.begin(), item.end(), invalidCharacter), item.end());
		result.push_back(stoi(item));
	}
	return result;
}
vector<process> interpretProcess(vector<int> inVector){
	vector<process> resultV;
	
	for (int i = 0; i < inVector.size() - 1; i+=2){
		process p{ inVector[i], inVector[i + 1], inVector[i + 1], 0,0 };
		resultV.push_back(p);
	}
	return resultV;
}

void doFIFO(int& curTime, int processCount, vector<process> processes, vector<process> &stableProcesses){

	while (!processes.empty()){
		if (processes[0].arrivalTime <= curTime){
			process cur = processes[0];
			curTime += cur.totalTime;
			int waitingTime = curTime - cur.arrivalTime;
			//int t = waitingTime + processes[0].totalTime;
			//times.push_back(t);
			stableProcesses[cur.index].time = waitingTime;
			processes.erase(processes.begin());
		}
		else curTime++;
	}
}

void doSJF(int& curTime, int processCount, vector<process> processes,vector<process> &stableProcesses){
	process * cur = nullptr;// &stableProcesses[0];
	int curIndex = 0;

	while (!processes.empty()){		

		//process cur = processes[0];
		//int cnt = 1;
		/*while (cnt<processes.size()){
			if (processes[cnt].arrivalTime == cur.arrivalTime){
				if (processes[cnt].totalTime < cur.totalTime){
					cur = processes[cnt];
					curIndex = cnt; 
				}
				cnt++;
			}
			else break;
		}*/
		if (cur!=nullptr && cur->timeLeft == 0){
			int waitingTime = curTime - cur->arrivalTime;
			//int t = waitingTime + cur.totalTime;
			stableProcesses[cur->index].time = waitingTime;
			processes.erase(processes.begin() + curIndex);
			cur = nullptr;
		}
		if (cur == nullptr){
			for (int i = 0; i < processes.size(); i++){
				if (processes[i].arrivalTime > curTime)
					break;
				if (cur == nullptr || processes[i].totalTime < cur->totalTime){
					cur = &stableProcesses[processes[i].index];
					curIndex = i;
				}
			}
		}
		curTime++;//= cur.totalTime;

		if (cur != nullptr){
			cur->timeLeft--;
		}
		//times.push_back(myTime{t,find(originalProcesses,cur));

	}
}
void doSRT(int& curTime, int processCount, vector<process> processes, vector<process> &stableProcesses){
	int curIndex = 0;
	process * cur = &processes[0];
	while (!processes.empty()){
		for (int i = 0; i < processes.size(); i++){
			if (processes[i].arrivalTime >curTime)
				break;
			if (processes[i].timeLeft < cur->timeLeft&&processes[i].arrivalTime == curTime){
				cur = &processes[i];
				curIndex = i;
			}
		}		
		if (cur->timeLeft == 0){
			int waitingTime = curTime - cur->arrivalTime;
			stableProcesses[cur->index].time = waitingTime;
			processes.erase(processes.begin() + curIndex);
			if (processes.size() > 0){
				cur = &processes[0];
				curIndex = 0;
				for (int i = 0; i < processes.size(); i++){
					if (processes[i].arrivalTime >curTime)
						break;
					if (processes[i].timeLeft < cur->timeLeft){
						cur = &processes[i];
						curIndex = i ;
					}
				}
			}
		}
		else {
			cur->timeLeft = cur->timeLeft - 1;
			curTime++;
		}
	}
}
int layerLimit(int layer){
	if (layer == 0)
		return 1;
	if (layer == 1)
		return 2;
	if (layer == 2)
		return 4;
	if (layer == 3)
		return 16;
	return -1;
}
void doMLF(int& curTime, int processCount, vector<process> processes, vector<process> &stableProcesses){
	vector<process*> layer5;
	vector<process*> layer4;
	vector<process*> layer3;
	vector<process*> layer2;
	vector<process*> layer1;

	vector< vector<process*>> layers;
	layers.push_back(layer1);
	layers.push_back(layer2);
	layers.push_back(layer3);
	layers.push_back(layer4);
	layers.push_back(layer5);
	int curIndex = -1;

	process * cur =  nullptr;// &stableProcesses[0];
	//layers[0].push_back(cur);
	//cout << layers[0].empty();
	int recentlyErased = 0;
	while (!processes.empty() ||!layers[0].empty() || !layers[1].empty() || !layers[2].empty() || !layers[3].empty() || !layers[4].empty()){
		//gfcout << "IN";
		//if ((curIndex == -1) && (!processes.empty() || !layers[0].empty() || !layers[1].empty() || !layers[2].empty() || !layers[3].empty() || !layers[4].empty()))
		//	curIndex = 0;
		if (cur!=nullptr&&cur->timeLeft == 0){
			//cout << "CUR TIME " << curTime<<"\n";
			int waitingTime = curTime - cur->arrivalTime;
			stableProcesses[cur->index].time = waitingTime;
			layers[curIndex].erase(layers[curIndex].begin());
			cur = nullptr;
			//curIndex = -1;
			recentlyErased = 1;
		}
		else {
			if (cur!=nullptr&& curIndex <4 && cur->time >= layerLimit(curIndex)){//Finished with this layer
				layers[curIndex].erase(layers[curIndex].begin());
				layers[curIndex + 1].push_back(cur);
				cur->time = 0;
				cur = nullptr;
				//curIndex = -1;
			}
			if (processes.size() > 0 && curTime >= processes[0].arrivalTime){//new process at this time
				cur = &stableProcesses[processes[0].index];
				processes.erase(processes.begin());
				curIndex = 0;
				layers[curIndex].push_back(cur);
			}
			else{
				for (int i = 0; i < 5; i++){//get lowest process
					if (!layers[i].empty()){
						if (i == curIndex){
							if (cur!=nullptr&&recentlyErased == 0&&curIndex==4){
								layers[curIndex].erase(layers[curIndex].begin());
								layers[curIndex].push_back(cur);
							}
						}
						cur = layers[i][0];
						curIndex = i;
						break;
					}
				}
			}
			if (cur != nullptr){
				cur->timeLeft = cur->timeLeft - 1;
				cur->time++;
			}
			curTime++;
			//recentlyErased = 0;
		}
	}
}

void scheduler(vector<process> processes, string kind){
	int curTime = processes[0].arrivalTime;
	int processCount = processes.size();
	//vector<int> times;

	if (kind.compare(FIFO) == 0)
		doFIFO(curTime, processCount, processes, processes);
	else if (kind.compare(SJF) == 0)
		doSJF(curTime, processCount, processes, processes);
	else if (kind.compare(SRT) == 0)
		doSRT(curTime, processCount, processes, processes);
	else if (kind.compare(MLF) == 0)
		doMLF(curTime, processCount, processes, processes);

	float sum = 0;
	for (int i = 0; i < processes.size(); i++)
		sum += processes[i].time;
	float average = sum / processCount;
	cout << fixed;
	cout << setprecision(2);
	cout << average << " ";
	for (int i = 0; i < processes.size(); i++)
		cout << processes[i].time << " ";
	cout << "\n";
}

bool compareByArrival(const process &a, const process &b){
	return a.arrivalTime < b.arrivalTime;
}
void parse(vector<int> resultV){
	if (resultV.size() < 2) return;
	vector<process> processes = interpretProcess(resultV);
	stable_sort(processes.begin(), processes.end(), compareByArrival);
	for (int i = 0; i < processes.size(); i++){
		processes[i].index = i;
		//cout << processes[i].arrivalTime << " " << processes[i].totalTime <<  " " << processes[i].index << "\n";
	}
	scheduler(processes, FIFO);
	scheduler(processes, SJF);
	scheduler(processes, SRT);
	scheduler(processes, MLF);
}


int main(){
	string fileName = "input.txt";
	string outFileName = "78552559.txt";
	cout << "ENTER FILE PATH\n$ ";
	string newDir;// = R"(C:\path\to\directory\)"
	getline(cin, newDir);
	char buf[4096]; // never know how much is needed
	string s = newDir + fileName;
	string s2 = newDir + outFileName;

	ofstream ofin(s2);
	streambuf *coutbuf = cout.rdbuf(); //save old buf
	cout.rdbuf(ofin.rdbuf()); //redirect std::cout to out.txt!

	ifstream input(s);
	if (!input)
		cout << "\n input file not opened successfully \""<<s<<"\"";
	if (!ofin)
		cout << "\n output file not opened successfully";

	if (!input){
		cerr << "File could not be opened: " << s << endl;
		return 0;
	}

	string line;

	while (getline(input, line)){
		vector<int> resultV = split(line);
		parse(resultV);
	}
	cout << "\n";
	
	std::cout.rdbuf(coutbuf); //reset to standard output again
	cout << "Outut to " << s2 << " \n";


	return 0;
}