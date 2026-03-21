/*
    Aqui é a formalizalção e aplicação um pouco mais robusta do POO no projeto teste-appleJuice.cpp
    Este é o arquivo final que foi disponibilizado no repositório: https://github.com/FrankSteps/DigitalCounter555_Simulator
*/


#include <iostream>
#include <string>
#include <stdlib.h>
#include <chrono>
#include <thread>

struct ErrorMsg{
    [[noreturn]] static void CriticalError(const std::string& msg){
        std::cerr << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
};


struct Converter{
    static void toBinary(unsigned int decimal, int bits){
        for(int i = bits-1; i >= 0; i--){
            std::cout << ((decimal >> i) & 1);
        }
        std::cout << std::endl;
    }
};


class Chip4017{
    private:
        unsigned int Out = 1;
        unsigned int LimitReset;

    public:
        explicit Chip4017(unsigned int limitReset) : LimitReset(limitReset) {
            if(limitReset == 0 || limitReset > 10){
                throw std::invalid_argument("Reset pin must be between 1 and 10");
            }
        };

        void shift(){
            if(Out >= (1u << (LimitReset-1))){
                reset();
            } else {
                Out = Out << 1;
            }
        }

        void reset(){
            Out = 1;
        }

        unsigned int getOut() const{
            return Out;
        }
};


class Chip555{
    private:
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
            : R1(r1), R2(r2), C(c) {};

        void pulse(){
            clock();
            std::this_thread::sleep_for(std::chrono::duration<double>(tHigh()));
            clock();
            std::this_thread::sleep_for(std::chrono::duration<double>(tLow()));
        }
};


class runBoard{
    private:
        Chip4017& chip4017;
        Chip555&  chip555;
        unsigned int resetQP = 0;

    public:
        runBoard (Chip555& c555, Chip4017& c4017, unsigned int resetQP)
        : chip4017(c4017), chip555(c555), resetQP(resetQP) {}

        void run(){
            while(true){
                Converter::toBinary(chip4017.getOut(), resetQP);
                chip555.pulse();
                chip4017.shift(); 
            }
        }
};


int main(){ 
    try{
        const int resetQP = 4;

        Chip4017 chip4017(resetQP);
        Chip555 chip555(1000, 10000, 7.37e-6);

        runBoard board(chip555, chip4017, resetQP);
        board.run();

    } catch(const std::exception& e) {
        ErrorMsg::CriticalError(e.what());
    }

    return 0;
}