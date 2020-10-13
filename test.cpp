#include <iostream>
#include <vector>
#include <string>

using namespace std;

vector<string> split(string str, char delimiter);

int main(){
    string test = "min sung kang";
    vector<string> result = split(test, ' ');
    for (int i=0;i<result.size();i++){
        cout << result[i] << " ";
    }
}

vector<string> split(string input, char delimiter) {
    vector<string> answer;
    stringstream ss(input);
    string temp;
 
    while (getline(ss, temp, delimiter)) {
        answer.push_back(temp);
    }
 
    return answer;
}