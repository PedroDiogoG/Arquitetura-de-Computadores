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
        case 8: bRegName = "sp";  break;
        case 7: bRegName = "tos"; break;
        case 6: bRegName = "cpp"; break;
        case 5: bRegName = "lv"; break;
        case 4: bRegName = "sp"; break;
        case 3: bRegName = "mbru"; break;
        case 2: bRegName = "mbr"; break;
        case 1: bRegName = "pc"; break;
        case 0: bRegName = "mdr"; break;
        default: bRegName = "indefinido"; break;
    }
    if(bRegName == "sp")
        return reg.SP;
    else if(bRegName == "tos")
        return reg.TOS;
    else if(bRegName == "cpp")
        return reg.CPP;
    else if(bRegName == "lv")
        return reg.LV;
    else if(bRegName == "mbru")
        return zeroExtend(reg.MBR);
    else if(bRegName == "mbr")
        return static_cast<uint32_t>(signExtend(reg.MBR));
    else if(bRegName == "pc")
        return reg.PC;
    else // "mdr"
        return reg.MDR;
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
    oss << "*******************************" << "\n";
    oss << "mar = " << std::bitset<32>(reg.MAR) << "\n"
        << "mdr = " << std::bitset<32>(reg.MDR) << "\n"
        << "pc  = " << std::bitset<32>(reg.PC)  << "\n"
        << "mbr = " << std::bitset<8>(reg.MBR)   << "\n"
        << "sp  = " << std::bitset<32>(reg.SP)  << "\n"
        << "lv  = " << std::bitset<32>(reg.LV)  << "\n"
        << "cpp = " << std::bitset<32>(reg.CPP) << "\n"
        << "tos = " << std::bitset<32>(reg.TOS) << "\n"
        << "opc = " << std::bitset<32>(reg.OPC) << "\n"
        << "h   = " << std::bitset<32>(reg.H)   << "\n";
    oss << "*******************************";
    return oss.str();
}


std::string memoryToString(const std::vector<uint32_t> &mem) {
    std::ostringstream oss;
    oss << "*******************************" << "\n";
    for (size_t i = 0; i < mem.size(); i++) {
        oss << std::bitset<32>(mem[i]) << "\n";
    }
    oss << "*******************************";
    return oss.str();
}


std::vector<uint32_t> loadMemory(const std::string &filename) {
    std::vector<uint32_t> mem;
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Arquivo de dados não encontrado. Inicializando memória com valores padrão." << std::endl;

        mem.resize(16, 0);
        if(mem.size() >= 5)
            mem[4] = 0x00000002;
        return mem;
    }
    std::string line;
    while (std::getline(fin, line)) {
        if(!line.empty()){
            uint32_t word = static_cast<uint32_t>(std::stoul(line, nullptr, 2));
            mem.push_back(word);
        }
    }
    fin.close();

    if(mem.size() < 16)
        mem.resize(16, 0);
    return mem;
}


bool loadRegs(const std::string& filename, Registers& reg) {
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Erro ao abrir o arquivo de registradores." << std::endl;
        return false;
    }

    std::string linha;
    while (std::getline(fin, linha)) {
        if (linha.empty()) continue;

        std::istringstream iss(linha);
        std::string nome, igual, valor_bin;
        if (!(iss >> nome >> igual >> valor_bin)) {
            std::cerr << "Linha mal formatada: " << linha << std::endl;
            continue;
        }

        // Remove o caractere '=' e transforma o nome em minúsculo (opcional)
        if (nome.back() == '=') nome.pop_back();
        for (auto& c : nome) c = std::tolower(c);

        // Converte binário para inteiro
        uint32_t valor = 0;
        if (valor_bin.length() == 8)
            valor = static_cast<uint8_t>(std::bitset<8>(valor_bin).to_ulong());
        else
            valor = static_cast<uint32_t>(std::bitset<32>(valor_bin).to_ulong());

        // Atribui ao registrador correspondente
        if (nome == "mar") reg.MAR = valor;
        else if (nome == "mdr") reg.MDR = valor;
        else if (nome == "pc")  reg.PC  = valor;
        else if (nome == "mbr") reg.MBR = static_cast<uint8_t>(valor);
        else if (nome == "sp")  reg.SP  = valor;
        else if (nome == "lv")  reg.LV  = valor;
        else if (nome == "cpp") reg.CPP = valor;
        else if (nome == "tos") reg.TOS = valor;
        else if (nome == "opc") reg.OPC = valor;
        else if (nome == "h")   reg.H   = valor;
        else std::cerr << "Registrador desconhecido: " << nome << std::endl;
    }

    fin.close();
    return true;
}

int main() {

    std::vector<std::string> instructions;
    std::ifstream fin("micro.txt");
    if (!fin) {
        std::cerr << "Não foi possível abrir o arquivo de microinstruções." << std::endl;
        return 1;
    }
    std::string line;
    while (std::getline(fin, line)) {
        if(!line.empty())
            instructions.push_back(line);
    }
    fin.close();


    std::vector<uint32_t> memory = loadMemory("dados.txt");

    std::ofstream flog("log.txt");
    if (!flog) {
        std::cerr << "Não foi possível criar o arquivo de log." << std::endl;
        return 1;
    }


    flog << "============================================================" << "\n";
    flog << "Initial memory state" << "\n";
    flog << memoryToString(memory) << "\n";


    Registers reg;
    if (!loadRegs("registradores.txt", reg)) {
        return 1;
    }
    /*
    reg.MAR = 0x00000004;
    reg.MDR = 0x00000000;
    reg.PC  = 0x00000000;
    reg.MBR = 0x00;
    reg.SP  = 0x00000004;
    reg.LV  = 0x00000000;
    reg.CPP = 0x00000000;
    reg.TOS = 0x00000000;
    reg.OPC = 0x00000000;
    reg.H   = 0x00000000;
    */
    flog << "============================================================" << "\n";
    flog << "Initial register state" << "\n";
    flog << registersToString(reg) << "\n";
    flog << "============================================================" << "\n";
    flog << "Start of Program" << "\n";
    flog << "============================================================" << "\n";

    int cycle = 1;
    for (const auto &instrLine : instructions) {
        if (instrLine.length() != 23) continue;

        flog << "Cycle " << cycle << "\n";

        std::string ctrl8 = instrLine.substr(0, 8);
        std::string busC9 = instrLine.substr(8, 9);
        std::string mem2  = instrLine.substr(17, 2);
        std::string dec4  = instrLine.substr(19, 4);

        flog << "ir = " << ctrl8 << " " << busC9 << " " << mem2 << " " << dec4 << "\n";


        std::string bRegName;
        uint32_t busB_val = getBusBValue(dec4, reg, bRegName);
        flog << "b = " << bRegName << "\n";


        std::vector<std::string> cRegs = getBusCRegisters(busC9);
        flog << "c = ";
        if(cRegs.empty())
            flog << "Nenhum";
        else {
            for (size_t i = 0; i < cRegs.size(); i++) {
                flog << cRegs[i];
                if(i < cRegs.size()-1)
                    flog << ", ";
            }
        }
        flog << "\n\n";

        flog << "> Registers before instruction" << "\n";
        flog << registersToString(reg) << "\n\n";

        bool carryOut = false, flagN = false, flagZ = false;
        uint32_t Sd = aluOperation(reg.H, busB_val, ctrl8, carryOut, flagN, flagZ);


        applyBusCSelector(busC9, reg, Sd);


        if (mem2 == "10") {
            size_t addr = reg.MAR;
            if(addr < memory.size()) {
                memory[addr] = reg.MDR;
                flog << "Memory WRITE: address " << addr << " updated with MDR value." << "\n";
            } else {
                flog << "Memory WRITE: address out of range!" << "\n";
            }
        } else if (mem2 == "01") {
            size_t addr = reg.MAR;
            if(addr < memory.size()) {
                reg.MDR = memory[addr];
                flog << "Memory READ: MDR updated with memory content at address " << addr << "." << "\n";
            } else {
                flog << "Memory READ: address out of range!" << "\n";
            }
        }

        flog << "\n> Registers after instruction" << "\n";
        flog << registersToString(reg) << "\n\n";
        flog << "> Memory after instruction" << "\n";
        flog << memoryToString(memory) << "\n";
        flog << "============================================================" << "\n";
        cycle++;
    }

    flog << "Cycle " << cycle << "\n";
    flog << "No more lines, EOP." << "\n";
    flog.close();

    std::cout << "Processamento concluido. Verifique o arquivo 'log.txt'." << std::endl;
    return 0;
}
