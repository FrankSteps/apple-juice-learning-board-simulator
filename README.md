# Apple Juice learning board simulator

![Placa Apple Juice](images/apple-juice.png)

O simulador com interface gráfica da placa de aprendizagem Apple Juice foi desenvolvido para o laboratório da FnEsc, no Departamento de Física da UFS. O sistema foi implementado, principalmente, utilizando o paradigma de programação orientada a objetos, com algumas funcionalidades de caráter procedural.

A Apple Juice é uma placa de aprendizagem voltada ao estudo de circuitos digitais construídos com circuitos integrados discretos, como o CD4017, NE555 e CD4026. O sistema oferece suporte à entrada de clock externo, permitindo incrementar a contagem na parte do circuito responsável pela decodificação binária para decimal (a seção que utiliza o CD4026). Além disso, possui um botão de reset para os circuitos CD4017 e um switch para alternar entre o clock externo e o clock interno gerado pelo 555 configurado em modo astável. 

Leia sobre os componentes que estão presentes no projeto clicando [aqui](./documentacao/appleJuice.pdf).

O objetivo deste projeto foi aplicar os conceitos de programação orientada a objetos apresentados em sala de aula na disciplina de POO, sob orientação do professor Carlos Estombelo. Conceitos como tratamento de exceções com try e catch, encapsulamento, herança e polimorfismo foram utilizados ao longo do desenvolvimento do sistema. 

A documentação completa escrita pelo professor passando as orientações está disponível [aqui](./paraDisciplina/orientacao.pdf).


## Interface gráfica do Apple Juice

<p align="center">
  <a href="https://youtu.be/SVXHdgML4G8">
    <img src="https://img.youtube.com/vi/SVXHdgML4G8/maxresdefault.jpg" 
         alt="Assista ao vídeo de apresentação do projeto">
  </a>
</p>

<p align="center">
    Clique para assistir ao vídeo no YouTube
</p>


## Features 
Simulação do CI NE555 em modo astável
<br>
Simulação do CD4017 (contador Johnson)
<br>
Decodificação com CD4026
<br>
Interface gráfica utilizando raylib
<br>


## Estrutura do projeto
```
Apple-juice-learning-board-simulator/
├── documentacao                    # Documentação escrita por Francisco do projeto
│   ├── appleJuice.pdf                
│   └── appleJuice.tex
├── images                          # Imagens utilizadas no README
│   ├── apple-juice-simulator.png 
│   └── apple-juice.png 
├── paraDisciplina                  # Materiais complementares da disciplina
│   ├── orientacao.pdf
│   └── roteiro.pdf
├── testes                          # Testes unitários e experimentais
│   ├── 4026test.cpp                # teste de Francisco
│   ├── clockGenerator-test.cpp     # teste de Francisco
│   ├── teste-appleJuice.cpp        # teste de Francisco
│   ├── teste.cpp                   # teste de Renato
│   └── testes.cpp                  # teste de Arthur 
│
├── apple-juice.cpp                 # Arquivo principal do simulador
├── CONTRIBUTING.md                 # Diretrizes para contribuição no projeto
├── LICENSE                         # Licença do projeto (GNU GPLv3)
├── Makefile                        # Script de compilação e execução
├── README.md                       # Documentação principal do repositório
└── shell.nix                       # Arquivo de configuração para compilar em distros nixOS
```

## Pré-requisitos
- g++ (com suporte a C++17)
- cmake 
- raylib
- pkg-config

> **NixOS:** use `nix-shell` antes de compilar. O arquivo `shell.nix` já está incluído no repositório.


## Como compilar e rodar

```bash
# clone o repositório
git clone https://github.com/FrankSteps/Apple-juice-learning-board-simulator
cd Apple-juice-learning-board-simulator

# compile o simulador
make

# compile e rode o simulador
make run

# compile e rode os testes unitários
make test

# remova os binários gerados
make clean
```

## Compatibilidade
Este projeto é compatível com Linux, Windows e macOS.

## Licença
Este projeto está licenciado sob a GNU GPLv3. Veja o arquivo LICENSE para mais detalhes.

## Colaboradores
| [<img src="https://avatars.githubusercontent.com/u/177877856?v=4" width="115"><br><sub>@franksteps</sub>](https://github.com/franksteps) | [<img src="https://avatars.githubusercontent.com/u/186333867?v=4" width="115"><br><sub>@4rth-gs</sub>](https://github.com/4rth-g) | [<img src="https://avatars.githubusercontent.com/u/190228986?v=4" width="115"><br><sub>@RenatoVPF</sub>](https://github.com/RenatoVPF) | [<img src="https://avatars.githubusercontent.com/u/186655848?v=4" width="115"><br><sub>@Cadu-ux</sub>](https://github.com/Cadu-ux) | [<img src="https://avatars.githubusercontent.com/u/161770679?v=4" width="115"><br><sub>@matheusmatos4</sub>](https://github.com/matheusmatos4) |
| :---: | :---: | :---: | :---: | :---: |
