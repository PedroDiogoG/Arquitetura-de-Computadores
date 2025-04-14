#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <cstdint>

using namespace std;

bitset<32> MAR("00000000000000000000000000000000");
bitset<32> MDR("00000000000000000000000000000000");
bitset<32> PC ("00000000000000000000000000000000");
bitset<32> SP ("00000000000000000000000000000000");
bitset<32> LV ("00000000000000000000000000000000");
bitset<32> CPP("00000000000000000000000000000000");
bitset<32> TOS("00000000000000000000000000000010");
bitset<32> OPC("00000000000000000000000000000000");
bitset<32> H  ("00000000000000000000000000000001");

bitset<8> MBR ("10000001");

string c_bus;
string b_bus;

bitset<32> Decod(const vector<int>& instrucao) { //Faz a decodificacao
    string bitsDecod;

    for (int j = instrucao.size() - 4; j < instrucao.size(); j++) {
        bitsDecod.push_back(instrucao[j] ? '1' : '0');
    }

    bitset<4> Bitset_Decod(bitsDecod);

    bitset<32> saida(0);
    int indice = static_cast<int>(Bitset_Decod.to_ulong());
    cout << "INDICEEEE " <<  indice << endl;
    switch (indice) {
        case 0:
            saida = MDR;
            b_bus = "MDR";
            break;
        case 1:
            saida = PC;
            b_bus = "PC";
            break;
        case 2: {
            // Se o bit de sinal (bit 7) for 1, os bits superiores devem ser preenchidos com 1.
            unsigned long valor = MBR.to_ulong();
            if (MBR.test(7)) { // verifica se o bit de sinal é 1 ou 0
                valor |= 0xFFFFFF00; // preenche os 24 bits superiores com 1
            }
            saida = bitset<32>(valor);
            b_bus = "MBR";
            break;
        }
        case 3:
            // por default os bits vazios sao 0
            saida = bitset<32>(MBR.to_ulong());
            b_bus = "MBRU";
            break;
        case 4:
            saida = SP;
            b_bus = "SP";
            break;
        case 5:
            saida = LV;
            b_bus = "LV";
            break;
        case 6:
            saida = CPP;
            b_bus = "CPP";
            break;
        case 7:
            saida = TOS;
            b_bus = "TOS";
            break;
        case 8:
            saida = OPC;
            b_bus = "OPC";
            break;
        default:
            break;
        } 

    return saida;
}

void getC_bus(const bitset<9>& seletor){
    for (int i = 0; i < 9; ++i) {
        if(seletor.test(i)){
            switch (i)
            {
            case 0:
                c_bus += "MAR ";
                break;
            case 1:
                c_bus += "MDR ";
                break;
            case 2:
                c_bus += "PC ";
                break;
            case 3:
                c_bus += "SP ";
                break;
            case 4:
                c_bus += "LV ";
                break;
            case 5:
                c_bus += "CPP ";
                break;
            case 6:
                c_bus += "TOS ";
                break;
            case 7:
                c_bus += "OPC ";
                break;
            case 8:
                c_bus += "H ";
                break;
            default:
                break;
            }
        }
    }
}

void Seletor(const bitset<9>& seletor, const bitset<32>& valor) {
    for (int i = 0; i < 9; ++i) {
        if(seletor.test(i)){
            switch (i)
            {
            case 0:
                MAR = valor;
                c_bus += "MAR ";
                break;
            case 1:
                MDR = valor;
                c_bus += "MDR ";
                break;
            case 2:
                PC = valor;
                c_bus += "PC ";
                break;
            case 3:
                SP = valor;
                c_bus += "SP ";
                break;
            case 4:
                LV = valor;
                c_bus += "LV ";
                break;
            case 5:
                CPP = valor;
                c_bus += "CPP ";
                break;
            case 6:
                TOS = valor;
                c_bus += "TOS ";
                break;
            case 7:
                OPC = valor;
                c_bus += "OPC ";
                break;
            case 8:
                H = valor;
                c_bus += "H ";
                break;
            default:
                break;
            }
        }
    }
}

bitset<9> getBitSelect(const vector<int>& instrucao){ // Pega os 9 bits de select
    string bitsSelect;
    for (int k = 8; k < 8 + 9; k++) {
        bitsSelect.push_back(instrucao[k] ? '1' : '0');
    }
    bitset<9> Bitset_Select(bitsSelect);
    return Bitset_Select;
}

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

void printRegisters(){
    cout << "MAR = " << MAR << endl;
    cout << "MDR = " << MDR << endl;
    cout << "PC = "  << PC  << endl;
    cout << "MBR = " << MBR << endl;
    cout << "SP =  " << SP  << endl;
    cout << "LV =  " << LV  << endl;
    cout << "CPP = " << CPP << endl;
    cout << "TOS = " << TOS << endl;
    cout << "OPC = " << OPC << endl;
    cout << "H   = " << H   << endl;
}

void Print(const vector<int> &instrucao, const vector<bitset<32>> &resultado, const bitset<32> &A, const bitset<32> &B, int PC)
    {
    unsigned int IR_value = 0;
    for (int bit : instrucao) {
    IR_value = (IR_value << 1) | bit; // Desloca bits
    }
    bitset<21> IR(IR_value);
    getC_bus(getBitSelect(instrucao));
    cout << "============================================================" << endl;
    cout << "Cycle " << PC << endl;
    cout << "IR " << IR << endl;
    cout << "b_bus = " << b_bus << endl;
    cout << "c_bus = " << c_bus << endl;
    
    cout << endl << "> Registers before instruction" << endl;
    printRegisters();

    bitset<32> S = resultado[0];
    int CO = resultado[1][0]; // acesso ao bit 0 do bitset<32>
    bitset<32> SD = FuncDesloc(instrucao, S)[0];
    bool Z = (SD == 0);
    bool N = SD[0];
    Seletor(getBitSelect(instrucao),SD);
    
    cout << endl << "> Registers after instruction" << endl;
    printRegisters();

    b_bus.clear();
    c_bus.clear();
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
    int IR;                             // Instrucao atual 

    string linha;
    while (getline(inputFile, linha)) {
        vector<int> bits;
        for (char c : linha) {
            bits.push_back(c - '0'); // 0 tem valor 48 e 1 tem valor 49 na tabela ASCII
        }                            // Caso seja 0 => 48(int) - 48(int) = 0(int), caso seja 1 => 49(int) - 48(int) = 1(int)
        Palavras.push_back(bits);    // formato do vector => [{1,0,1,0,1,0},{0,1,0,1,1,1}]
    }

    vector<bitset<32>> resultado;
    cout << "Start of Program" << endl;

    for (int i = 0; i < Palavras.size(); i++) {
        PC = i + 1;
        int op0 = Palavras[i][2];
        int op1 = Palavras[i][3];
        
        bitset<32> A = H;
        bitset<32> B = Decod(Palavras[i]);

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