# Arduino Door Control System (SafeDoor)

## Overview

This system implements an automated door control with alarm features, visitor detection, and manual/automatic control. The system uses an LDR sensor to detect luminosity and automatically control door opening/closing, plus has a mobile application developed in App Inventor for remote control via Bluetooth.

## Attachments

### Tinkercad
- [Assembly Image](https://ibb.co/sJ1s44V4)

### App Inventor
- [MIT App Inventor Code](https://ibb.co/G4RyhND1)

## Components Used

### Hardware

- **Arduino UNO** (main controller)
- **Servo Motor** (pin 9) - controls door opening/closing
- **Doorbell Button** (pin 2) - detects visitors
- **LDR Sensor** (pin A2) - detects luminosity
- **Piezo Buzzer** (pin 8) - sound signals
- **Bluetooth Module** - communication with mobile app
- **Accelerometer** - motion/vibration detection
- **Proximity Sensor** - visitor detection by proximity

### Indicator LEDs

- **Door Green LED** (pin 13)
- **Door Red LED** (pin 12)
- **Alarm Green LED** (pin 11)
- **Alarm Red LED** (pin 10)

## Pin Definitions

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

## Mobile Application (App Inventor)

### Application Features

The App Inventor application offers complete SafeDoor system control through Bluetooth communication:

#### 1. Manual Control Interface

- Switch for door control
- Automatic mode control through a button (toggle - on/off)
- Visual status indicators for door, automatic control, alarms, and visitor mode

#### 2. Voice Recognition System

- Voice command for "trancar porta"
- Voice command for "destravar porta"
- Voice command for "ativar modo visitante"
- TextToSpeech integration for audio feedback (response)

#### 3. Integrated Chat Bot

- Conversation interface for text command control
- Automatic system responses

#### 4. Sensor Monitoring

- **Accelerometer**: Detects movement/vibration to activate alarm
- **Proximity Sensor**: When approaching the screen, activates visitor mode

## App Inventor Programming Logic

### System Initialization

```blocks
when Screen1.Initialize
do set global visitante to ""
set global respostaArd to ""
set global botao to ""
set global alarme to false
set global automatico to 1
```

### Voice Recognition Control (Door)

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

### Button Control (Automatic Control)

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

### Accelerometer (Alarm)

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

### Proximity Detection (Visitor)

```blocks
when Proximidade_Sensor.ProximityChanged
do if get distance >= MaximumRange
then call BluetoothClient1.SendText
text "E"
set visitanteMode_status to "ATIVADO"
else set visitanteMode_status to "DESATIVADO"
```

### Door Switch Control

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

### Bluetooth Communication

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

### Chat Interface

```blocks
when ChatBot1.GotResponse
do set TextBox2 to get responseText
get responseText
```

### Slider Control

```blocks
when Slider1.PositionChanged
do call BluetoothClient1.SendText
text join "F" get thumbPosition
```

## App Inventor Components Used

### User Interface

- **TextBox1, TextBox2** - Text input and output
- **Button1** - Automatic mode control
- **porta_switch** - Switch for manual door control
- **Slider1** - Position/frequency control
- **ChatBot1** - Chat interface for commands

### Connectivity

- **BluetoothClient1** - Arduino communication
- **microfone** - Audio capture for voice recognition

### Sensors

- **Acelerometro_Sensor** - Motion detection
- **Proximidade_Sensor** - Proximity detection

### Time Control

- **Clock1** - Timer for Bluetooth communication polling

## Global Variables

### State Control (Arduino)

- **controle_automatico** - Flag to enable/disable automatic control
- **modo_visitante_ativo** - Visitor mode flag
- **alarme_ativo** - Alarm state flag
- **alarme_era_ativo_antes** - Alarm state backup before visitor mode

### Global Variables (App Inventor)

- **global visitante** - Visitor mode status
- **global respostaArd** - Stores Arduino response
- **global botao** - Button state
- **global alarme** - Alarm state
- **global automatico** - Automatic mode state

### Time Control

- **tempo_inicio_modo_visitante** - Visitor mode start timestamp
- **duracao_modo_visitante** - Visitor mode duration (10 seconds)

### Hardware Control

- **posicao_servo_atual** - Current servo motor position
- **ultimo_status_porta** - Last door status cache
- **limiarLDR** - LDR sensor threshold value (500)

## Main Operation

### Setup (Arduino)

1. Configures pins as input/output
2. Configures interrupt for doorbell button
3. Initializes serial communication (9600 baud)
4. Connects servo motor
5. Displays system started message

### Initialization (App Inventor)

1. Initializes global variables
2. Configures Bluetooth connection
3. Activates sensors (accelerometer, proximity)
4. Starts timer for continuous monitoring

### Main Loop (Arduino)

The main loop executes the following tasks:

1. **Serial Command Processing**
   - Reads commands via Serial/Bluetooth
   - Executes commands based on first character

2. **Visitor Detection**
   - Checks button interrupt flag
   - Sends "VD" notification via Serial/Bluetooth

3. **Visitor Mode Control**
   - Manages visitor mode timeout (10 seconds)
   - Restores previous alarm state

4. **Automatic Door Control**
   - Reads LDR sensor when automatic control is active
   - Opens/closes door based on luminosity

5. **Status Monitoring**
   - Reports door status changes via Serial/Bluetooth

## Available Commands

The system accepts commands via serial/Bluetooth communication:

| Command | Function | Parameter | Origin |
|---------|----------|-----------|--------|
| A | Closes door manually | - | App/Serial |
| B | Opens door manually | - | App/Serial |
| C | Turns alarm on | - | App/Serial |
| D | Turns alarm off | - | App/Serial |
| E | Activates visitor mode | - | App/Serial |
| F | Controls buzzer frequency | Frequency (ms) | App/Serial |
| G | Controls automatic mode | 0 = off, 1 = on | App/Serial |

## Main Features

### 1. Automatic Door Control

- **LDR Sensor**: Detects ambient luminosity
- **Threshold**: 500 (configurable)
- **Logic**:
  - Value > 500: Door open
  - Value ≤ 500: Door closed

### 2. Visitor Mode

- **Duration**: 10 seconds
- **Activation**: Physical button, voice command, proximity sensor
- **Actions**:
  - Temporarily disables alarm
  - Opens door
  - After timeout, restores previous state

### 3. Alarm System

- **States**: On/Off
- **Indicators**: Green/red LEDs
- **Activation**: Accelerometer (detects movement/vibration)
- **Integration**: Compatible with visitor mode

### 4. Visitor Detection

- **Methods**:
  - Physical button interrupt
  - Proximity sensor
  - Voice command
- **Response**: Serial/Bluetooth notification ("VD")

### 5. Voice Control

- **Supported commands**:
  - "trancar porta"
  - "destravar porta"
  - "ativar modo visitante"
- **Feedback**: Speech synthesis confirming actions

## Visual Indicators

### Door LEDs

- **Green**: Door open
- **Red**: Door closed

### Alarm LEDs

- **Green**: Alarm on
- **Red**: Alarm off

### App Interface

- **Door Status**: Indicative text (TRANCADA/DESTRANCADA)
- **Alarm Status**: Indicative text (ATIVADO/DESATIVADO)
- **Visitor Mode**: Visual indication when active

## Serial/Bluetooth Communication

### Status Messages

- **"PF"** - Door closed
- **"PA"** - Door open
- **"AL"** - Alarm on
- **"AD"** - Alarm off
- **"VL"** - Visitor mode on
- **"MVD"** - Visitor mode off
- **"VD"** - Visitor detected

### Configuration

- **Baud Rate**: 9600
- **Format**: Plain text with line breaks
- **Protocol**: Bluetooth Serial Profile (SPP)

## Main Functions

### Arduino

- **portaFechada()**: Closes door and updates indicators
- **portaAberta()**: Opens door and updates indicators
- **alarmeLigado()/alarmeDesligado()**: Controls alarm state
- **modoVisitanteLigado()/modoVisitanteDesligado()**: Manages visitor mode
- **botaoInterrupt()**: Interrupt function for doorbell
- **frequenciaBuzzer()**: Controls buzzer with specified frequency

### App Inventor

- **Voice Control**: Processing of spoken commands
- **Sensor Monitoring**: Continuous reading of accelerometer and proximity
- **Chat Interface**: Text command processing
- **Bluetooth Communication**: Data sending/receiving

## Technical Specifications

### Timing

- **Loop delay**: 500ms
- **Servo delay**: 500ms after movement
- **Visitor mode**: 10 seconds
- **Bluetooth polling**: Continuous via Clock1

### Limits

- **Servo positions**: 0° (closed) and 90° (open)
- **LDR threshold**: 500 (adjustable)
- **Baud rate**: 9600
- **Bluetooth range**: ~10 meters

## Team Members

- Maria Eduarda B.
- Gabriella A.
- Daniel R.
- Rafael T.