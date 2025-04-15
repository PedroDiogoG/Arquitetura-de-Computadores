#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <bitset>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <algorithm>

// regs
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

//funções para extensão de sinal
int32_t signExtend(uint8_t value) {
    return (value & 0x80) ? static_cast<int32_t>(0xFFFFFF00 | value)
                          : static_cast<int32_t>(value);
}

uint32_t zeroExtend(uint8_t value) {
    return static_cast<uint32_t>(value);
}

// Campos da microinstrução de 23 bits:
//   bits 0-7: controle da ULA (ctrl8)
//   bits 8-16: seletor do barramento C (busC, 9 bits)
//   bits 17-18: controle de memória (mem2)
//   bits 19-22: decod do barramento B (dec4)
uint32_t aluOperation(uint32_t A, uint32_t B, const std::string &ctrl,
                        bool &carryOut, bool &N, bool &Z) {
    bool SLL8 = (ctrl[0] == '1');
    bool SRA1 = (ctrl[1] == '1');
    bool F0   = (ctrl[2] == '1');
    bool F1   = (ctrl[3] == '1');
    bool ENA  = (ctrl[4] == '1');
    bool ENB  = (ctrl[5] == '1');
    bool INVA = (ctrl[6] == '1');
    bool INC  = (ctrl[7] == '1');

    uint32_t A_eff = ENA ? A : 0;
    uint32_t B_eff = (ENB) ? (INVA ? ~B : B) : 0;
    uint64_t result64 = 0;
    if (F0 && F1)
        result64 = static_cast<uint64_t>(A_eff) + static_cast<uint64_t>(B_eff) + (INC ? 1 : 0);
    else
        result64 = A_eff;
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

// decod do barramento B
// Mapeamento (4 bits, dec4): 8:"opc", 7:"tos", 6:"cpp", 5:"lv", 4:"sp", 3:"mbru", 2:"mbr", 1:"pc", 0:"mdr"
uint32_t getBusBValue(const std::string &codeStr, const Registers &reg, std::string &bRegName) {
    int code = std::stoi(codeStr, nullptr, 2);
    if (code == 4) { bRegName = "sp"; return reg.SP; }
    else if (code == 0) { bRegName = "mdr"; return reg.MDR; }
    else if (code == 1) { bRegName = "pc"; return reg.PC; }
    else if (code == 2) { bRegName = "mbr"; return static_cast<uint32_t>(signExtend(reg.MBR)); }
    else if (code == 3) { bRegName = "mbru"; return zeroExtend(reg.MBR); }
    else if (code == 5) { bRegName = "lv"; return reg.LV; }
    else if (code == 6) { bRegName = "cpp"; return reg.CPP; }
    else if (code == 7) { bRegName = "tos"; return reg.TOS; }
    else if (code == 8) { bRegName = "opc"; return reg.OPC; }
    else { bRegName = "indefinido"; return 0; }
}

// regs habilitados no barramento C a partir do campo de 9 bits
// Mapeamento (posição 0 a 8): 0:"h", 1:"opc", 2:"tos", 3:"cpp", 4:"lv", 5:"sp", 6:"pc", 7:"mdr", 8:"mar".
std::vector<std::string> getBusCRegisters(const std::string &sel) {
    std::vector<std::string> regs;
    if (sel.size() != 9) return regs;
    if (sel[0]=='1') regs.push_back("h");
    if (sel[1]=='1') regs.push_back("opc");
    if (sel[2]=='1') regs.push_back("tos");
    if (sel[3]=='1') regs.push_back("cpp");
    if (sel[4]=='1') regs.push_back("lv");
    if (sel[5]=='1') regs.push_back("sp");
    if (sel[6]=='1') regs.push_back("pc");
    if (sel[7]=='1') regs.push_back("mdr");
    if (sel[8]=='1') regs.push_back("mar");
    return regs;
}

//att os regs do barramento C de acordo com o seletor (9 bits)
// Ordem: [0]: H, [1]: OPC, [2]: TOS, [3]: CPP, [4]: LV, [5]: SP, [6]: PC, [7]: MDR, [8]: MAR.
void applyBusCSelector(const std::string &sel, Registers &reg, uint32_t Sd) {
    if (sel.size() != 9) return;
    if (sel[0]=='1') reg.H = Sd;
    if (sel[1]=='1') reg.OPC = Sd;
    if (sel[2]=='1') reg.TOS = Sd;
    if (sel[3]=='1') reg.CPP = Sd;
    if (sel[4]=='1') reg.LV = Sd;
    if (sel[5]=='1') reg.SP = Sd;
    if (sel[6]=='1') reg.PC = Sd;
    if (sel[7]=='1') reg.MDR = Sd;
    if (sel[8]=='1') reg.MAR = Sd;
}

// Funções para formatar o estado da memória e dos registradores para o log
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
    for (size_t i = 0; i < mem.size(); i++)
        oss << std::bitset<32>(mem[i]) << "\n";
    oss << "*******************************";
    return oss.str();
}

// carrega a memoria de um arquivo, se nao, inicia com 16 bits
std::vector<uint32_t> loadMemory(const std::string &filename) {
    std::vector<uint32_t> mem;
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Arquivo de dados nao encontrado. Inicializando memoria com valores padrao." << std::endl;
        mem.resize(16, 0);
        if (mem.size() >= 5)
            mem[4] = 0x00000002;
        return mem;
    }
    std::string line;
    while (std::getline(fin, line)) {
        if (!line.empty())
            mem.push_back(static_cast<uint32_t>(std::stoul(line, nullptr, 2)));
    }
    fin.close();
    if (mem.size() < 16)
        mem.resize(16, 0);
    return mem;
}


// traducao de ILOAD
std::vector<std::string> translateILOAD(int x) {
    std::vector<std::string> micro;
    // Mic 1: H = LV
    micro.push_back("00110001" + std::string("100000000") + "00" + "0000");
    // Para cada incremento: H = H+1
    for (int i = 0; i < x; i++) {
        micro.push_back("00110010" + std::string("100000000") + "00" + "0000");
    }
    // Mic para leitura: MAR = H; rd
    micro.push_back("00111000" + std::string("000000001") + "01" + "0000");
    // Mic para escrita: MAR = SP = SP+1; wr (dec4 "0100" para selecionar sp)
    micro.push_back("00110100" + std::string("000000001") + "10" + "0100");
    // Mic para copiar: TOS = MDR
    micro.push_back("00110111" + std::string("001000000") + "00" + "0000");
    return micro;
}

// trad DUP
std::vector<std::string> translateDUP() {
    std::vector<std::string> micro;
    // Mic 1: MAR = SP = SP+1;
    micro.push_back("00110011" + std::string("000000001") + "00" + "0100");
    // Mic 2: MDR = TOS; wr
    micro.push_back("00111100" + std::string("010000000") + "10" + "0000");
    return micro;
}


std::vector<std::string> translateBIPUSH(const std::string &arg) {
    std::vector<std::string> micro;
    // Mic 1: SP = MAR = SP+1
    micro.push_back("00110100" + std::string("000000001") + "00" + "0100");
    // Mic 2: fetch especial, onde os 8 arg bits são os primeiros bits
    micro.push_back(arg + std::string("000000000") + "11" + "0000");
    // Mic 3: MDR = TOS = H; wr
    micro.push_back("00111100" + std::string("010000000") + "10" + "0000");

    return micro;
}

//trad ijvm
std::vector<std::string> translateInstructions(const std::string &filename) {
    std::vector<std::string> microSeq;
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Nao foi possivel abrir o arquivo de instrucoes." << std::endl;
        return microSeq;
    }
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string instr;
        iss >> instr;

        std::transform(instr.begin(), instr.end(), instr.begin(), ::toupper);
        if (instr == "ILOAD") {
            int x;
            iss >> x;
            std::vector<std::string> aux = translateILOAD(x);
            microSeq.insert(microSeq.end(), aux.begin(), aux.end());
        } else if (instr == "DUP") {
            std::vector<std::string> aux = translateDUP();
            microSeq.insert(microSeq.end(), aux.begin(), aux.end());
        } else if (instr == "BIPUSH") {
            std::string arg;
            iss >> arg;
            std::vector<std::string> aux = translateBIPUSH(arg);
            microSeq.insert(microSeq.end(), aux.begin(), aux.end());
        } else {
            std::cerr << "Instrucao desconhecida: " << instr << std::endl;
        }
    }
    fin.close();
    return microSeq;
}

// le os regs de um arquivo
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
    // instrucoes -> microinstrucoes(23 bits)
    std::vector<std::string> microinstructions = translateInstructions("micro.txt");
    if (microinstructions.empty()) {
        std::cerr << "Nenhuma microinstrucao gerada. Encerrando." << std::endl;
        return 1;
    }

    // carrega memoria
    std::vector<uint32_t> memory = loadMemory("dados.txt");

    // escrita no log
    std::ofstream flog("log.txt");
    if (!flog) {
        std::cerr << "Nao foi possivel criar o arquivo de log." << std::endl;
        return 1;
    }

    //estado inicial
    flog << "============================================================" << "\n";
    flog << "Initial memory state" << "\n";
    flog << memoryToString(memory) << "\n";

    // le os regs
    Registers reg;
    if (!loadRegs("registradores.txt", reg)) {
        return 1;
    }


    flog << "============================================================" << "\n";
    flog << "Initial register state" << "\n";
    flog << registersToString(reg) << "\n";
    flog << "============================================================" << "\n";
    flog << "Start of Program" << "\n";
    flog << "============================================================" << "\n";

    int cycle = 1;
    // Executa as mics (uma por ciclo)
    for (const auto &instrLine : microinstructions) {
        if (instrLine.length() != 23) continue;

        flog << "Cycle " << cycle << "\n";
        // Divide a mic em 4 campos
        std::string ctrl8 = instrLine.substr(0, 8);
        std::string busC9 = instrLine.substr(8, 9);
        std::string mem2  = instrLine.substr(17, 2);
        std::string dec4  = instrLine.substr(19, 4);

        flog << "ir = " << ctrl8 << " " << busC9 << " " << mem2 << " " << dec4 << "\n";

        // Determina o reg do barramento B
        std::string bRegName;
        uint32_t busB_val = getBusBValue(dec4, reg, bRegName);
        flog << "b = " << bRegName << "\n";

        // Determina os regs habilitados no barramento C
        std::vector<std::string> cRegs = getBusCRegisters(busC9);
        flog << "c = ";
        if (cRegs.empty()) flog << "Nenhum";
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

        // Executa a operação da ULA
        bool carryOut = false, flagN = false, flagZ = false;
        uint32_t Sd = aluOperation(reg.H, busB_val, ctrl8, carryOut, flagN, flagZ);

        // Atualiza os regs do barramento C com o resultado (Sd)
        applyBusCSelector(busC9, reg, Sd);

        // "10" -> WRITE: escreve MDR na posição de memória indicada por MAR
        // "01" -> READ: atualiza MDR com o conteúdo da memória em MAR
        // "11" será tratado como a instrução fetch, fazendo H = MBR.
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
                flog << "Memory READ: MDR updated with memory content at address " << addr << ".\n";
            } else {
                flog << "Memory READ: address out of range!\n";
            }
        } else if (mem2 == "11") {

            reg.H = zeroExtend(reg.MBR);
            flog << "Fetch: H updated with zero-extended MBR value.\n";
        }

        flog << "\n> Registers after instruction" << "\n";
        flog << registersToString(reg) << "\n\n";
        flog << "> Memory after instruction" << "\n";
        flog << memoryToString(memory) << "\n";
        flog << "============================================================" << "\n";
        cycle++;
    }

    flog << "Cycle " << cycle << "\n";
    flog << "No more lines, EOP.\n";
    flog.close();

    std::cout << "Processamento concluido. Verifique o arquivo 'log.txt'." << std::endl;
    return 0;
}
