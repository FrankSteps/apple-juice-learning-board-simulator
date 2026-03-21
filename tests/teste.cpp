/*
    Testes feitos por Renato antes do merge.
    Neste arquivo, a interface gráfica com raylib não havia sido implementada ainda, mas o sistema
    de visualização no próprio terminal já estava mais consolidada.
*/

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdint>
#include <string>
#include <atomic>

#include <mutex>

// Chip555: gerador de clock (NE555 em modo astável)
class Chip555{
    private:
        // configuração dos componentes eletrônicos -> 1 capacitor e 2 resistores
        // output começa em LOW (false)
        bool output = false;
        double R1, R2, C;
        const double Ln2 = 0.693;

        double tHigh(){
            return Ln2*(R1 + R2)*C;
        }

        double tLow(){
            return Ln2*R2*C;
        }

        void clock(){
            output = !output;
        }

    public:
        Chip555(double r1, double r2, double c )
            : R1(r1), R2(r2), C(c) {}

        // um período completo (HIGH + LOW)
        void pulse(){
            clock();
            std::this_thread::sleep_for(std::chrono::duration<double>(tHigh()));
            clock();
            std::this_thread::sleep_for(std::chrono::duration<double>(tLow()));
        }

        // útil pra debug/print
        double period() const {
            // tHigh + tLow
            return 0.693*(R1 + 2.0*R2)*C;
        }
};

// Chipe4017 (versão baseada no modelo: bit "1" deslocando-se para a direita)
class Chip4017{
    public:
        explicit Chip4017(unsigned int limitReset) : LimitReset(limitReset){
            if (LimitReset < 1 || LimitReset > 10) {
                throw std::invalid_argument("O pino de reset tem que estar configurado entre 1 e 10");
            }
            reset();
        }
         void shift() {
        // Desloca o bit ligado para a direita; quando sair, volta ao topo
        Out >>= 1;
        if (Out == 0) {
            Out = 1u << (LimitReset - 1);
            }
        }

        void reset() {
            // Estado inicial: última posição ligada 
            Out = 1u << (LimitReset - 1);
        }

        uint32_t getOut() const { return Out; }
        unsigned getLimitReset() const { return LimitReset; }

    
    
    private:
        unsigned LimitReset;
        uint32_t Out{0};
};


//painel de lampadas(renderização)
class LampPanel {
    public:
        explicit LampPanel(unsigned n) : n_(n) {}

        void draw(uint32_t bits, bool ligado) const {
            clearScreen();

            std::cout << "=== Simulador (estilo placa) ===\n";
            std::cout << "Status: " << (ligado ? "\033[1;92mLIGADO\033[0m" : "\033[91mDESLIGADO\033[0m") << "\n\n";

            // Mostra lâmpadas da esquerda para a direita (n_ lâmpadas)
            // Como seu Out usa bit alto primeiro, vamos mapear do bit (n_-1) até 0:
            std::cout << "\033[1;95mLâmpadas:\033[0m ";
            for (int i = (int)n_ - 1; i >= 0; --i) {
                bool on = (bits >> i) & 1u;
                if (on) {
                    std::cout << "\033[1;92m● \033[0m";  // verde brilhante
                } else {
                    std::cout << "\033[92m○ \033[0m";
                }
            }
            std::cout << "\n\n";

            std::cout << "Controles:\n";
            std::cout << "  ENTER  -> liga/desliga\n";
            std::cout << "  r      -> reset\n";
            std::cout << "  0      -> sair\n";
            std::cout << "\n";
            std::cout.flush();
        }

    private:
        unsigned n_;

        static void clearScreen() {
            // ANSI: limpa tela e volta cursor pro topo (funciona bem no Linux)
            std::cout << "\033[2J\033[H";
        }
};

int main() {
    try {
        const unsigned LAMPADAS = 8;

        
        const double R1 = 10000.0;     // 10kΩ
        const double R2 = 10000.0;     // 10kΩ
        const double C  = 11e-6;       // 22µF
        Chip555 chip555(R1, R2, C);

        Chip4017 chip4017(LAMPADAS);
        LampPanel painel(LAMPADAS);

        std::atomic<bool> running{true};
        std::atomic<bool> ligado{false};

        std::mutex mtx; // protege o estado do 4017 (shift/reset/getOut)

        // Desenha o estado inicial
        {
            std::lock_guard<std::mutex> lock(mtx);
            painel.draw(chip4017.getOut(), ligado.load());
        }

        // Thread do "motor": só gera pulsos quando estiver ligado
        std::thread motor([&]{
            while (running.load()) {
                if (ligado.load()) {
                    // 555 gera o "tempo" do clock (um ciclo completo)
                    chip555.pulse();

                    // na borda do clock (simplificado): avança uma vez por pulso
                    {
                        std::lock_guard<std::mutex> lock(mtx);
                        chip4017.shift();
                        painel.draw(chip4017.getOut(), true);
                    }
                } else {
                    // Quando desligado, dorme um pouco pra não consumir CPU
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            }
        });

        // Loop de entrada: ENTER alterna ligado/desligado, "0" sai, "r" reset
        std::string line;
        while (running.load()) {
            std::getline(std::cin, line);
            if (!std::cin) break; // EOF / erro

            if (line == "0") {
                running.store(false);
                break;
            }

            if (line == "r" || line == "R") {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    chip4017.reset();
                    painel.draw(chip4017.getOut(), ligado.load());
                }
                continue;
            }

            if (line.empty()) {
                // ENTER: toggle
                bool novo = !ligado.load();
                ligado.store(novo);

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    painel.draw(chip4017.getOut(), novo);
                }
                continue;
            }

            // qualquer outra coisa: ignora e redesenha
            {
                std::lock_guard<std::mutex> lock(mtx);
                painel.draw(chip4017.getOut(), ligado.load());
            }
        }

        running.store(false);
        if (motor.joinable()) motor.join();

        std::cout << "\nEncerrado.\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }
}