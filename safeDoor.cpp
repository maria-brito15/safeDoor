// ====================================================================
// SISTEMA DE PORTA AUTOMÁTICA - ARDUINO
// Sistema inteligente para controle de acesso com detecção automática
// de luz, alarme integrado e modo visitante temporário
// ====================================================================

#include <Servo.h>

// ====================================================================
// DEFINIÇÕES DOS PINOS DOS COMPONENTES
// ====================================================================
#define botao_campainha 2        // Pino do botão da campainha (entrada digital)

// Pinos dos LEDs indicadores da porta
#define led_verde_porta 13       // LED verde - indica porta aberta
#define led_vermelho_porta 12    // LED vermelho - indica porta fechada

// Pinos dos LEDs indicadores do alarme
#define led_verde_alarme 11      // LED verde - indica alarme ligado
#define led_vermelho_alarme 10   // LED vermelho - indica alarme desligado

// Pinos dos sensores e atuadores
#define ldr A2                   // Sensor de luz (LDR) no pino analógico A2
#define servo_motor 9            // Servo motor para controlar abertura/fechamento da porta
#define buzzer 8                 // Buzzer para emitir sons de alerta

// ====================================================================
// VARIÁVEIS GLOBAIS DE CONTROLE DO SISTEMA
// ====================================================================
bool controle_automatico = true;              // Controla se o sistema opera automaticamente
int frequencia_buzzer = 0;                    // Frequência atual do buzzer (não utilizada)

// Variáveis do modo visitante
bool modo_visitante_ativo = false;            // Indica se o modo visitante está ativo
bool alarme_ativo = false;                    // Indica se o alarme está ativo
bool alarme_era_ativo_antes = false;          // Salva estado do alarme antes do modo visitante
unsigned long tempo_inicio_modo_visitante = 0; // Timestamp do início do modo visitante
const unsigned long duracao_modo_visitante = 10000; // Duração do modo visitante (10 segundos)

// Variáveis de controle e comunicação
int comando = 0;                              // Variável para armazenar comandos recebidos
int posicao_servo_atual = -1;                 // Posição atual do servo (-1 = indefinida)
int ultimo_status_porta = -1;                 // Último status da porta reportado via serial

// Variável para interrupção do botão
volatile bool botao_pressionado_flag = false; // Flag para sinalizar pressão do botão (volatile para ISR)

// Objeto do servo motor
Servo servo;                                  // Instância do servo motor

// Configuração do sensor LDR
const int limiarLDR = 500;                    // Limiar de luminosidade para abertura automática

// ====================================================================
// FUNÇÃO DE INICIALIZAÇÃO - EXECUTADA UMA VEZ AO LIGAR O ARDUINO
// ====================================================================
void setup() {
  // Configuração dos pinos de entrada e saída
  pinMode(botao_campainha, INPUT_PULLUP);     // Botão com resistor pull-up interno habilitado

  // Configuração dos LEDs da porta como saída
  pinMode(led_verde_porta, OUTPUT);
  pinMode(led_vermelho_porta, OUTPUT);
  
  // Configuração dos LEDs do alarme como saída
  pinMode(led_verde_alarme, OUTPUT);
  pinMode(led_vermelho_alarme, OUTPUT);

  // Configuração da interrupção para detecção de visitante
  // Quando o botão é pressionado (RISING), chama a função botaoInterrupt
  attachInterrupt(digitalPinToInterrupt(botao_campainha), botaoInterrupt, RISING);

  // Inicialização da comunicação serial
  Serial.begin(9600);                         // Configura taxa de transmissão serial

  // Inicialização do servo motor
  servo.attach(servo_motor);                  // Conecta o servo ao pino definido
  delay(500);                                 // Aguarda estabilização do servo
  
  // Mensagem de inicialização
  Serial.println("SISTEMA INICIADO");
}

// ====================================================================
// LOOP PRINCIPAL - EXECUTADO CONTINUAMENTE
// ====================================================================
void loop() {
  // ----------------------------------------------------------------
  // PROCESSAMENTO DE COMANDOS VIA COMUNICAÇÃO SERIAL
  // ----------------------------------------------------------------
  if (Serial.available() > 0) {
    // Lê comando da comunicação serial até encontrar quebra de linha
    String input = Serial.readStringUntil('\n');
    input.trim();                                  // Remove espaços em branco

    if (input.length() > 0) {
      char comando = input.charAt(0);              // Primeiro caractere é o comando
      int valor = 0;

      // Se há mais caracteres, extrai o valor numérico
      if (input.length() > 1) {
        valor = input.substring(1).toInt();
      }

      // Exibe informações do comando recebido
      Serial.print("Comando Recebido: ");
      Serial.print(comando);
      Serial.print(" ");
      Serial.println(valor);

      // Executa o comando correspondente
      executarComando(comando, valor);
    }
  }

  // ----------------------------------------------------------------
  // PROCESSAMENTO DE DETECÇÃO DE VISITANTE
  // ----------------------------------------------------------------
  if (botao_pressionado_flag) {
    // Informa detecção de visitante via serial
    Serial.println("Visitante Detectado!");
    Serial.println("VD");                         // Código para aplicação externa
    botao_pressionado_flag = false;               // Reseta flag de interrupção
  }

  // ----------------------------------------------------------------
  // CONTROLE DO TEMPO DO MODO VISITANTE
  // ----------------------------------------------------------------
  if (modo_visitante_ativo && (millis() - tempo_inicio_modo_visitante >= duracao_modo_visitante)) {
    // Tempo do modo visitante expirou
    portaFechada();                               // Força fechamento da porta
    modo_visitante_ativo = false;                 // Desativa modo visitante

    // Reestabelece estado anterior do alarme
    if (alarme_era_ativo_antes) {
      alarmeLigado();
    }

    // Informa desativação do modo visitante
    Serial.println("Modo Visitante: DESATIVADO");
    Serial.println("MVD");                        // Código para aplicação externa
  }

  // ----------------------------------------------------------------
  // CONTROLE AUTOMÁTICO BASEADO EM SENSOR DE LUZ
  // ----------------------------------------------------------------
  if (!modo_visitante_ativo && controle_automatico) {
    // Lê valor do sensor LDR (0-1023)
    int valorLDR = analogRead(ldr);
	
    // Decide abertura/fechamento baseado na luminosidade
    if (valorLDR > limiarLDR) {
      portaAberta();                              // Há luz suficiente - abre porta
    } else {
      portaFechada();                             // Pouca luz - fecha porta
    }
  }

  // ----------------------------------------------------------------
  // RELATÓRIO DE STATUS DA PORTA VIA SERIAL
  // ----------------------------------------------------------------
  if (posicao_servo_atual == 0 && ultimo_status_porta != 0) {
    // Porta mudou para fechada
    Serial.println("Status Atual da Porta: FECHADA");
    ultimo_status_porta = 0;
  } else if (posicao_servo_atual == 90 && ultimo_status_porta != 1) {
    // Porta mudou para aberta
    Serial.println("Status Atual da Porta: ABERTA");
    ultimo_status_porta = 1;
  }

  // Pausa entre ciclos do loop principal
  delay(500);
}

// ====================================================================
// FUNÇÃO PARA FECHAR A PORTA
// ====================================================================
void portaFechada() {
  // Verifica se a porta não está já fechada para evitar movimento desnecessário
  if (posicao_servo_atual != 0) {
    // Atualiza indicadores visuais da porta
    digitalWrite(led_verde_porta, LOW);           // Apaga LED verde (porta aberta)
    digitalWrite(led_vermelho_porta, HIGH);       // Acende LED vermelho (porta fechada)
    
    // Movimenta servo para posição fechada
    servo.write(0);                               // Move servo para 0° (posição fechada)
    delay(500);                                   // Aguarda conclusão do movimento
    posicao_servo_atual = 0;                      // Atualiza registro da posição atual

    // Informa fechamento da porta
    Serial.println("Porta -> FECHADA");
    Serial.println("PF");                         // Código para aplicação externa
  }
}

// ====================================================================
// FUNÇÃO PARA ABRIR A PORTA
// ====================================================================
void portaAberta() {
  // Verifica se a porta não está já aberta para evitar movimento desnecessário
  if (posicao_servo_atual != 90) {
    // Atualiza indicadores visuais da porta
    digitalWrite(led_verde_porta, HIGH);          // Acende LED verde (porta aberta)
    digitalWrite(led_vermelho_porta, LOW);        // Apaga LED vermelho (porta fechada)
    
    // Movimenta servo para posição aberta
    posicao_servo_atual = 90;                     // Atualiza registro da posição
    servo.write(90);                              // Move servo para 90° (posição aberta)
    delay(500);                                   // Aguarda conclusão do movimento

    // Informa abertura da porta
    Serial.println("Porta -> ABERTA");
    Serial.println("PA");                         // Código para aplicação externa
  }
}

// ====================================================================
// FUNÇÃO PARA ATIVAR O ALARME
// ====================================================================
void alarmeLigado() {
  // Atualiza indicadores visuais do alarme
  digitalWrite(led_verde_alarme, HIGH);           // Acende LED verde (alarme ativo)
  digitalWrite(led_vermelho_alarme, LOW);         // Apaga LED vermelho (alarme inativo)
  
  // Atualiza estado interno do alarme
  alarme_ativo = true;

  // Informa ativação do alarme
  Serial.println("Status Alarme: LIGADO");
  Serial.println("AL");                           // Código para aplicação externa
}

// ====================================================================
// FUNÇÃO PARA DESATIVAR O ALARME
// ====================================================================
void alarmeDesligado() {
  // Atualiza indicadores visuais do alarme
  digitalWrite(led_verde_alarme, LOW);            // Apaga LED verde (alarme ativo)
  digitalWrite(led_vermelho_alarme, HIGH);        // Acende LED vermelho (alarme inativo)
  
  // Atualiza estado interno do alarme
  alarme_ativo = false;

  // Informa desativação do alarme
  Serial.println("Status Alarme: DESLIGADO");
  Serial.println("AD");                           // Código para aplicação externa
}

// ====================================================================
// FUNÇÃO PARA ATIVAR O MODO VISITANTE
// ====================================================================
void modoVisitanteLigado() {
  // Informa ativação do modo visitante
  Serial.println("Status Modo Visitante: LIGADO");
  Serial.println("VL");                           // Código para aplicação externa
  
  // Salva estado atual do alarme para restaurar depois
  alarme_era_ativo_antes = alarme_ativo;
  
  // Desativa alarme durante modo visitante para evitar falsos alarmes
  if (alarme_ativo) {
    alarmeDesligado();
  }
  
  // Abre porta automaticamente para o visitante
  portaAberta();
  
  // Ativa modo visitante e inicia contagem de tempo
  modo_visitante_ativo = true;
  tempo_inicio_modo_visitante = millis();         // Marca timestamp de início
}

// ====================================================================
// FUNÇÃO PARA DESATIVAR O MODO VISITANTE
// ====================================================================
void modoVisitanteDesligado() {
  // Força fechamento da porta
  portaFechada();
  
  // Desativa modo visitante
  modo_visitante_ativo = false;
  
  // Restaura estado anterior do alarme
  if (alarme_era_ativo_antes) {
    alarmeLigado();
  }

  // Informa desativação do modo visitante
  Serial.println("Status Modo Visitante: DESLIGADO");
  Serial.println("MVD");                          // Código para aplicação externa
}

// ====================================================================
// FUNÇÃO PARA CONTROLAR O MODO AUTOMÁTICO
// ====================================================================
void controlarAutomatico(int valor) {
  // Interpreta valor recebido para ativar/desativar controle automático
  if (valor == 1) {
    controle_automatico = true;
    Serial.println("Controle Automatico: ATIVADO");
  } else if (valor == 0) {
    controle_automatico = false;
    Serial.println("Controle Automatico: DESATIVADO");
  } else {
    // Valor inválido recebido
    Serial.println("Valor Invalido para controlarAutomatico (use 0 ou 1)");
  }
}

// ====================================================================
// FUNÇÃO PARA EXECUTAR COMANDOS RECEBIDOS VIA SERIAL
// ====================================================================
void executarComando(char comando, int valor) {
  // Interpreta e executa comandos baseados no caractere recebido
  switch (comando) {
    case 'A':                                     // Comando A: Força fechamento da porta
      portaFechada();
      controlarAutomatico(0);                     // Desativa controle automático
      break;
      
    case 'B':                                     // Comando B: Força abertura da porta
      portaAberta();
      controlarAutomatico(0);                     // Desativa controle automático
      break;
      
    case 'C':                                     // Comando C: Ativa alarme
      alarmeLigado();
      break;
      
    case 'D':                                     // Comando D: Desativa alarme
      alarmeDesligado();
      break;
      
    case 'E':                                     // Comando E: Ativa modo visitante
      modoVisitanteLigado();
      break;
      
    case 'F':                                     // Comando F: Controla buzzer
      frequenciaBuzzer(valor);
      break;
      
    case 'G':                                     // Comando G: Controla modo automático
      controlarAutomatico(valor);                 // Valor deve ser 0 ou 1
      break;
      
    default:                                      // Comando não reconhecido
      Serial.println("ERRO: Comando Inválido.");
      break;
  }
}

// ====================================================================
// FUNÇÃO DE INTERRUPÇÃO PARA BOTÃO DA CAMPAINHA
// ====================================================================
void botaoInterrupt() {
  // Esta função é chamada automaticamente quando o botão é pressionado
  // Deve ser rápida e apenas definir flags - não fazer processamento complexo
  botao_pressionado_flag = true;                  // Sinaliza detecção de visitante
}

// ====================================================================
// FUNÇÃO PARA CONTROLAR BUZZER
// ====================================================================
void frequenciaBuzzer(int freq) {
  // Gera um pulso simples no buzzer com frequência especificada
  // Frequência determina o tempo que o buzzer fica ligado/desligado
  
  digitalWrite(8, HIGH);                          // Liga buzzer
  delay(freq);                                    // Mantém ligado por 'freq' millisegundos
  digitalWrite(8, LOW);                           // Desliga buzzer
  delay(freq);                                    // Mantém desligado por 'freq' millisegundos
  
}