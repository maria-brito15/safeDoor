# Sistema de Controle de Porta Arduino (SafeDoor)

[English Version Here](README_en.md)

## Visão Geral

Este sistema implementa um controle automatizado de porta com recursos de alarme, detecção de visitantes e controle manual/automático. O sistema utiliza um sensor LDR para detectar luminosidade e controlar a abertura/fechamento da porta automaticamente, além de possuir um aplicativo móvel desenvolvido em App Inventor para controle remoto via Bluetooth.

## Anexos

### Tinkercad
- [Imagem da Montagem](https://ibb.co/sJ1s44V4)

### App Inventor
- [Código no MIT App Inventor](https://ibb.co/G4RyhND1)

## Componentes Utilizados

### Hardware

- **Arduino UNO** (controlador principal)
- **Servo Motor** (pino 9) - controla a abertura/fechamento da porta
- **Botão de Campainha** (pino 2) - detecta visitantes
- **Sensor LDR** (pino A2) - detecta luminosidade
- **Piezo Buzzer** (pino 8) - sinalizações sonoras
- **Módulo Bluetooth** - comunicação com aplicativo móvel
- **Acelerômetro** - detecção de movimento/vibração
- **Sensor de Proximidade** - detecção de visitantes por proximidade

### LEDs Indicadores

- **LED Verde da Porta** (pino 13)
- **LED Vermelho da Porta** (pino 12)
- **LED Verde do Alarme** (pino 11)
- **LED Vermelho do Alarme** (pino 10)

## Definições de Pinos

```c
#define botao_campainha 2
#define led_verde_porta 13
#define led_vermelho_porta 12
#define led_verde_alarme 11
#define led_vermelho_alarme 10
#define ldr A2
#define servo_motor 9
#define buzzer 8
```

## Aplicativo Mobile (App Inventor)

### Funcionalidades do Aplicativo

O aplicativo desenvolvido em App Inventor oferece controle completo do sistema SafeDoor através de comunicação Bluetooth:

#### 1. Interface de Controle Manual

- Switch para controle da porta
- Controle de modo automático através de um botão (toggle - liga/desliga)
- Indicadores visuais de status de porta, controle automático, alarmes e modo visitante

#### 2. Sistema de Reconhecimento de Voz

- Comando de voz para "trancar porta"
- Comando de voz para "destravar porta"
- Comando de voz para "ativar modo visitante"
- Integração com TextToSpeech para feedback sonoro (resposta)

#### 3. Chat Bot Integrado

- Interface de conversação para controle por comandos de texto
- Respostas automáticas do sistema

#### 4. Monitoramento de Sensores

- **Acelerômetro**: Detecta movimento/vibração para ativar alarme
- **Sensor de Proximidade**: Ao se aproximar da tela, ativa o modo para visitantes

## Lógica de Programação do App Inventor

### Inicialização do Sistema

```blocks
when Screen1.Initialize
do set global visitante to ""
set global respostaArd to ""
set global botao to ""
set global alarme to false
set global automatico to 1
```

### Controle por Reconhecimento de Voz (Porta)

```blocks
when SpeechRecognizer1.AfterGettingText
do if compare texts (get result) = "trancar porta"
then call BluetoothClient1.SendText
text "PF"
set porta_status to "TRANCADA"
else if compare texts (get result) = "destravar porta"
then call BluetoothClient1.SendText
text "PA"
set porta_status to "DESTRANCADA"
else if compare texts (get result) = "ativar modo visitante"
then call BluetoothClient1.SendText
text "VL"
```

### Controle de Botões (Controle Automático)

```blocks
when Button1.Click
do if get global automatico = 1
then call BluetoothClient1.SendText
text "G 1"
set global automatico to false
else call BluetoothClient1.SendText
text "G 0"
set global automatico to 0
```

### Acelerômetro (Alarme)

```blocks
when Acelerometro_Sensor.Shaking
do if get global alarme = false
then call BluetoothClient1.SendText
text "C"
set alarme_status to "ATIVADO"
if get global alarme = true
then call BluetoothClient1.SendText
text "D"
set alarme_status to "DESATIVADO"
```

### Detecção de Proximidade (Visitante)

```blocks
when Proximidade_Sensor.ProximityChanged
do if get distance >= MaximumRange
then call BluetoothClient1.SendText
text "E"
set visitanteMode_status to "ATIVADO"
else set visitanteMode_status to "DESATIVADO"
```

### Controle do Switch da Porta

```blocks
when porta_switch.Changed
do if porta_switch.On = true
then call BluetoothClient1.SendText
text "B"
set porta_status to "DESTRANCADA"
else call BluetoothClient1.SendText
text "A"
set porta_status to "TRANCADA"
```

### Comunicação Bluetooth

```blocks
when Clock1.Timer
do if call BluetoothClient1.BytesAvailableToReceive > 0
then set global respostaArd to call BluetoothClient1.ReceiveText
numberOfBytes: call BluetoothClient1.BytesAvailableToReceive
if get global respostaArd = "VD"
then call TextToSpeech1.Speak
message "visitante detectado"
set global respostaArd to ""
```

### Interface de Chat

```blocks
when ChatBot1.GotResponse
do set TextBox2 to get responseText
get responseText
```

### Controle por Slider

```blocks
when Slider1.PositionChanged
do call BluetoothClient1.SendText
text join "F" get thumbPosition
```

## Componentes do App Inventor Utilizados

### Interface de Usuário

- **TextBox1, TextBox2** - Entrada e saída de texto
- **Button1** - Controle do modo automático
- **porta_switch** - Switch para controle manual da porta
- **Slider1** - Controle de posição/frequência
- **ChatBot1** - Interface de chat para comandos

### Conectividade

- **BluetoothClient1** - Comunicação com Arduino
- **microfone** - Captura de áudio para reconhecimento de voz

### Sensores

- **Acelerometro_Sensor** - Detecção de movimento
- **Proximidade_Sensor** - Detecção de proximidade

### Controle de Tempo

- **Clock1** - Timer para polling da comunicação Bluetooth

## Variáveis Globais

### Controle de Estado (Arduino)

- **controle_automatico** - Flag para ativar/desativar controle automático
- **modo_visitante_ativo** - Flag do modo visitante
- **alarme_ativo** - Flag do estado do alarme
- **alarme_era_ativo_antes** - Backup do estado do alarme antes do modo visitante

### Variáveis Globais (App Inventor)

- **global visitante** - Status do modo visitante
- **global respostaArd** - Armazena resposta do Arduino
- **global botao** - Estado do botão
- **global alarme** - Estado do alarme
- **global automatico** - Estado do modo automático

### Controle de Tempo

- **tempo_inicio_modo_visitante** - Timestamp do início do modo visitante
- **duracao_modo_visitante** - Duração do modo visitante (10 segundos)

### Controle de Hardware

- **posicao_servo_atual** - Posição atual do servo motor
- **ultimo_status_porta** - Cache do último status da porta
- **limiarLDR** - Valor limiar para o sensor LDR (500)

## Funcionamento Principal

### Setup (Arduino)

1. Configura os pinos como entrada/saída
2. Configura interrupção para o botão da campainha
3. Inicializa comunicação serial (9600 baud)
4. Conecta o servo motor
5. Exibe mensagem de sistema iniciado

### Inicialização (App Inventor)

1. Inicializa variáveis globais
2. Configura conexão Bluetooth
3. Ativa sensores (acelerômetro, proximidade)
4. Inicia timer para monitoramento contínuo

### Loop Principal (Arduino)

O loop principal executa as seguintes tarefas:

1. **Processamento de Comandos Seriais**
   - Lê comandos via Serial/Bluetooth
   - Executa comandos baseados no primeiro caractere

2. **Detecção de Visitantes**
   - Verifica flag de interrupção do botão
   - Envia notificação "VD" via Serial/Bluetooth

3. **Controle do Modo Visitante**
   - Gerencia timeout do modo visitante (10 segundos)
   - Restaura estado anterior do alarme

4. **Controle Automático da Porta**
   - Lê sensor LDR quando controle automático está ativo
   - Abre/fecha porta baseado na luminosidade

5. **Monitoramento de Status**
   - Reporta mudanças no status da porta via Serial/Bluetooth

## Comandos Disponíveis

O sistema aceita comandos via comunicação serial/Bluetooth:

| Comando | Função | Parâmetro | Origem |
|---------|--------|-----------|--------|
| A | Fecha a porta manualmente | - | App/Serial |
| B | Abre a porta manualmente | - | App/Serial |
| C | Liga o alarme | - | App/Serial |
| D | Desliga o alarme | - | App/Serial |
| E | Ativa modo visitante | - | App/Serial |
| F | Controla frequência do buzzer | Frequência (ms) | App/Serial |
| G | Controla modo automático | 0 = desliga, 1 = liga | App/Serial |

## Funcionalidades Principais

### 1. Controle Automático da Porta

- **Sensor LDR**: Detecta luminosidade ambiente
- **Limiar**: 500 (configurável)
- **Lógica**:
  - Valor > 500: Porta aberta
  - Valor ≤ 500: Porta fechada

### 2. Modo Visitante

- **Duração**: 10 segundos
- **Ativação**: Botão físico, comando de voz, sensor de proximidade
- **Ações**:
  - Desativa alarme temporariamente
  - Abre a porta
  - Após timeout, restaura estado anterior

### 3. Sistema de Alarme

- **Estados**: Ligado/Desligado
- **Indicadores**: LEDs verde/vermelho
- **Ativação**: Acelerômetro (detecta movimento/vibração)
- **Integração**: Compatível com modo visitante

### 4. Detecção de Visitantes

- **Métodos**:
  - Interrupção por botão físico
  - Sensor de proximidade
  - Comando de voz
- **Resposta**: Notificação via Serial/Bluetooth ("VD")

### 5. Controle por Voz

- **Comandos suportados**:
  - "trancar porta"
  - "destravar porta"
  - "ativar modo visitante"
- **Feedback**: Síntese de voz confirmando ações

## Indicadores Visuais

### LEDs da Porta

- **Verde**: Porta aberta
- **Vermelho**: Porta fechada

### LEDs do Alarme

- **Verde**: Alarme ligado
- **Vermelho**: Alarme desligado

### Interface do App

- **Status da Porta**: Texto indicativo (TRANCADA/DESTRANCADA)
- **Status do Alarme**: Texto indicativo (ATIVADO/DESATIVADO)
- **Modo Visitante**: Indicação visual quando ativo

## Comunicação Serial/Bluetooth

### Mensagens de Status

- **"PF"** - Porta fechada
- **"PA"** - Porta aberta
- **"AL"** - Alarme ligado
- **"AD"** - Alarme desligado
- **"VL"** - Modo visitante ligado
- **"MVD"** - Modo visitante desligado
- **"VD"** - Visitante detectado

### Configuração

- **Baud Rate**: 9600
- **Formato**: Texto simples com quebras de linha
- **Protocolo**: Bluetooth Serial Profile (SPP)

## Funções Principais

### Arduino

- **portaFechada()**: Fecha a porta e atualiza indicadores
- **portaAberta()**: Abre a porta e atualiza indicadores
- **alarmeLigado()/alarmeDesligado()**: Controla estado do alarme
- **modoVisitanteLigado()/modoVisitanteDesligado()**: Gerencia modo visitante
- **botaoInterrupt()**: Função de interrupção para campainha
- **frequenciaBuzzer()**: Controla buzzer com frequência especificada

### App Inventor

- **Controle de Voz**: Processamento de comandos falados
- **Monitoramento de Sensores**: Leitura contínua de acelerômetro e proximidade
- **Interface de Chat**: Processamento de comandos de texto
- **Comunicação Bluetooth**: Envio/recebimento de dados

## Características Técnicas

### Timing

- **Loop delay**: 500ms
- **Servo delay**: 500ms após movimento
- **Modo visitante**: 10 segundos
- **Polling Bluetooth**: Contínuo via Clock1

### Limites

- **Posições servo**: 0° (fechado) e 90° (aberto)
- **Limiar LDR**: 500 (ajustável)
- **Baud rate**: 9600
- **Alcance Bluetooth**: ~10 metros

## Integrantes

- Maria Eduarda B.
- Gabriella A.
- Daniel R.
- Rafael T.
