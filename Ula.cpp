#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <cstdint>

using namespace std;

vector<bitset<32>> FuncAB(const vector<int>& instrucao, const bitset<32>& A, const bitset<32>& B) {
    bool ENA  = instrucao[4];
    bool ENB  = instrucao[5];
    bool INVA = instrucao[6];

    bitset<32> operandoA = ENA ? A : bitset<32>(0); // Se ENA = 1, OpA = A; se ENA = 0, OpA = 0
    if (INVA) operandoA = ~operandoA;
    bitset<32> operandoB = ENB ? B : bitset<32>(0); // Se ENB = 1, OpB = B; se ENB = 0, OpB = 0

    // S = ((A & ENA) ^ INVA) & (B & ENB)
    bitset<32> S = (operandoA & operandoB);
    bitset<32> CO; // Por padrão, todos os bits serão 0

    return {S,CO};
}

vector<bitset<32>> FuncAouB(const vector<int> &instrucao, const bitset<32> &A, const bitset<32> &B) {
    bool ENA  = instrucao[4];
    bool ENB  = instrucao[5];
    bool INVA = instrucao[6];

    bitset<32> operandoA = ENA ? A : bitset<32>(0);
    if (INVA) operandoA = ~operandoA;
    bitset<32> operandoB = ENB ? B : bitset<32>(0);

    // S = (A & ENA) ^ INVA | (B & ENB)
    bitset<32> S = operandoA | operandoB;
    bitset<32> CO;

    return {S,CO};
}

vector<bitset<32>> FuncInvB(const vector<int> &instrucao, const bitset<32> &A, const bitset<32> &B) {
    bool ENB  = instrucao[5];

    bitset<32> operandoB = ENB ? B : bitset<32>(0);
    bitset<32> S = ~operandoB;
    bitset<32> CO(0);

    return {S,CO};
}

vector<bitset<32>> FuncSomador(const vector<int> &instrucao, const bitset<32> &A, const bitset<32> &B) {
    bool ENA = instrucao[4];
    bool ENB = instrucao[5];
    bool INVA = instrucao[6];
    bool carry = instrucao[7];

    bitset<32> operandoA = ENA ? A : bitset<32>(0);// Se ENA = 1, OpA = A; se ENA = 0, OpA = 0
    if (INVA) operandoA = ~operandoA;
    bitset<32> operandoB = ENB ? B : bitset<32>(0);

    bitset<32> soma;
    for (int i = 0; i < 32; ++i) {

        bool a_bit = operandoA[i];
        bool b_bit = operandoB[i];

        bool s_bit = a_bit ^ b_bit ^ carry; // Se 1 ou 3 forem 1, s_bit = 1
                                            // se 0 ou 2 forem 1, s_bit = 0
        soma[i] = s_bit;
        carry = (a_bit & b_bit) | ((a_bit ^ b_bit) & carry); // Se A = B = 1, carry = 1
    }                                                        // Se ( A = 1 | B = 1 ) & Carry, carry = 1

    bitset<32> carry_out;
    carry_out[0] = carry;

    return {soma, carry_out};
}

vector<bitset<32>> FuncDesloc(const vector<int> &instrucao, const bitset<32> &S){
    bitset<32> SD = S;
    if (instrucao[0]) {
        SD <<= 8; // Desloca os 8 bits pra esquerda
        // SLL8
    }else if (instrucao[1]) {
        SD = bitset<32>((int32_t(SD.to_ulong()) >> 1)); // desloca 1 pra direita se o bit 1 for 1
        //SRA1
    }else {
        SD = S;
    }
    return {SD};
}


void Print(const vector<int> &instrucao, const vector<bitset<32>> &resultado, const bitset<32> &A, const bitset<32> &B, int PC)
    {
    unsigned int IR_value = 0;
    for (int bit : instrucao) {
    IR_value = (IR_value << 1) | bit; // Desloca bits
    }
    bitset<8> IR(IR_value);

    bitset<32> S = resultado[0];
    int CO = resultado[1][0]; // acesso ao bit 0 do bitset<32>
    bitset<32> SD = FuncDesloc(instrucao, S)[0];
    bool Z = (SD == 0);
    bool N = SD[0];

    // Impressão
    cout << "============================================================" << endl;
    cout << "Cycle " << PC << endl << endl;
    cout << "PC = " << PC << endl;
    cout << "IR = " << IR << endl;
    cout << "B  = " << B << endl;
    cout << "A  = " << A << endl;
    cout << "S  = " << S << endl;
    cout << "SD = " << SD << endl;
    cout << "CO = " << CO << endl;
    cout << "Z = " << Z << endl;
    cout << "N = " << N << endl;


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
    int IR;                             // Instrucao atual = 6 bits de entrada

    string linha;
    while (getline(inputFile, linha)) {
        vector<int> bits;
        for (char c : linha) {
            bits.push_back(c - '0'); // 0 tem valor 48 e 1 tem valor 49 na tabela ASCII
        }                            // Caso seja 0 => 48(int) - 48(int) = 0(int), caso seja 1 => 49(int) - 48(int) = 1(int)
        Palavras.push_back(bits);    // formato do vector => [{1,0,1,0,1,0},{0,1,0,1,1,1}]
    }

    vector<bitset<32>> resultado;
    bitset<32> A(string("00000000000000000000000000000001"));
    bitset<32> B(string("10000000000000000000000000000000"));

    cout << "Start of Program" << endl;

    for (int i = 0; i < Palavras.size(); i++) {
        PC = i + 1;
        int op0 = Palavras[i][2];
        int op1 = Palavras[i][3];

        if (Palavras[i][0] == 1 && Palavras[i][1] == 1) {
            unsigned int IR_value = 0;
            for (int bit : Palavras[i]) {
                IR_value = (IR_value << 1) | bit;
            }
            bitset<8> IR(IR_value);
            cout << "============================================================" << endl;
            cout << "Cycle " << PC << endl << endl;
            cout << "PC = " << PC << endl;
            cout << "IR = " << IR << endl;
            cout << "> Error, invalid control signals." << endl;
            continue;
        }

        /* o jeito que achei pra ajustar a ordem da bagaça, porque o professor coloca sll8 e sra1 como os primeiros
         ent tive que trocar isso daqui*/

        if (op1 == 0 && op0 == 0) {
            resultado = FuncAB(Palavras[i], A, B);
        } else if (op1 == 0 && op0 == 1) {
            resultado = FuncAouB(Palavras[i], A, B);
        } else if (op1 == 1 && op0 == 0) {
            resultado = FuncInvB(Palavras[i], A, B);
        } else if (op1 == 1 && op0 == 1) {
            resultado = FuncSomador(Palavras[i], A, B);
        }


        Print(Palavras[i], resultado, A, B, PC);

    }


    cout << "============================================================" << endl;
    cout << "Cycle " << PC + 1 << endl;
    cout << "> Line is empty, EOP." << endl;

    return 0;
}
