/*
    O simulador com interface gráfica da placa de aprendizagem, a Apple Juice, foi desenvolvido para o laboratório da FnEsc, no Departamento de Física da UFS. 
    O sistema foi implementado principalmente utilizando o paradigma de programação orientada a objetos, com algumas funcionalidades de caráter procedural.

    A Apple Juice é uma placa de aprendizagem voltada ao estudo de circuitos digitais, construídos com circuitos integrados discretos, como o CD4017, NE555 
    e CD4026. O sistema oferece suporte à entrada de clock externo, permitindo incrementar a contagem na parte do circuito responsável pela decodificação 
    binária para decimal (a parte que possui os CD4026). Além disso, possui um botão de reset para os circuitos CD4017 e um switch para alternar entre o clock 
    externo e o clock interno gerado pelo 555 configurado em modo astável.

    O objetivo deste projeto foi aplicar os conceitos de programação orientada a objetos apresentados em sala de aula na disciplina de POO, sob orientação do 
    professor Carlos Estombelo. Conceitos como tratamento de exceções com try e catch, encapsulamento, herança e polimorfismo foram implementados e utilizados 
    ao longo do desenvolvimento deste código. 

    Responsáveis pelo desenvolvimento deste projeto: Francisco, Renato, Arthur, Matos e Carlos Eduardo 


    Notas do(s) desenvolvedor(es):
        *  Optei por não utilizar a diretiva using namespace para garantir uma apresentação mais clara do código. Evitar o uso de namespace reforça que determinada 
        parte do código foi retirada de uma biblioteca específica. Essa mesma motivação me levou a encapsular a biblioteca raylib dentro do meu próprio namespace
        ray, conforme documentado mais adiante.

        — Ass.: Francisco
 
        *  ...
*/

// bibliotecas utilizadas no projeto
#include <iostream>               // Entrada e saída padrão (cout, cerr, etc.)
#include <string>                 // Manipulação de strings (std::string, std::to_string, etc.)
#include <cmath>                  // Funções matemáticas (pow, sin, etc.)
#include <cstdint>                // Tipos inteiros com tamanho fixo (uint32_t, int64_t, etc.)
#include <mutex>                  // Controle de exclusão mútua para threads (std::mutex, std::lock_guard)
#include <atomic>                 // Variáveis atômicas para comunicação segura entre threads (std::atomic)
#include <stdexcept>              // Exceções padrão (std::invalid_argument)
#include <thread>                 // Threads do C++ (std::thread)
#include <chrono>                 // Controle de tempo e delays (std::chrono::duration, sleep_for)
#include <cstdlib>                // Funções utilitárias gerais da biblioteca C (std::exit, std::rand, std::abs, etc.)


/*
    Biblioteca responsável pela interface gráfica:

    Coloquei esta biblioteca dentro do namespace 'ray' para diferenciá-la de outras bibliotecas.
    Como ela foi escrita em C, não possui namespace nativamente, ao contrário de bibliotecas C++ como 'iostream', 
    que ficam dentro do 'std'.
*/
namespace ray{
    #include <raylib.h>
}


/*
    Classe base que simula o funcionamento de um display decodificador CD4026, responsável por incrementar a contagem 
    de 0 a 9 e gerar um sinal de carry quando a contagem reinicia (Out volta a 0). 

    A aplicação do modificador 'virtual' permite o polimorfismo, possibilitando que métodos da classe base sejam 
    sobrescritos pelas classes derivadas.
*/
class Chip4026 {
protected:
    bool carryOut = false;      // Indica se houve estouro da contagem (Out voltou a 0)
    unsigned int Out = 0;       // Valor atual do display (0 a 9)

public:
    virtual ~Chip4026() = default;

    // Incrementa a contagem. Se atingir 9, reinicia e ativa carryOut
    virtual void add() {
        if (Out == 9) {
            Out = 0;
            carryOut = true;
        } else {
            Out++;
            carryOut = false;
        }
    }

    // Reseta o display e desativa o carry
    virtual void reset() {
        Out = 0;
        carryOut = false;
    }

    // Retorna o valor atual do display
    unsigned int getOut() const { 
        return Out; 
    }

    // Retorna se houve carry na última contagem
    bool getCarryOut() const { 
        return carryOut; 
    }
};



/*
    Classe que representa o display das unidades.
    Herda Chip4026 e mantém comportamento padrão da contagem de 0 a 9.
*/
class Unidade : public Chip4026 {
public:
    // O override indica que este método sobrescreve uma função virtual da classe base, assim o polimorfismo funciona em tempo de execução
    void add() override {
        Chip4026::add();
    }
};



/*
    Classe que representa o display das dezenas.
    Herda Chip4026 e adiciona a funcionalidade de incrementar apenas quando recebe um carry da unidade anterior.
*/
class Dezena : public Chip4026 {
public:
    void addOnCarry(bool carryIn) {
        if (carryIn) {
            add(); 
        }
    }
};

/*  
    ------------------------------------------------------------------------------------
    ------------------------------------------------------------------------------------
    |BLOCOS RESPONSÁVEL PELA CONSTRUÇÃO DE UM ÚNICO DÍGITO DE UM DISPLAY DE 7 SEGMENTOS| 
    ------------------------------------------------------------------------------------
    ------------------------------------------------------------------------------------
*/

// Responsável por desenhar um segmento individual (retângulo)
static void DrawSegment(ray::Vector2 pos, float width, float height, bool on, ray::Color color) {
    ray::Color c = on ? color : ray::Fade(ray::BLACK, 0.15f);
    ray::DrawRectangleV(pos, (ray::Vector2){width, height}, c);
}



/*
    static_cast<size_t>(Segment::A) converte o valor do enum class Segment para um índice válido de array.
    O array seg[] armazena quais segmentos do display estão ligados (true) ou desligados (false).

    Essa abordagem é mais segura e legível, garantindo que cada segmento seja referenciado corretamente pelo seu nome.
    Os segmentos são listados em seus datasheets por ordem alfabética, portanto, usar enum class garante melhor desenvolvimento.
*/


// Criei um enum class para que a busca pelos segmentos fosse mais intuitiva
// ID de cada segmento: a = 0; b = 1; c = 2; d = 3; e = 4; f = 5; g = 6;
enum class segments{
    a, b, c, d, e, f, g 
};


// Desenha um display de 7 segmentos com base no valor (0-9)
static void DrawSevenSegment(ray::Vector2 pos, float size, unsigned int value, ray::Color color) {
    float w = size * 0.2f;
    float h = size * 0.05f;
    float gap = size * 0.02f;

    bool seg[7] = {false}; // definindo o valor de todos os elementos para "false"
    switch(value) {
        case 0: 
            seg[static_cast<size_t>(segments::a)] = true;
            seg[static_cast<size_t>(segments::b)] = true;
            seg[static_cast<size_t>(segments::c)] = true;
            seg[static_cast<size_t>(segments::d)] = true;
            seg[static_cast<size_t>(segments::e)] = true;
            seg[static_cast<size_t>(segments::f)] = true;
            break;

        case 1: 
            seg[static_cast<size_t>(segments::b)] = true;
            seg[static_cast<size_t>(segments::c)] = true;
            break;

        case 2: 
            seg[static_cast<size_t>(segments::a)] = true;
            seg[static_cast<size_t>(segments::b)] = true;
            seg[static_cast<size_t>(segments::d)] = true;
            seg[static_cast<size_t>(segments::e)] = true;
            seg[static_cast<size_t>(segments::g)] = true;
            break;

        case 3: 
            seg[static_cast<size_t>(segments::a)] = true;
            seg[static_cast<size_t>(segments::b)] = true;
            seg[static_cast<size_t>(segments::c)] = true;
            seg[static_cast<size_t>(segments::d)] = true;
            seg[static_cast<size_t>(segments::g)] = true;
            break;

        case 4: 
            seg[static_cast<size_t>(segments::b)] = true;
            seg[static_cast<size_t>(segments::c)] = true;
            seg[static_cast<size_t>(segments::f)] = true;
            seg[static_cast<size_t>(segments::g)] = true; 
            break;

        case 5: 
            seg[static_cast<size_t>(segments::a)] = true;
            seg[static_cast<size_t>(segments::c)] = true;
            seg[static_cast<size_t>(segments::d)] = true;
            seg[static_cast<size_t>(segments::f)] = true;
            seg[static_cast<size_t>(segments::g)] = true;
            break;

        case 6: 
            seg[static_cast<size_t>(segments::a)] = true;
            seg[static_cast<size_t>(segments::c)] = true;
            seg[static_cast<size_t>(segments::d)] = true;
            seg[static_cast<size_t>(segments::e)] = true;
            seg[static_cast<size_t>(segments::f)] = true;
            seg[static_cast<size_t>(segments::g)] = true;
            
            break;

        case 7: 
            seg[static_cast<size_t>(segments::a)] = true;
            seg[static_cast<size_t>(segments::b)] = true;
            seg[static_cast<size_t>(segments::c)] = true;
            break;

        case 8: 
            seg[static_cast<size_t>(segments::a)] = true;
            seg[static_cast<size_t>(segments::b)] = true; 
            seg[static_cast<size_t>(segments::c)] = true;
            seg[static_cast<size_t>(segments::d)] = true; 
            seg[static_cast<size_t>(segments::e)] = true;
            seg[static_cast<size_t>(segments::f)] = true; 
            seg[static_cast<size_t>(segments::g)] = true;
            break;

        case 9:  
            seg[static_cast<size_t>(segments::a)] = true;
            seg[static_cast<size_t>(segments::b)] = true; 
            seg[static_cast<size_t>(segments::c)] = true;
            seg[static_cast<size_t>(segments::d)] = true; 
            seg[static_cast<size_t>(segments::f)] = true; 
            seg[static_cast<size_t>(segments::g)] = true;
            break;
    }

    // vetor que armazena as posições dos segmentos
    ray::Vector2 positions[7] = {
        {pos.x + w + gap, pos.y},                        // A
        {pos.x + size - h, pos.y + w + gap},             // B
        {pos.x + size - h, pos.y + size - w - gap},      // C
        {pos.x + w + gap, pos.y + size - h + 35},        // D
        {pos.x, pos.y + size - w - gap},                 // E
        {pos.x, pos.y + w + gap},                        // F
        {pos.x + w + gap, pos.y + size/2 - h/2 + 20}     // G
    };

    // vetor que armazena as dimensões dos segmentos
    ray::Vector2 dims[7] = {
        {size - 2*w - 2*gap, h},                         // A
        {h, size/2 - w - gap},                           // B
        {h, size/2 - w - gap},                           // C
        {size - 2*w - 2*gap, h},                         // D
        {h, size/2 - w - gap},                           // E
        {h, size/2 - w - gap},                           // F
        {size - 2*w - 2*gap, h}                          // G
    };

    
    for(int i=0; i<7; i++) {
        DrawSegment(positions[i], dims[i].x, dims[i].y, seg[i], color);
    }
}


/*
    Simulação do Chip555 configurado em modo astável
    Consulte a seção de Astable Mode (Free‑Running) no datasheet do NE555 / LM555 aproximadamente nas páginas 7–8, onde são apresentadas as
    fórmulas e explicações para tHigh, tLow, período e frequência da oscilação.
*/
class Chip555 {
private:
    double R1, R2, C;
    double tHigh = 0.0;
    double tLow  = 0.0;
    double period = 0.0;
    double freq   = 0.0;

    // logarítmo natural de 2
    const double Ln2    = 0.693;
    std::atomic<bool> stateHigh{false};

    void calcTimings() {
        // Modo astável (aprox): tH = 0.693*(R1+R2)*C; tL = 0.693*R2*C
        tHigh = Ln2 * (R1 + R2) * C;
        tLow  = Ln2 * (R2) * C;
        period = tHigh + tLow;
        freq = (period > 0) ? (1.0 / period) : 0.0;
    }

public:
    /*
        Esse construtor cria um objeto Chip555, inicializa seus parâmetros R1, R2 e C, verifica 
        se eles são válidos, e calcula os tempos de pulso e frequência para o sinal astável.
    */
    Chip555(double r1Ohms, double r2Ohms, double cFarads)
        : R1(r1Ohms), R2(r2Ohms), C(cFarads) {
        if (R1 <= 0 || R2 <= 0 || C <= 0) {
            throw std::invalid_argument("R1, R2 e C precisam ser > 0");
        }
        calcTimings();
    }

    // Simula um ciclo de clock (HIGH e LOW) com delays
    void pulse() {
        stateHigh = true;
        std::this_thread::sleep_for(std::chrono::duration<double>(tHigh));

        stateHigh = false;
        std::this_thread::sleep_for(std::chrono::duration<double>(tLow));
    }

    // estes métodos apenas acessam os valores sem alterá-los (const foi usado aqui como uma aplicação de segurança)
    bool isHigh() const { 
        return stateHigh; 
    }

    double getFrequency() const { 
        return freq; 
    }

    double getPeriod() const { 
        return period; 
    }
};


// Chip4017 (contador johnsson)
// Consulte o datasheet do CD4017 para informações mais detalhadas a respeito de seu funcionamento.
class Chip4017 {
private:
    unsigned LimitReset;
    uint32_t Out{0};

public:
    explicit Chip4017(unsigned limitReset)
        : LimitReset(limitReset) {
        if (LimitReset < 1 || LimitReset > 10) {
            throw std::invalid_argument("LimitReset precisa estar entre 1 e 10.");
        }
        reset();
    }

    // método que reage ao pulso do clock deslocando o bit mais significativo para a direita
    void shift() {
        Out >>= 1;
        if (Out == 0) {
            Out = 1u << (LimitReset - 1);
        }
    }

    // método para aplicar o reset no chips
    void reset() {
        Out = 1u << (LimitReset - 1);
    }

    // apenas retornam - não podem alterar o valor
    uint32_t getOut() const { 
        return Out;
    }

    unsigned getLimitReset() const { 
        return LimitReset;                          
    }
};


/*
    Parte do código responsável pela simulação da formação do efeito de luminosidade dos leds para deixa-los mais realistas
*/
static void DrawLedGlow(ray::Vector2 center, float radius, ray::Color core, ray::Color glow) {
    // Ajustes finos do glow
    const int rings = 18;        
    const float step = 1.5f;     
    const float gamma = 1.5f;    

    for (int i = rings; i >= 1; --i) {
        float t = (float)i / (float)rings;      
        float r = radius + i * step;

        // Curva de queda do brilho: mais suave perto do LED e decai no fim 
        float falloff = powf(t, gamma);         
        unsigned char a = (unsigned char)(glow.a * falloff * 0.2f);

        ray::Color c = glow;
        c.a = a;
        DrawCircleV(center, r, c);
    }

    // Núcleo do LED
    ray::DrawCircleV(center, radius, core);

    // “brilho interno” para parecer LED real
    ray::Color hi = core;
    hi.a = 90;
    ray::DrawCircleV((ray::Vector2){ center.x - radius * 0.25f, center.y - radius * 0.25f }, radius * 0.45f, hi);

    // Borda
    ray::DrawCircleLines((int)center.x, (int)center.y, radius, ray::Fade(ray::BLACK, 0.30f));
}


// Desenha a placa: fundo escuro, retângulo arredondado e linhas verticais como textura
static void DrawPanel(ray::Rectangle rec) {
    // fundo geral
    ray::ClearBackground((ray::Color){ 18, 20, 24, 255 });

    // “placa”
    ray::DrawRectangleRounded(rec, 0.15f, 16, (ray::Color){ 30, 34, 42, 255 });
    ray::DrawRectangleRoundedLinesEx(rec, 0.15f, 16, 2.0f, ray::Fade(ray::RAYWHITE, 0.10f));

    // textura leve (linhas)
    for (int x = (int)rec.x + 10; x < (int)(rec.x + rec.width); x += 22) {
        ray::DrawLine(x, (int)rec.y + 8, x, (int)(rec.y + rec.height) - 8, ray::Fade(ray::RAYWHITE, 0.02f));
    }
}


/*
    Esta classe é responsável pela parte principal do simulador: Ele simula o conjunto de todos os circuitos integrados em uma única classe
    Também é responsável pela interface gráfica
*/
class BoardAppleJuice {
private:
    unsigned qtLeds;
    double R1, R2, C;

public:
    BoardAppleJuice(unsigned leds, double r1, double r2, double c)
        : qtLeds(leds), R1(r1), R2(r2), C(c) {}

    void run() {
        // criando a janela do simulador e limitando em 60 FPS
        ray::InitWindow(1200, 700, "Simulador do Apple Juice");
        ray::SetTargetFPS(60); 

        // criando os objetos e passando os parâmetros para os construtores
        Chip4017 chip4017(qtLeds);
        Chip555  chip555(R1, R2, C);

        // criando o objeto de cada chip 4026: Um responsável por mostrar as dezenas e outra responsável por mostrar as unidades
        Unidade unidade;
        Dezena dezena;


        /*
            ----------------------------------------------------------------------------------------------
            ----------------------------------------------------------------------------------------------
            |ESSA É PARTE PRINCIPAL DO PROJETO: É AQUI AONDE TUDO COMEÇA A ACONTECER NA PLACA APPLE JUICE|
            ----------------------------------------------------------------------------------------------
            ----------------------------------------------------------------------------------------------
        */

        // Variáveis atômicas para controlar o estado do simulador (running e ligado)
        // e mutex para proteger o acesso aos chips compartilhados entre threads
        std::atomic<bool> running{true};
        std::atomic<bool> ligado{false};

        std::mutex mtx; 

        // Thread responsável pelo pulso do 555 e atualização do CD4017
        std::thread motor([&]{
            while (running.load()) {
                if (!ligado.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(30));
                    continue;
                }
                // Clock interno do 555 (modo astável)
                chip555.pulse(); // alterna entre HIGH/LOW internamente

                // Atualiza os chips e displays a cada pulso
                std::lock_guard<std::mutex> lock(mtx);
                chip4017.shift();
                unidade.add();
                dezena.addOnCarry(unidade.getCarryOut());
            }
        });



        // colocando a condição "&&" junto ao running.load(), foi possível resolver o problema do loop infinito do programa que impedia o mesmo de ser fechado adequadamente
        while (running.load() && !ray::WindowShouldClose()) {

            // condicionais responsáveis pelo controle do simulador
            if (ray::IsKeyPressed(ray::KEY_ENTER)) {
                ligado.store(!ligado.load());
            }

            if (ray::IsKeyPressed(ray::KEY_R)) {
                std::lock_guard<std::mutex> lock(mtx);
                chip4017.reset();
                unidade.reset();
                dezena.reset();
            }

            if (ray::IsKeyPressed(ray::KEY_ZERO)) {
                running.store(false);
                break;
            }

            uint32_t bits = 0;
            {
                std::lock_guard<std::mutex> lock(mtx);
                bits = chip4017.getOut();
            }


            // Cor padrão para os botões
            ray::Color btnColor = ray::LIGHTGRAY;

            // Botão de reset dos displays
            ray::Rectangle btnReset = { 400, 450, 140, 40 };

            // Responsável por identificar se o botão esquerdo do mouse foi pressionado
            if (ray::IsMouseButtonPressed(ray::MOUSE_LEFT_BUTTON)) {
                ray::Vector2 mouse = ray::GetMousePosition();

                if (mouse.x >= btnReset.x && mouse.x <= btnReset.x + btnReset.width &&
                    mouse.y >= btnReset.y && mouse.y <= btnReset.y + btnReset.height) {
                    std::lock_guard<std::mutex> lock(mtx);
                    unidade.reset();
                    dezena.reset();
                }
            }

            // renderizando as imagens na tela:
            ray::BeginDrawing();
                ray::Rectangle panel = { 60, 80, 1080, 320 };
                DrawPanel(panel);


                // Status para feedback do usuário 
                const char* status = ligado.load() ? "LIGADO" : "DESLIGADO";
                ray::DrawText(
                    ray::TextFormat("Status: %s  |  ENTER liga/desliga  |  R reset all", status),
                    40, 20, 18, ray::Fade(ray::RAYWHITE, 0.85f)
                );

                ray::Color clk = chip555.isHigh() ? (ray::Color){ 50, 220, 130, 255 } : (ray::Color){ 200, 60, 60, 255 };
                ray::DrawCircle(830, 30, 7, clk);
                ray::DrawText("CLK", 845, 24, 16, ray::Fade(ray::RAYWHITE, 0.70f));

                ray::DrawText(
                    ray::TextFormat("555: f=%.2f Hz | T=%.3f s", chip555.getFrequency(), chip555.getPeriod()),
                    60, 80, 18, ray::Fade(ray::RAYWHITE, 0.55f)
                );


                // LEDs existentes
                float baseY = 280.0f;
                float radius = 32.0f;
                float margem = 120.0f;
                float areaUtil = 1200.0f - 2 * margem;
                float gap = areaUtil / (qtLeds - 1);
                float startX = margem;

                float t = (float)ray::GetTime();
                float breathe = 0.5f + 0.5f * sinf(t * 3.2f);

                /*
                    Loop que percorre todos os LEDs, desenhando cada um com brilho se estiver aceso e exibindo 
                    seu número correspondente abaixo.
                */
                for (int i = (int)qtLeds - 1; i >= 0; --i) {
                    bool on = ((bits >> i) & 1u) != 0;
                    int idx = (int)qtLeds - 1 - i;
                    ray::Vector2 c = { startX + idx * gap, baseY };

                    ray::Color offCore = (ray::Color){ 120, 125, 135, 255 };
                    ray::Color offGlow = (ray::Color){ 120, 125, 135, 0 };

                    unsigned char aGlow = (unsigned char)(70 + 90 * breathe);
                    ray::Color onCore = (ray::Color){ 70, 255, 130, 220 };
                    ray::Color onGlow = (ray::Color){ 70, 255, 130, aGlow };

                    if(on) {
                        DrawLedGlow(c, radius, onCore, onGlow);
                    } else {
                        DrawLedGlow(c, radius, offCore, offGlow);
                    } 

                    ray::DrawText(
                        ray::TextFormat("L%d", idx + 1),
                        (int)(c.x - 14),
                        (int)(c.y + 52),
                        18,
                        ray::Fade(ray::RAYWHITE, 0.60f)
                    );
                }

                // Displays de 7 segmentos
                ray::Vector2 posUnidade = { 950-740, 450 };
                ray::Vector2 posDezena  = { 800-740, 450 };
                float displaySize = 120.0f;

                DrawSevenSegment(posDezena, displaySize, dezena.getOut(), (ray::Color){70, 255, 130, 255});
                DrawSevenSegment(posUnidade, displaySize, unidade.getOut(), (ray::Color){70, 255, 130, 255});

                // Botão de reset 
                ray::DrawRectangleRec(btnReset, btnColor);
                ray::DrawText("Reset Display", (int)(btnReset.x + 5), (int)(btnReset.y + 5), 18, ray::BLACK);

                // Mensagem de apoio
                ray::DrawText("Dica: aumente C (ex.: 47uF) para ficar mais lento; diminua C (ex.: 10uF) para acelerar.", 60, 360, 16, ray::Fade(ray::RAYWHITE, 0.45f));
            ray::EndDrawing();
        }

        // Para a thread definindo 'running' como falso e aguarda sua finalização com join se ainda estiver ativa.
        running.store(false);
        if(motor.joinable()) {
            motor.join();
        }
        ray::CloseWindow();
    }
};


// Função main: cria e executa o simulador Apple Juice
// Uso do try e catch são ótimos para debug
int main() {
    try {
        // Adicione os valores para simulação aqui:
        unsigned leds = 4;      // Número total de LEDs para o 4017
        double R1 = 1000.0;     // Resistor R1 do 555
        double R2 = 10000.0;    // Resistor R2 do 555
        double C  = 7.37e-6;    // Capacitor do 555

        // Cria o simulador e o executa
        BoardAppleJuice appleJuice(leds, R1, R2, C); 
        appleJuice.run();                             
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Erro nos parâmetros do simulador: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& e) {
        std::cerr << "Erro inesperado: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Erro desconhecido ocorreu!" << std::endl;
        return EXIT_FAILURE;
    }
    // Programa finalizado com sucesso
    return EXIT_SUCCESS;
}