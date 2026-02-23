/*
    Arquivo de testes unitários — Simulador Apple Juice
    Verifica o comportamento esperado dos chips: Chip4026, Chip555 e Chip4017.

    Compilação (sem raylib): g++ testes.cpp -o testes -std=c++17
    Execução:                ./testes

    Feito por Arthur
*/

#include <iostream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <cmath>
#include <cstdint>
#include <atomic>
#include <bitset>

// ─── Infraestrutura de testes ────────────────────────────────────────────────

static int totalTestes = 0;
static int totalPassou = 0;

static void check(bool condicao, const std::string& descricao) {
    totalTestes++;
    if (condicao) {
        std::cout << "  [OK] " << descricao << "\n";
        totalPassou++;
    } else {
        std::cout << "  [FALHOU] " << descricao << "\n";
    }
}

// Verifica se o bloco lança a exceção esperada
template<typename ExcecaoEsperada, typename Bloco>
static void checkThrows(Bloco bloco, const std::string& descricao) {
    totalTestes++;
    try {
        bloco();
        std::cout << "  [FALHOU] " << descricao << " (nenhuma exceção lançada)\n";
    } catch (const ExcecaoEsperada&) {
        std::cout << "  [OK] " << descricao << "\n";
        totalPassou++;
    } catch (...) {
        std::cout << "  [FALHOU] " << descricao << " (exceção errada)\n";
    }
}

// ─── Cópias das classes (sem dependência de raylib) ──────────────────────────

class Chip {
public:
    virtual ~Chip() = default;
    virtual void visualize() const = 0;
};

class Chip4026 : public Chip {
protected:
    bool carryOut = false;
    unsigned int Out = 0;
public:
    virtual ~Chip4026() = default;

    virtual void add() {
        if (Out == 9) { Out = 0; carryOut = true; }
        else          { Out++;   carryOut = false; }
    }

    virtual void reset() { Out = 0; carryOut = false; }

    unsigned int getOut()    const { return Out; }
    bool         getCarryOut() const { return carryOut; }

    void visualize() const override {
        std::cout << "[CD4026] Out=" << Out << " | CarryOut=" << (carryOut ? "true" : "false") << "\n";
    }
};

class Unidade : public Chip4026 {};

class Dezena : public Chip4026 {
private:
    using Chip4026::add;
public:
    void addOnCarry(bool carryIn) { if (carryIn) add(); }
};

class Chip555 : public Chip {
private:
    double R1, R2, C;
    double tHigh = 0.0, tLow = 0.0, period = 0.0, freq = 0.0;
    const double Ln2 = 0.693;
    std::atomic<bool> stateHigh{false};

    void calcTimings() {
        tHigh  = Ln2 * (R1 + R2) * C;
        tLow   = Ln2 * R2 * C;
        period = tHigh + tLow;
        freq   = (period > 0) ? (1.0 / period) : 0.0;
    }
public:
    Chip555(double r1, double r2, double c) : R1(r1), R2(r2), C(c) {
        if (R1 <= 0 || R2 <= 0 || C <= 0)
            throw std::invalid_argument("R1, R2 e C precisam ser > 0");
        calcTimings();
    }

    void reset()             { stateHigh = false; }
    bool   isHigh()    const { return stateHigh; }
    double getFrequency() const { return freq; }
    double getPeriod()    const { return period; }

    void visualize() const override {
        std::cout << "[NE555] f=" << freq << " Hz | T=" << period
                  << " s | State=" << (stateHigh ? "HIGH" : "LOW") << "\n";
    }
};

class Chip4017 : public Chip {
private:
    unsigned LimitReset;
    uint32_t Out{0};
public:
    explicit Chip4017(unsigned limitReset) : LimitReset(limitReset) {
        if (LimitReset < 1 || LimitReset > 10)
            throw std::invalid_argument("LimitReset precisa estar entre 1 e 10.");
        reset();
    }

    void shift() {
        Out >>= 1;
        if (Out == 0) Out = 1u << (LimitReset - 1);
    }

    void reset()               { Out = 1u << (LimitReset - 1); }
    uint32_t getOut()    const { return Out; }
    unsigned getLimitReset() const { return LimitReset; }

    void visualize() const override {
        std::cout << "[CD4017] Out=0b" << std::bitset<10>(Out)
                  << " | LimitReset=" << LimitReset << "\n";
    }
};

// ─── Testes ──────────────────────────────────────────────────────────────────

void testarChip4026() {
    std::cout << "\n[Chip4026 / Unidade / Dezena]\n";

    Unidade u;
    check(u.getOut() == 0,        "estado inicial é 0");
    check(u.getCarryOut() == false, "carry inicial é false");

    for (int i = 0; i < 9; i++) u.add();
    check(u.getOut() == 9,         "após 9 incrementos Out == 9");
    check(u.getCarryOut() == false, "carry ainda false antes do estouro");

    u.add(); // estouro
    check(u.getOut() == 0,        "após estouro Out volta a 0");
    check(u.getCarryOut() == true, "carry ativado no estouro");

    u.add();
    check(u.getCarryOut() == false, "carry desativado após primeiro incremento pós-estouro");

    u.reset();
    check(u.getOut() == 0,         "reset zera Out");
    check(u.getCarryOut() == false, "reset zera carryOut");

    // Dezena: só avança com carry = true
    Dezena d;
    d.addOnCarry(false);
    check(d.getOut() == 0, "addOnCarry(false) não incrementa");

    d.addOnCarry(true);
    check(d.getOut() == 1, "addOnCarry(true) incrementa");
}

void testarChip555() {
    std::cout << "\n[Chip555]\n";

    // Construção válida
    Chip555 c(10000.0, 10000.0, 11e-6);
    check(c.getFrequency() > 0,  "frequência positiva após construção");
    check(c.getPeriod() > 0,     "período positivo após construção");
    check(c.isHigh() == false,   "estado inicial LOW");

    // Verificação aproximada da fórmula: f = 1 / (0.693 * (R1 + 2*R2) * C)
    double fEsperada = 1.0 / (0.693 * (10000.0 + 2.0 * 10000.0) * 11e-6);
    check(std::fabs(c.getFrequency() - fEsperada) < 0.01, "frequência calculada corretamente");

    c.reset();
    check(c.isHigh() == false, "reset mantém estado LOW");

    // Construção inválida
    checkThrows<std::invalid_argument>([]{ Chip555(0, 10000, 11e-6); },  "R1 = 0 lança invalid_argument");
    checkThrows<std::invalid_argument>([]{ Chip555(10000, -1, 11e-6); }, "R2 negativo lança invalid_argument");
    checkThrows<std::invalid_argument>([]{ Chip555(10000, 10000, 0); },  "C = 0 lança invalid_argument");
}

void testarChip4017() {
    std::cout << "\n[Chip4017]\n";

    Chip4017 c(4);
    // Com LimitReset=4, estado inicial: 0b1000 = 8
    check(c.getOut() == 0b1000, "estado inicial correto para LimitReset=4");

    c.shift();
    check(c.getOut() == 0b0100, "shift desloca bit para direita");

    c.shift();
    c.shift();
    c.shift(); // volta ao início após 4 shifts
    check(c.getOut() == 0b1000, "após LimitReset shifts, retorna ao estado inicial");

    c.shift();
    c.reset();
    check(c.getOut() == 0b1000, "reset restaura estado inicial");

    // Limites do construtor
    checkThrows<std::invalid_argument>([]{ Chip4017(0);  }, "LimitReset=0 lança invalid_argument");
    checkThrows<std::invalid_argument>([]{ Chip4017(11); }, "LimitReset=11 lança invalid_argument");

    // Limites válidos
    Chip4017 min(1);
    check(min.getOut() == 1, "LimitReset=1 inicializa com Out=1");

    Chip4017 max(10);
    check(max.getOut() == (1u << 9), "LimitReset=10 inicializa com bit na posição 9");
}

void testarPolimorfismo() {
    std::cout << "\n[Polimorfismo via Chip*]\n";

    // Verifica que os três chips funcionam através de ponteiro para a base abstrata
    Chip* chips[] = { new Unidade(), new Chip555(10000, 10000, 11e-6), new Chip4017(4) };

    bool todosVisualizaram = true;
    for (Chip* c : chips) {
        try { c->visualize(); }
        catch (...) { todosVisualizaram = false; }
        delete c;
    }
    check(todosVisualizaram, "visualize() chamado polimorficamente sem erros");
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "=== Testes — Simulador Apple Juice ===\n";

    testarChip4026();
    testarChip555();
    testarChip4017();
    testarPolimorfismo();

    std::cout << "\n──────────────────────────────────────\n";
    std::cout << "Resultado: " << totalPassou << "/" << totalTestes << " testes passaram.\n";

    return (totalPassou == totalTestes) ? 0 : 1;
}
