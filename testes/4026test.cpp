/*
    Aqui foram iniciadas os primeiros testes com o chip cd4026 antes de receber uma interface gráfica.
    Aqui foi apenas um exercício de lógica com o paradigma orientado a objetos sendo melhor formalizado.
    
    Foi criado por Francisco e serviu como base de estudo, para a sua implementação e formalização no 
    "projeto final".

    O projeto foi testando anteriormente em um repositório separado: https://github.com/FrankSteps/cd4026_decade_counter_simulator
*/

#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <chrono>


class Chip4026 {
    protected:
        bool carryOut = false;
        unsigned int Out = 0;

    public:
        virtual ~Chip4026() = default;

        virtual void add() {
            if (Out == 9) {
                Out = 0;
                carryOut = true;
            } else {
                Out++;
                carryOut = false;
            }
        }

        virtual void reset() {
            Out = 0;
            carryOut = false;
        }

        unsigned int getOut() const { 
            return Out; 
        }

        bool getCarryOut() const { 
            return carryOut; 
        }
};


class Unit : public Chip4026 {
    public:
        void add() override {
            Chip4026::add();
        }
};


class Tens : public Chip4026 {
    public:
        void addOnCarry(bool carryIn) {
            if (carryIn) {
                add(); 
            }
        }
};


int main() {
    Unit chip4026A;
    Tens chip4026B;
    std::string input;

    std::cout << "Press ENTER to external clock. Ctrl+C to exit.\n";

    while(true) {
        std::getline(std::cin, input);

        chip4026A.add();
        chip4026B.addOnCarry(chip4026A.getCarryOut());

        std::cout << "Tens: " << chip4026B.getOut() << " | Unit: " << chip4026A.getOut() << "\n";
    }

    return 0;
}