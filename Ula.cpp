#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <bitset>
#include <cstdint>
<<<<<<< HEAD
#include <vector>
#include <iomanip>
=======

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
>>>>>>> 58500a3f97d25456a1499b8cd606dbea29a0ab4a

bitset<32> Decod(const vector<int>& instrucao) { //Faz a decodificacao
    string bitsDecod;

<<<<<<< HEAD
struct Registers {
    uint32_t H;
    uint32_t OPC;
    uint32_t TOS;
    uint32_t CPP;
    uint32_t LV;
    uint32_t SP;
    uint32_t PC;
    uint32_t MDR;
    uint32_t MAR;
    uint8_t MBR; // 8 bits
};


int32_t signExtend(uint8_t value) {
    if (value & 0x80)
        return static_cast<int32_t>(0xFFFFFF00 | value);
    else
        return static_cast<int32_t>(value);
}


uint32_t zeroExtend(uint8_t value) {
    return static_cast<uint32_t>(value);
}


uint32_t aluOperation(uint32_t A, uint32_t B, const std::string &ctrl, bool &carryOut, bool &N, bool &Z) {
    bool SLL8 = (ctrl[0] == '1');
    bool SRA1 = (ctrl[1] == '1');
    bool F0   = (ctrl[2] == '1');
    bool F1   = (ctrl[3] == '1');
    bool ENA  = (ctrl[4] == '1');
    bool ENB  = (ctrl[5] == '1');
    bool INVA = (ctrl[6] == '1');
    bool INC  = (ctrl[7] == '1');

    uint32_t A_eff = ENA ? A : 0;
    uint32_t B_eff = 0;
    if (ENB)
        B_eff = INVA ? ~B : B;

    uint64_t result64 = 0;

    if (F0 && F1) {
        result64 = static_cast<uint64_t>(A_eff) + static_cast<uint64_t>(B_eff) + (INC ? 1 : 0);
    } else {

        result64 = A_eff;
    }
    carryOut = ((result64 >> 32) != 0);
    uint32_t result = static_cast<uint32_t>(result64);


    if (SLL8 && !SRA1)
        result = result << 8;
    else if (SRA1 && !SLL8)
        result = static_cast<uint32_t>(static_cast<int32_t>(result) >> 1);

    Z = (result == 0);
    N = ((result >> 31) & 1) == 1;

    return result;
}


uint32_t getBusBValue(const std::string &codeStr, const Registers &reg, std::string &bRegName) {
    int code = std::stoi(codeStr, nullptr, 2);
    switch(code) {
        case 8: bRegName = "opc"; return reg.OPC;
        case 7: bRegName = "tos"; return reg.TOS;
        case 6: bRegName = "cpp"; return reg.CPP;
        case 5: bRegName = "lv";  return reg.LV;
        case 4: bRegName = "sp";  return reg.SP;
        case 3: bRegName = "mbru"; return zeroExtend(reg.MBR);
        case 2: bRegName = "mbr";  return static_cast<uint32_t>(signExtend(reg.MBR));
        case 1: bRegName = "pc";  return reg.PC;
        case 0: bRegName = "mdr"; return reg.MDR;
        default:
            bRegName = "indefinido";
            return 0;
    }
}

=======
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
>>>>>>> 58500a3f97d25456a1499b8cd606dbea29a0ab4a

std::vector<std::string> getBusCRegisters(const std::string &sel) {
    std::vector<std::string> regs;
    if (sel.size() != 9) return regs;
    if (sel[0] == '1') regs.push_back("h");
    if (sel[1] == '1') regs.push_back("opc");
    if (sel[2] == '1') regs.push_back("tos");
    if (sel[3] == '1') regs.push_back("cpp");
    if (sel[4] == '1') regs.push_back("lv");
    if (sel[5] == '1') regs.push_back("sp");
    if (sel[6] == '1') regs.push_back("pc");
    if (sel[7] == '1') regs.push_back("mdr");
    if (sel[8] == '1') regs.push_back("mar");
    return regs;
}

void applyBusCSelector(const std::string &sel, Registers &reg, uint32_t Sd) {
    if (sel.size() != 9) return;
    if (sel[0] == '1') reg.H = Sd;
    if (sel[1] == '1') reg.OPC = Sd;
    if (sel[2] == '1') reg.TOS = Sd;
    if (sel[3] == '1') reg.CPP = Sd;
    if (sel[4] == '1') reg.LV = Sd;
    if (sel[5] == '1') reg.SP = Sd;
    if (sel[6] == '1') reg.PC = Sd;
    if (sel[7] == '1') reg.MDR = Sd;
    if (sel[8] == '1') reg.MAR = Sd;
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

<<<<<<< HEAD
std::string registersToString(const Registers &reg) {
    std::ostringstream oss;
    oss << "mar = " << std::bitset<32>(reg.MAR) << "\n"
        << "mdr = " << std::bitset<32>(reg.MDR) << "\n"
        << "pc  = " << std::bitset<32>(reg.PC)  << "\n"
        << "mbr = " << std::bitset<8>(reg.MBR)   << "\n"
        << "sp  = " << std::bitset<32>(reg.SP)  << "\n"
        << "lv  = " << std::bitset<32>(reg.LV)  << "\n"
        << "cpp = " << std::bitset<32>(reg.CPP) << "\n"
        << "tos = " << std::bitset<32>(reg.TOS) << "\n"
        << "opc = " << std::bitset<32>(reg.OPC) << "\n"
        << "h   = " << std::bitset<32>(reg.H);
    return oss.str();
=======
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
>>>>>>> 58500a3f97d25456a1499b8cd606dbea29a0ab4a
}

int main() {

    std::vector<std::string> instructions;

    std::ifstream fin("tarefa2Etp2.txt");
    if (!fin) {
        std::cerr << "Nao foi possivel abrir o arquivo de instrucoes." << std::endl;
        return 1;
    }


    std::string line;
    while (std::getline(fin, line)) {
        if(!line.empty())
            instructions.push_back(line);
    }
    fin.close();

    std::ofstream flog("log.txt");
    if (!flog) {
        std::cerr << "Nao foi possivel criar o arquivo de log." << std::endl;
        return 1;
    }

<<<<<<< HEAD
=======
    vector<vector<int>> Palavras;       // i representa as operacoes ou linhas de codigo, j representa os 6 bits
    int PC = 0;                         // Program Count = linhas de codigo executadas
    int IR;                             // Instrucao atual 
>>>>>>> 58500a3f97d25456a1499b8cd606dbea29a0ab4a

    for (const auto &instr : instructions) {
        flog << instr << std::endl;
    }

<<<<<<< HEAD
    flog << "\n=====================================================" << std::endl;
    flog << "> Initial register states" << std::endl;


    Registers reg;
=======
    vector<bitset<32>> resultado;
    cout << "Start of Program" << endl;

    for (int i = 0; i < Palavras.size(); i++) {
        PC = i + 1;
        int op0 = Palavras[i][2];
        int op1 = Palavras[i][3];
        
        bitset<32> A = H;
        bitset<32> B = Decod(Palavras[i]);
>>>>>>> 58500a3f97d25456a1499b8cd606dbea29a0ab4a

    reg.MAR = 0x00000000;
    reg.MDR = 0x00000000;
    reg.PC  = 0x00000000;
    reg.MBR = 0x81; // 10000001
    reg.SP  = 0x00000000;
    reg.LV  = 0x00000000;
    reg.CPP = 0x00000000;
    reg.TOS = 0x00000002;
    reg.OPC = 0x00000000;
    reg.H   = 0x00000001;

    flog << registersToString(reg) << std::endl;

    flog << "\n=====================================================" << std::endl;
    flog << "Start of program" << std::endl;
    flog << "=====================================================" << std::endl;


    int cycle = 1;
    for (const auto &instrLine : instructions) {

        if (instrLine.length() != 21) continue;

        flog << "Cycle " << cycle << std::endl;

        std::string ctrl8 = instrLine.substr(0, 8);    // 8 bits de controle da ULA
        std::string sel9  = instrLine.substr(8, 9);     // 9 bits do seletor do barramento C
        std::string dec4  = instrLine.substr(17, 4);    // 4 bits do decodificador do barramento B


        flog << "ir = " << ctrl8 << " " << sel9 << " " << dec4 << std::endl << std::endl;


        std::string bRegName;
        uint32_t busB_val = getBusBValue(dec4, reg, bRegName);
        flog << "b_bus = " << bRegName << std::endl;


        std::vector<std::string> cRegs = getBusCRegisters(sel9);
        flog << "c_bus = ";
        if (cRegs.empty()) flog << "Nenhum";
        else {
            for (size_t i = 0; i < cRegs.size(); i++) {
                flog << cRegs[i];
                if (i < cRegs.size()-1)
                    flog << ", ";
            }
        }
        flog << std::endl << std::endl;


        flog << "> Registers before instruction" << std::endl;
        flog << registersToString(reg) << std::endl << std::endl;


        bool carryOut = false, flagN = false, flagZ = false;

        uint32_t Sd = aluOperation(reg.H, busB_val, ctrl8, carryOut, flagN, flagZ);


        applyBusCSelector(sel9, reg, Sd);


        flog << "> Registers after instruction" << std::endl;
        flog << registersToString(reg) << std::endl;
        flog << "=====================================================" << std::endl;

        cycle++;
    }

    flog << "Cycle " << cycle << std::endl;
    flog << "No more lines, EOP." << std::endl;

    flog.close();
    std::cout << "Processamento concluido. Verifique o arquivo 'log.txt'." << std::endl;
    return 0;
}
