#include <iostream>
#include <ctime>
#include "md5.h"

using namespace std;

int scrollSymbol(int index, string& word, const string& drum) {
	if (index > word.length() - 1 || index < 0) {
		return -1;
	}
	int indexNextChar = drum.find(word[index]) + 1;
	if (indexNextChar < drum.length()) {
		word[index] = drum[indexNextChar];
		return 0;
	}
	else {
		word[index] = drum[0];
		return scrollSymbol(index - 1, word, drum);
	}
}

int main()
{
	int passwordLen = 8;
	bool shorterAlso = 1;
	string charsToUse = "qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM0123456789";
	string targetHash = "274672838a8002344fed81ca1228bf05";
	int showEachNPassword = 10000;
	
    MD5 md5 ;
    char* hashWord;
	int charsCount = charsToUse.length();	
	int i = 0;
	if (!shorterAlso) {
		i = passwordLen - 1;
	}
	
	bool founded = 0;
	time_t end, start = clock();	
	for(; i < passwordLen; i++){
		string str = "";
		int count = 0;
        for(int j = 0; j <= i; j++){
            str += charsToUse[0];
        }
		
		int n = 0;
		while (!scrollSymbol(str.length() - 1, str, charsToUse)){
			string hash = string(md5.digestString(&str[0]));
			n++;
			if (showEachNPassword){
				if(n % showEachNPassword == 0) {
					cout << str << "\r";
				}
			}
			if (hash == targetHash) {
				end = clock();
				cout << "Founded password to hash: " << hash << " \n the password is: " << str << endl;
				founded = 1;
				break;
			}
		}
		if (founded)
			break;
	}
	cout << "Search";
	if (founded) {
		cout << " successful";
	}
	else {
		end = clock();
		cout << " unsuccessful";
	}
	time_t result = end - start;
	int h = result /(60 * 60 * CLOCKS_PER_SEC);
	int min = result /(60 * CLOCKS_PER_SEC);
	int s = (result / CLOCKS_PER_SEC) % 60;
	int ms = result % CLOCKS_PER_SEC;
	cout << " spent: " << h << " h " << min << " min " << s << "." << ms << " s" << endl;
	
    return 0;
}