#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <bitset>
#include <cstdint>
#include <vector>
#include <iomanip>


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


    for (const auto &instr : instructions) {
        flog << instr << std::endl;
    }

    flog << "\n=====================================================" << std::endl;
    flog << "> Initial register states" << std::endl;


    Registers reg;

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
