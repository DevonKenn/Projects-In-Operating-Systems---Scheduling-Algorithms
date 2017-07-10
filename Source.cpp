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
#include <bitset>
#include <tuple>

using namespace std;

vector<int> BM;
vector<int> PM;
bool TLB = false;
vector<int> TLBV;
vector<int> LRU = { };
vector<int> MYF = {};

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
void newPM(){
	int frames = 1024;
	int words = 512;
	vector<int> NPM;
	for (int i = 0; i < frames*words; i++){
		if (i<words)
			NPM.push_back(0);
		else
			NPM.push_back(-1);
	}
	PM = NPM;
}
vector<int> newMask(){
	int frames = 32;
	vector<int> MASK;
	MASK.push_back(1);
	for (int i = 1; i <frames; i++)
		MASK.push_back(MASK[i-1]<<1);
	reverse(MASK.begin(), MASK.end());
	return MASK;
}
vector<int> newMask2(){
	int frames = 32;
	vector<int> MASK2;
	vector<int> MASK = newMask();
	for (int i = 0; i <frames; i++)
		MASK2.push_back(~MASK[i]);
	return MASK2;
}

tuple<int,int> BMSearch(int val){
	vector<int> MASK = newMask();
	for (int i = 0; i < BM.size(); i++){
		for (int j = 0; j < MASK.size(); j++){
			int test = BM[i] & MASK[j];
			if (test == 0){
				if (val = 2){
					int test2;
					if (j  == MASK.size()-1 && i < BM.size() - 1) test2 = BM[i + 1] & MASK[0];
					else if (j < MASK.size() - 1) test2 = BM[i] & MASK[j + 1];
					else continue;
					if (test == test2) return make_tuple(i, j);
				}return make_tuple(i,j);
			}
		}
	}
	return make_tuple(-1, -1);
}

vector<string> translateAddress(int VA){
	string S,P,W;
	string binary = bitset<32>(VA).to_string();
	for (int i = 4; i < 13; i++) S = S + binary[i];
	for (int i = 13; i < 23; i++) P = P + binary[i];
	for (int i = 23; i < 32; i++) W = W + binary[i];
	return vector<string>{S, P, W};
}
void miss(int SP, int S, int P){
	if (TLBV.size() < 4){
		TLBV.push_back(SP);
		LRU.push_back(3);
		MYF.push_back(PM[S] + P);
	}
	else{
		for (int i = 0; i < TLBV.size(); i++){
			if (LRU[i] == 0){
				LRU[i] = 3;
				TLBV[i] = SP;
				MYF[i] = PM[S] + P;
			}
		}
	}
	for (int i = 0; i < TLBV.size(); i++){
		if (TLBV[i] != SP){
			if (LRU[i] >0) LRU[i] --;
		}
	}
}
void readAddressTLB(int VA){
	string SP;
	string binary = bitset<32>(VA).to_string();
	vector<string> address = translateAddress(VA);
	int S = stoi(address[0], NULL, 2);
	int P = stoi(address[1], NULL, 2);
	int W = stoi(address[2], NULL, 2);
	for (int i = 4; i < 23; i++) SP = SP + binary[i];
	for (int i = 0; i <TLBV.size(); i++){
		if ((TLBV[i] == stoi(SP, NULL, 2))){
			for (int j = 0; j < TLBV.size(); j++){
				if (TLBV[j] != stoi(SP, NULL, 2) && LRU[j]>0) LRU[j] --;
				if (TLBV[j] == stoi(SP, NULL, 2)) LRU[j] = 3;
			}
			cout << "h " << PM[PM[S + P]] + W << " ";
			return;
		}
	}cout << "m ";
	if (PM[S] > 0){
		if (PM[PM[S] + P] > 0){
			cout << PM[PM[S] + P] + W << " ";//Physical address 
			miss(stoi(SP,NULL,2),S,P);
		}
		else if (PM[PM[S] + P] == -1) cout << "pf" << " ";
		else cout << "err" << " ";
	}
	else if (PM[S] == -1) cout << "pf" << " ";
	else cout << "err" << " ";
}
void readAddress(int VA){
	vector<string> address = translateAddress(VA);
	int S = stoi(address[0], NULL, 2);
	int P = stoi(address[1], NULL, 2);
	int W = stoi(address[2], NULL, 2);
	if (PM[S] > 0){
		if (PM[PM[S] + P] > 0) cout << PM[PM[S] + P] + W << " ";//Physical address 
		else if (PM[PM[S] + P] == -1) cout << "pf" << " ";
		else cout << "err" << " ";
	}
	else if (PM[S] == -1) cout << "pf" << " ";
	else cout << "err" << " ";
}
void writeBit(int index, int bitN, int bitVal){
	if (bitVal == 1) BM[index] = BM[index] | newMask()[bitN];
	else if (bitVal == 0) BM[index] = BM[index] | newMask2()[bitN];
}

void newBM(){
	int frames = 32;
	vector<int> NBM;
	for (int i = 0; i < frames; i++)
		NBM.push_back(0);
	BM = NBM;
	writeBit(0, 0, 1);
}
void writeAddressTLB(int VA){
	string SP;
	string binary = bitset<32>(VA).to_string();
	vector<string> address = translateAddress(VA);
	int S = stoi(address[0], NULL, 2);
	int P = stoi(address[1], NULL, 2);
	int W = stoi(address[2], NULL, 2);
	for (int i = 4; i < 23; i++) SP = SP + binary[i];
	for (int i = 0; i <TLBV.size(); i++){
		if ((TLBV[i] == stoi(SP, NULL, 2))){
			for (int j = 0; j < TLBV.size(); j++){
				if (TLBV[j] != stoi(SP, NULL, 2) && LRU[j]>0) LRU[j] --;
				if (TLBV[j] == stoi(SP, NULL, 2)) LRU[j] = 3;
			}
			cout << "h " << PM[PM[S+P]]+W<<" " ;
			return;
		}
	}cout << "m ";
	if (PM[S] == -1 ){
		cout << "pf" << " ";
		return;
	}
	if (PM[S] > 0){//Write to existing table
		if (PM[PM[S] + P] > 0){
			cout << PM[PM[S] + P] + W << " ";//Physical address 
			miss(stoi(SP,NULL,2),S,P);
		}
		else if (PM[PM[S] + P] == -1) cout << "pf" << " ";
		else{

			tuple<int, int> IJPage = BMSearch(1);
			if (get<0>(IJPage) != -1){
				miss(stoi(SP, NULL, 2), S, P);
				writeBit(get<0>(IJPage), get<1>(IJPage), 1);
				PM[PM[S] + P] = (get<0>(IJPage) * 32 + get<1>(IJPage)) * 512;
				for (int i = PM[PM[S] + P]; i < (PM[PM[S] + P] + 512); i++) PM[i] = 0;
				cout << PM[PM[S] + P] + W << " ";//Physical address 
			}
		}
	}
	else if (PM[S] == 0){
		tuple<int, int> IJ = BMSearch(2);
		if (get<0>(IJ) != -1){
			miss(stoi(SP, NULL, 2), S, P);
			writeBit(get<0>(IJ), get<1>(IJ), 1);
			if (get<1>(IJ) +1 == newMask().size()) writeBit(get<0>(IJ) +1, 0, 1);
			else writeBit(get<0>(IJ), get<1>(IJ)+1, 1);
			PM[S] = (get<0>(IJ) * 32 + get<1>(IJ)) * 512;
			for (int i = PM[S]; i < (PM[S] + 2 * 512); i++) PM[i] = 0;
			tuple<int, int> IJPage = BMSearch(1);
			if (get<0>(IJPage) != -1){
				writeBit(get<0>(IJPage), get<1>(IJPage), 1);
				PM[PM[S] + P] = (get<0>(IJPage) * 32 + get<1>(IJPage)) * 512;
				for (int i = PM[PM[S] + P]; i < (PM[PM[S] + P] + 512); i++) PM[i] = 0;
				cout << PM[PM[S] + P] + W << " ";//Physical address 
			}
		}
	}
}
void writeAddress(int VA){
	vector<string> address = translateAddress(VA);
	int S = stoi(address[0], NULL, 2);
	int P = stoi(address[1], NULL, 2);
	int W = stoi(address[2], NULL, 2);
	if (PM[S] == -1 ){
		cout << "pf" << " ";
		return;
	}
	if (PM[S] > 0){//Write to existing table
		if (PM[PM[S] + P] > 0) cout << PM[PM[S] + P] + W << " ";//Physical address 
		else if (PM[PM[S] + P] == -1) cout << "pf" << " ";
		else {
			tuple<int, int> IJPage = BMSearch(1);
			if (get<0>(IJPage) != -1){
				writeBit(get<0>(IJPage), get<1>(IJPage), 1);
				PM[PM[S] + P] = (get<0>(IJPage) * 32 + get<1>(IJPage)) * 512;
				for (int i = PM[PM[S] + P]; i < (PM[PM[S] + P] + 512); i++) PM[i] = 0;
				cout << (get<0>(IJPage) * 32 + get<1>(IJPage)) * 512 <<" ";//Physical address 
			}
		}
	}
	else if (PM[S] == 0){
		tuple<int, int> IJ = BMSearch(2);
		if (get<0>(IJ) != -1){
			writeBit(get<0>(IJ), get<1>(IJ), 1);
			if (get<1>(IJ) +1 == newMask().size()) writeBit(get<0>(IJ) +1, 0, 1);
			else writeBit(get<0>(IJ), get<1>(IJ)+1, 1);
			PM[S] = (get<0>(IJ) * 32 + get<1>(IJ)) * 512;
			for (int i = PM[S]; i < (PM[S] + 2 * 512); i++) PM[i] = 0;
			tuple<int, int> IJPage = BMSearch(1);
			if (get<0>(IJPage) != -1){
				writeBit(get<0>(IJPage), get<1>(IJPage), 1);
				PM[PM[S] + P] = (get<0>(IJPage) * 32 + get<1>(IJPage)) * 512;
				for (int i = PM[PM[S] + P]; i < (PM[PM[S] + P] +  512); i++) PM[i] = 0;
				cout << (get<0>(IJPage) * 32 + get<1>(IJPage)) * 512 << " ";//Physical address 
			}
		}
	}
}

void parse(vector< vector<int> > resultV){
	newPM();
	newBM();
	//cout << resultV.size();
	for (int i = 0; i < resultV[0].size() - 1; i += 2){
		PM[resultV[0][i]] = resultV[0][i + 1];
		if (resultV[0][i + 1] != -1){
			for (int k = PM[resultV[0][i]] ; k < (PM[resultV[0][i]] + 2 * 512); k++) PM[k] = 0;
			writeBit(resultV[0][i + 1] /512/ 32, resultV[0][i + 1] / 512 % 32, 1);
			tuple<int, int> IJ = make_tuple(resultV[0][i + 1] / 512 / 32, resultV[0][i + 1] / 512 % 32);
			if (get<1>(IJ) +1 == newMask().size()) writeBit(get<0>(IJ) +1, 0, 1);
			else writeBit(get<0>(IJ), get<1>(IJ)+1, 1);

		}
	}
	for (int i = 0; i < resultV[1].size() - 2; i += 3){
		int P, S, F;
		P = resultV[1][i];
		S = resultV[1][i + 1];
		if (PM[S] == 0){
			PM[s] = resultV[0][i + 1];
			if (resultV[0][i + 1] != -1){
				for (int k = PM[resultV[0][i]]; k < (PM[resultV[0][i]] + 2 * 512); k++) PM[k] = 0;
				writeBit(resultV[0][i + 1] / 512 / 32, resultV[0][i + 1] / 512 % 32, 1);
				tuple<int, int> IJ = make_tuple(resultV[0][i + 1] / 512 / 32, resultV[0][i + 1] / 512 % 32);
				if (get<1>(IJ) +1 == newMask().size()) writeBit(get<0>(IJ) +1, 0, 1);
				else writeBit(get<0>(IJ), get<1>(IJ)+1, 1);

			}

		}
		F=	resultV[1][i + 2];
		PM[PM[S] + P] = F;
		if (F != -1){
			for (int k = PM[PM[S] + P]; k < (PM[PM[S] + P] + 2 * 512); k++) PM[k] = 0;
			writeBit(F / 512 / 32, F / 512 % 32, 1);
			for (int k = PM[PM[S] + P]; k < (PM[PM[S] + P] + 2 * 512); k++) PM[k] = 0;
		}
	}
	//for (int i = 0; i < 100;i++)
	//	cout << PM[i];
	for (int i = 0; i < resultV[2].size() - 1; i += 2){
		//cout << "\n " << resultV[2][i] << " " << resultV[2][i + 1];
		if (TLB){
			if (resultV[2][i] == 0) readAddressTLB(resultV[2][i + 1]);
			else writeAddressTLB(resultV[2][i + 1]);
		}
		else {
			if (resultV[2][i] == 0) readAddress(resultV[2][i + 1]);
			else writeAddress(resultV[2][i + 1]);
		}
		
	}

}

int main(){
	cout << "ENTER FILE PATH\n$ ";
	string newDir;// = R"(C:\path\to\directory\)"
	string fileName = "wide1.txt";
	string fileName2 = "wide2.txt";
	string outFileName = "785525591.txt";
	string outFileName2 = "785525592.txt";
	getline(cin, newDir);
	char buf[4096]; // never know how much is needed
	string s = newDir  + fileName;
	string s2 = newDir + outFileName;
	string s3 = newDir + fileName2;
	string s4 = newDir + outFileName2;

	ofstream ofin(s2);
	ofstream ofin2(s4);

	streambuf *coutbuf = cout.rdbuf(); //save old buf
	cout.rdbuf(ofin.rdbuf()); //redirect std::cout to out.txt!

	ifstream input(s);
	ifstream input2(s3);

	if (!input)
		cout << "\n input file 1 not opened successfully \"" << s << "\"";
	if (!input2)
		cout << "\n input file 2 not opened successfully \"" << s << "\"";
	if (!ofin)
	cout << "\n output file not opened successfully";
	if (!input){
		cerr << "File could not be opened: " << s << endl;
		return 0;
	}
	string line;
	vector<vector<int>> inputV;
	while (getline(input, line))
		inputV.push_back(split(line));
	while (getline(input2, line))
		inputV.push_back(split(line));
	//cout << inputV.size();
	parse(inputV);
	TLB = true;
	cout << "\n";
	std::cout.rdbuf(coutbuf); //reset to standard output again
	//streambuf *coutbuf = cout.rdbuf(); //save old buf
	cout.rdbuf(ofin2.rdbuf()); //redirect std::cout to out.txt!

	parse(inputV);
	cout << "\n";

	std::cout.rdbuf(coutbuf); //reset to standard output again
	cout << "Outut to " << s2 << " \n";

}