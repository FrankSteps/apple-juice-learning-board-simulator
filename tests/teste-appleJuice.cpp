/*
    Apenas o código base desenvolvido por Francisco apenas para servir de guia para o andamento do projeto
    Essas ideias foram amadurecendo conforme os colaboradores foram implementando suas lógicas e suas próprias ideias.
*/

// bibliotecas principais 
#include <iostream>
#include <string>
#include <stdlib.h>
#include <chrono>
#include <thread>


// Struct para tratamento de erros críticos: imprime a mensagem e encerra o programa imediatamente
struct ErrorMsg{
    [[noreturn]] static void CriticalError(const std::string& msg){
        std::cerr << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
};


// Converte um número decimal para binário e exibe no terminal
struct Converter{
    static void toBinary(unsigned int decimal, int bits){
        for(int i = bits-1; i >= 0; i--){
            std::cout << ((decimal >> i) & 1);
        }
        std::cout << std::endl;
    }
};


// classe responsável por simular o comportamento do chip CD4017 (um contador Johnson)
class Chip4017{
    private:
        unsigned int Out;            // inicializa a saída com o bit menos significativo em 1
        unsigned int LimitReset;     // criando o limite

    public:
        // Limita o valor do reset entre 1 e 10, como no chip real
        explicit Chip4017(unsigned int limitReset) : LimitReset(limitReset) {
            if(limitReset == 0 || limitReset > 10){
                throw std::invalid_argument("O pino de reset tem que estar configurado entre 0 e 10");
            }

            // inicia com bit mais significativo 1
            Out = 1u << (LimitReset - 1);   
        };

        // Desloca o bit '1' para a direita; se atingir o final, reseta
        void shift(){
            if(Out == 1){
                reset();
            } else {
                Out = Out >> 1;
            }
        }

        // Restaura a saída para o estado inicial (bit mais significativo = 1)
        void reset(){
            Out = 1u << (LimitReset - 1);
        }


        // retorna a saída atual (este métodos apenas acessa o valor out sem alterá-lo)
        unsigned int getOut() const{
            return Out;
        }
};


// responsável por simular um gerador de clock(pulsos) com o chip NE555 no modo ástavel
// para mais informações, consulte o datasheet do NE555
class Chip555{
    private:
        // configuração dos componentes eletrônicos -> 1 capacitor e 2 resistores 
        // há também a váriável output que começará como falso (valor baixo)
        bool output = false;
        double R1, R2, C;
        const double Ln2 = 0.693; // valor do logarítimo natural de 2


        // Métodos para calcular os valores em nível baixo (tLow) e nível alto (tHigh)
        double tHigh(){
            return Ln2*(R1 + R2)*C;
        }

        double tLow(){
            return Ln2*R2*C;
        }


        // método para alternar entre estado HIGH e LOW
        void clock(){
            output = !output;
        }

    public:
        Chip555(double r1, double r2, double c ) 
            : R1(r1), R2(r2), C(c) {};

        // Gera um pulso completo do NE555: alterna HIGH e LOW com os tempos calculados
        void pulse(){
            clock();
            std::this_thread::sleep_for(std::chrono::duration<double>(tHigh()));
            clock();
            std::this_thread::sleep_for(std::chrono::duration<double>(tLow()));
        }
};


// Simula a placa de aprendizagem Apple Juice
class runAppleJuice{
    private:
        // aplicando a classes dos chips aqui como objetos
        Chip4017& chip4017;
        Chip555&  chip555;
        unsigned int resetQP = 0;

    public:
        runAppleJuice (Chip555& c555, Chip4017& c4017, unsigned int resetQP)
        : chip4017(c4017), chip555(c555), resetQP(resetQP) {}

        // rodando o sistema infinitamente (colocado propositalmente: aperte ctrl z para interromper a simulação)
        void run(){
            while(true){
                Converter::toBinary(chip4017.getOut(), resetQP);  // mostra saída do 4017 em binário
                chip555.pulse();                                  // gera pulso do NE555
                chip4017.shift();                                 // desloca o bit do 4017
            }
        }
};


// função main configurado apenas para "orquestrar". O uso da tratativa de erros com "try" e "catch" foi aplicado aqui
int main(){ 
    try{
        // aplicando os devidos valores para cada objeto
        const int resetQP = 4;
        Chip4017 chip4017(resetQP);
        Chip555 chip555(1000, 10000, 7.37e-6);

        /*
            Aplicando os chips na placa Apple Juice para simulação
            e, logo em seguida, executando-os com "board.run()"
        */
        runAppleJuice board(chip555, chip4017, resetQP);
        board.run();

    } catch(const std::exception& e) {
        // indica erro e a sua causa caso haja algum problema na execução do código
        ErrorMsg::CriticalError(e.what());
    }

    return 0;
}