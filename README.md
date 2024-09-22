# Setup

Para a compilação e do programa é necessario que o cmake e make estejam instalados

```
# Para conexao por meio de ssh
git clone git@github.com:rhavyyz/SO_assignment.git

# Para conexao por meio de HTTPS
git clone https://github.com/rhavyyz/SO_assignment.git

mkdir build 
cd build

cmake ..

# Para compilação padrão
make
```


# Executando Server

Para incializar o servidor é necessario um json com as configurações e questões. Esse json segue o seguinte exemplo do arquivo `questions.json`

```
{
    "max_players": 20,
    "time_for_question": 10,
    "questions":[
        {
            "text": "how many days a regular year has?",
            "alternatives" : [
                "365",
                "200",
                "999",
                "777",
                "32",
                "3"
            ],
            "awnser_index": 0
        },
        {
            "text": "the humanity reached the moon?",
            "alternatives" : [
                "No",
                "Yes"
            ],
            "awnser_index": 1
        }
    ]
}
```

- em ”max players” deve armazenar um inteiro indicando o numero maximo de
usuarios que devem participar do quiz
- em ”time f or question” deve armazenar um inteiro indicando o tempo em se-
gundos que deve ser esperado entre cada pergunta.
- Ja no vetor de ”questions” devem haver objetos que representam as questoes.
- Cada quest˜ao deve ter um campo ”text” que armazena uma string sendo o enun-
ciado, um vetor ”alternatives”, contendo uma string para cada uma das alterna-
tivas, e um campo ”awnser index”, que guarda um inteiro, indicando qual das
alternativas esta correta (considerando o vetor 0 indexados).

Tendo esse json de configurações, basta executar o seguinte na pasta de build

```
./server path_to_json_file
```

# Executando Client

Para executar um client é necessario que um server esteja disponivel. Daí, basta executar o seguinte comando na pasta de build

```

# caso queira responder as perguntas 
./client codigo_do_servidor

# caso queira que o client simule o comportamente de um usuario 
# e responda sozinho
./client codigo_do_servidor -s
```