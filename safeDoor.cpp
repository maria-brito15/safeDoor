#include <Servo.h>

#define botao_campainha 2

#define led_verde_porta 13
#define led_vermelho_porta 12

#define led_verde_alarme 11
#define led_vermelho_alarme 10

#define ldr A2
#define servo_motor 9
#define buzzer 8

bool controle_automatico = true;
int frequencia_buzzer = 0;

bool modo_visitante_ativo = false;
bool alarme_ativo = false;
bool alarme_era_ativo_antes = false;
unsigned long tempo_inicio_modo_visitante = 0;
const unsigned long duracao_modo_visitante = 10000;

int comando = 0;
int posicao_servo_atual = -1;
int ultimo_status_porta = -1;

volatile bool botao_pressionado_flag = false;

Servo servo;

const int limiarLDR = 500; 

void setup() {
  pinMode(botao_campainha, INPUT_PULLUP);

  pinMode(led_verde_porta, OUTPUT);
  pinMode(led_vermelho_porta, OUTPUT);
  pinMode(led_verde_alarme, OUTPUT);
  pinMode(led_vermelho_alarme, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(botao_campainha), botaoInterrupt, RISING);

  Serial.begin(9600);

  servo.attach(servo_motor);
  delay(500);
  
  Serial.println("SISTEMA INICIADO");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      char comando = input.charAt(0);
      int valor = 0;

      if (input.length() > 1) {
        valor = input.substring(1).toInt();
      }

      Serial.print("Comando Recebido: ");
      Serial.print(comando);
      Serial.print(" ");
      Serial.println(valor);

      executarComando(comando, valor);
    }
  }

  if (botao_pressionado_flag) {
    Serial.println("Visitante Detectado!");
    Serial.println("VD");
    botao_pressionado_flag = false;
  }

  if (modo_visitante_ativo && (millis() - tempo_inicio_modo_visitante >= duracao_modo_visitante)) {
    portaFechada();
    modo_visitante_ativo = false;

    if (alarme_era_ativo_antes) {
      alarmeLigado();
    }

    Serial.println("Modo Visitante: DESATIVADO");
    Serial.println("MVD");
  }

  if (!modo_visitante_ativo && controle_automatico) {
    int valorLDR = analogRead(ldr);
	
    if (valorLDR > limiarLDR) {
      portaAberta();
    } else {
      portaFechada();
    }
  }

  if (posicao_servo_atual == 0 && ultimo_status_porta != 0) {
    Serial.println("Status Atual da Porta: FECHADA");
    ultimo_status_porta = 0;
  } else if (posicao_servo_atual == 90 && ultimo_status_porta != 1) {
    Serial.println("Status Atual da Porta: ABERTA");
    ultimo_status_porta = 1;
  }

  delay(500);
}

void portaFechada() {
  if (posicao_servo_atual != 0) {
    digitalWrite(led_verde_porta, LOW);
    digitalWrite(led_vermelho_porta, HIGH);
    
    servo.write(0);
    delay(500); 
    posicao_servo_atual = 0;

    Serial.println("Porta -> FECHADA");
    Serial.println("PF");
  }
}

void portaAberta() {
  if (posicao_servo_atual != 90) {
    digitalWrite(led_verde_porta, HIGH);
    digitalWrite(led_vermelho_porta, LOW);
    
    posicao_servo_atual = 90;
    servo.write(90);
    delay(500);

    Serial.println("Porta -> ABERTA");
    Serial.println("PA");
  }
}

void alarmeLigado() {
  digitalWrite(led_verde_alarme, HIGH);
  digitalWrite(led_vermelho_alarme, LOW);
  alarme_ativo = true;

  Serial.println("Status Alarme: LIGADO");
  Serial.println("AL");
}

void alarmeDesligado() {
  digitalWrite(led_verde_alarme, LOW);
  digitalWrite(led_vermelho_alarme, HIGH);
  alarme_ativo = false;

  Serial.println("Status Alarme: DESLIGADO");
  Serial.println("AD");
}

void modoVisitanteLigado() {
  Serial.println("Status Modo Visitante: LIGADO");
  Serial.println("VL");
  
  alarme_era_ativo_antes = alarme_ativo;
  
  if (alarme_ativo) {
    alarmeDesligado();
  }
  
  portaAberta();
  modo_visitante_ativo = true;
  tempo_inicio_modo_visitante = millis();
}

void modoVisitanteDesligado() {
  portaFechada();
  modo_visitante_ativo = false;
  
  if (alarme_era_ativo_antes) {
    alarmeLigado();
  }

  Serial.println("Status Modo Visitante: DESLIGADO");
  Serial.println("MVD");
}

void controlarAutomatico(int valor) {
  if (valor == 1) {
    controle_automatico = true;
    Serial.println("Controle Automatico: ATIVADO");
  } else if (valor == 0) {
    controle_automatico = false;
    Serial.println("Controle Automatico: DESATIVADO");
  } else {
    Serial.println("Valor Invalido para controlarAutomatico (use 0 ou 1)");
  }
}

void executarComando(char comando, int valor) {
  switch (comando) {
    case 'A':
      portaFechada();
      controlarAutomatico(0);
      break;
    case 'B':
      portaAberta();
      controlarAutomatico(0);
      break;
    case 'C':
      alarmeLigado();
      break;
    case 'D':
      alarmeDesligado();
      break;
    case 'E':
      modoVisitanteLigado();
      break;
    case 'F':
      frequenciaBuzzer(valor);
      break;
    case 'G':
      controlarAutomatico(valor);
      break;
    default:
      Serial.println("ERRO: Comando Inválido.");
      break;
  }
}

void botaoInterrupt() {
  botao_pressionado_flag = true;
}

void frequenciaBuzzer(int freq) {
  digitalWrite(8, HIGH);
  delay(freq);
  digitalWrite(8, LOW);
  delay(freq);
}