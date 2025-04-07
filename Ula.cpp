#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>

using namespace std;

vector<int> FuncAB(const vector<int> &instrucao,int A, int B){
    int ENA  = instrucao[2];
    int ENB  = instrucao[3];
    int INVA = instrucao[4];
    int INC  = instrucao[5]; 

    vector<int> resultado;
    int S = ((A & ENA) ^ INVA) & (B & ENB); // & = AND, ^ = XOR, | = OR
    int CO = 0;

    resultado.push_back(S);
    resultado.push_back(CO);

    return resultado;
}

vector<int> FuncAouB(const vector<int> &instrucao,int A, int B){
    int ENA  = instrucao[2];
    int ENB  = instrucao[3];
    int INVA = instrucao[4];
    int INC  = instrucao[5]; 

    vector<int> resultado;
    int S = ((A & ENA) ^ INVA) | (B & ENB); // & = AND, ^ = XOR, | = OR
    int CO = 0;

    resultado.push_back(S);
    resultado.push_back(CO);

    return resultado;
}

vector<int> FuncInvB(const vector<int> &instrucao,int A, int B){
    int ENA  = instrucao[2];
    int ENB  = instrucao[3];
    int INVA = instrucao[4];
    int INC  = instrucao[5]; 

    vector<int> resultado;
    int S =  !(B & ENB); // & = AND, ^ = XOR, | = OR, ! = INV
    int CO = 0;

    resultado.push_back(S);
    resultado.push_back(CO);

    return resultado;
}

vector<int> FuncSomador(const vector<int> &instrucao,int A, int B){
    int ENA  = instrucao[2];
    int ENB  = instrucao[3];
    int INVA = instrucao[4];
    int INC  = instrucao[5]; 

    vector<int> resultado;
    int S = (((A & ENA) ^ INVA) ^ (B & ENB)) ^ INC; // & = AND, ^ = XOR, | = OR
    int CO = (((A & ENA) ^ INVA) & (B & ENB)) | ((((A & ENA) ^ INVA) ^ (B & ENB)) & INC);

    resultado.push_back(S);
    resultado.push_back(CO);

    return resultado;
}

void Print(const vector<int> &instrucao,const vector<int> &resultado,int A, int B, int PC){
    unsigned int IR = 0;
    for (int bit : instrucao) {
        IR = (IR << 1) | bit;
    }

    cout << "============================================================" << endl;
    cout << "Cycle " << PC << endl << endl;
    cout << "PC = " << PC << endl;
    cout << "IR = " << bitset<6>(IR) << endl;
    cout << "A  = " << A << endl;
    cout << "B  = " << B << endl;
    cout << "S  = " << resultado[0] << endl;
    cout << "CO = " << resultado[1] << endl;
}

int main(int argc,char** argv){

    if(argc < 2){
        cout << "Digite algum arquivo!" << endl;
        exit(1);
    }

    ifstream inputFile(argv[1]);
    if(!inputFile){
        cout << "Erro ao abrir o arquivo!" << endl;
        exit(1);
    }
    
    vector<vector<int>> Palavras;       // i representa as operacoes ou linhas de codigo, j representa os 6 bits
    int PC = 0;                         // Program Count = linhas de codigo executadas
    int IR;                              // Instrucao atual = 6 bits de entrada
    int A = 0;      
    int B = 1;
    string linha;
    

    while (getline(inputFile, linha)) {
        vector<int> bits;  
        for (char c : linha) {
            bits.push_back(c - '0'); // 0 tem valor 48 e 1 tem valor 49 na tabela ASCII
        }                            // Caso seja 0 => 48(int) - 48(int) = 0(int), caso seja 1 => 49(int) - 48(int) = 1(int) 
        Palavras.push_back(bits);    // formato do vector => [{1,0,1,0,1,0},{0,1,0,1,1,1}]
    }
    
    cout << "Start of Program" << endl;

    vector<int> resultado;
    for(int i = 0 ; i < Palavras.size() ; i++){
        PC = i+1;
        if(Palavras[i][0] == 0 && Palavras[i][1] == 0){      // A e B
            resultado = FuncAB(Palavras[i],A,B);
            Print(Palavras[i],resultado,A,B,PC);
        }
        else if(Palavras[i][0] == 0 && Palavras[i][1] == 1){ // A ou B
            resultado = FuncAouB(Palavras[i],A,B);
            Print(Palavras[i],resultado,A,B,PC);
        }
        else if(Palavras[i][0] == 1 && Palavras[i][1] == 0){ // INV B
            resultado = FuncInvB(Palavras[i],A,B);
            Print(Palavras[i],resultado,A,B,PC);
        }
        else if(Palavras[i][0] == 1 && Palavras[i][1] == 1){ // Somador
            resultado = FuncSomador(Palavras[i],A,B);
            Print(Palavras[i],resultado,A,B,PC);
        }
    }
    cout << "============================================================" << endl;
    cout << "Cycle " << PC << endl;
    cout << "> Line is empty, EOP."<< endl;

    return 0;
}