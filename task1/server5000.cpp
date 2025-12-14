#include<iostream>
#include<sys/socket.h>
#include<cstring>
#include<arpa/inet.h>
#include<unistd.h>
#include<thread>
#include<mutex>
#include<sstream>
#include<unordered_map>
#include<netinet/in.h>
#include<vector>
using namespace std;

unordered_map<string, string> keyvalStore;
mutex keyvalMutex;

vector<pair<string, int>> servers={
    {"127.0.0.1", 5001},
    {"127.0.0.1", 5002}
};

void replicate(const string& cmd){
    for(auto& s: servers){
        int sock= socket(AF_INET, SOCK_STREAM, 0);
        if(sock<0) continue;

        sockaddr_in addr{};
        addr.sin_family= AF_INET;
        addr.sin_port= htons(s.second);
        inet_pton(AF_INET, s.first.c_str(), &addr.sin_addr);

        if(connect(sock, (sockaddr*)&addr, sizeof(addr))<0){
            close(sock);
            continue;
        }

        send(sock , cmd.c_str(), cmd.size(), 0);
        close(sock);
    }
}

string processCommand(const string& line){
    stringstream ss(line);
    vector<string> tokens;
    string tok;
    while(ss>>tok) tokens.push_back(tok);
    if(tokens.empty()) return "ERROR\n";

    if(tokens[0]=="REPL_SET" && tokens.size()==3){
        lock_guard<mutex> lock(keyvalMutex);
        keyvalStore[tokens[1]]=tokens[2];
        return "OK\n";
    }

    if(tokens[0]=="REPL_DELETE" && tokens.size()==2){
        lock_guard<mutex> lock(keyvalMutex);
        keyvalStore.erase(tokens[1]);
        return "OK\n";
    }

    if(tokens[0]=="SET" && tokens.size()==3){
        {
        lock_guard<mutex> lock(keyvalMutex);
        keyvalStore[tokens[1]]=tokens[2];
        }
        replicate("REPL_SET "+ tokens[1]+ " "+ tokens[2]+ "\n");
        return "OK\n";
    }

    if(tokens[0]=="GET" && tokens.size()==2){
        lock_guard<mutex> lock(keyvalMutex);
        if(keyvalStore.count(tokens[1])) return "VALUE "+keyvalStore[tokens[1]]+'\n';
        else return "NOT FOUND\n";
    }

    if(tokens[0]=="DELETE" && tokens.size()==2){
        {
        lock_guard<mutex> lock(keyvalMutex);
        keyvalStore.erase(tokens[1]);
        }
        replicate("REPL_DELETE "+ tokens[1]+ "\n");
        return "OK\n";
    }
    return "ERROR\n";
}

void handleClient(int clientSock){
    string buffer;
    while(true){
        char temp[1024];
        int n= recv(clientSock, temp, sizeof(temp), 0);
        if(n<=0){
            close(clientSock);
            return;
        }
        buffer.append(temp, n);

        size_t pos;
        while((pos= buffer.find('\n'))!= string::npos){
            string line= buffer.substr(0, pos);
            buffer.erase(0, pos+1);

            string response= processCommand(line);
            send(clientSock, response.c_str(), response.length(), 0);
        }
    }
}

int main(int argc, char* argv[]){
    int port = stoi(argv[1]);

    int serverSock= socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family= AF_INET;
    addr.sin_port= htons(port);
    addr.sin_addr.s_addr= INADDR_ANY;

    bind(serverSock, (sockaddr*)&addr, sizeof(addr));
    listen(serverSock, 10);

    cout<<"Server is running on port "<<port<<endl;

    while(true){
        int clientSock= accept(serverSock, nullptr, nullptr);
        thread t(handleClient , clientSock);
        t.detach();
    }
}