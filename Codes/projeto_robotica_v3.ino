/* prep

  CURSO:
  * Ciência da Computação – UNICAP

  Disciplina:
  * Robótica Inclusiva

  PROJETO:
  * Sistemas de Monitoramento e Irrigação com Arduino

  ALUNOS: 
  - Rafael Sampaio e Silva – RA 844559
  - Tarsila Amado Alves de Brito – RA 849038

*/

#include <LiquidCrystal.h>

#define umidadeAnalog A0
#define pushbuttonOptions 7
#define pushbuttonDone 8
#define relePin 9

LiquidCrystal lcd(2, 3, 4, 5, 11, 12);


int humidityValue;
int nivelDeIrrigacao = 1; // Frequência para irrigar plantas diferentes
bool configurandoNivel = false; // Flag para saber se está configurando o nível
unsigned long ultimoTempoDeIrrigacao = 0; // Armazena o tempo da última irrigação
unsigned long intervaloDeIrrigacao = 0; // Intervalo para próximo ciclo

// Definir os intervalos em milissegundos
#define INTERVALO_DIARIO 60000 // 24 horas
#define INTERVALO_2DIAS 172800000 // 48 horas

void MenuLCD(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humidade: ");
  lcd.print(humidityValue);
  lcd.setCursor(0, 1);

  if(nivelDeIrrigacao == 1){
    lcd.print("Frequencia: ALTA");
  }else 
  if(nivelDeIrrigacao == 2){
    lcd.print("Frequencia:DIARIA");
  }else
  if(nivelDeIrrigacao == 3){
    lcd.print("Frequencia:2DIAS");
  }
}
void ConfigurandoMenu(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Usar Frequencia:");
  lcd.setCursor(0, 1);

  if (nivelDeIrrigacao == 1){
    lcd.print("--->ALTA");
  }else
  if(nivelDeIrrigacao == 2){
    lcd.print("--->DIARIA");
  }else
  if(nivelDeIrrigacao == 3){
    lcd.print("--->2 DIAS");
  }
}
void abrirValvula(){
  digitalWrite(relePin, HIGH); 
}
void fecharValvula(){
  digitalWrite(relePin, LOW); 
}

void abrirBombaRapido() {
  digitalWrite(relePin, LOW);
  unsigned long tempoBomba = millis(); 
  while (millis() - tempoBomba < 5000) { // Mantém a bomba ligada por 5 segundos
    // Verifica os botões durante o funcionamento da bomba
    int buttonStateOptions = digitalRead(pushbuttonOptions);
    int buttonStateDone = digitalRead(pushbuttonDone);
    
    if (buttonStateOptions == LOW || buttonStateDone == LOW) {
      // Se qualquer botão for pressionado, interrompe o ciclo e sai
      configurandoNivel = true;
      fecharValvula(); // Fecha válvula para interromper a irrigação
      return;
    }
    delay(100); // Pequena pausa para evitar leituras excessivas
  }
  digitalWrite(relePin, HIGH); // Desliga a bomba
  delay(1000); // Pausa após desligar
}

void loop() {
  humidityValue = analogRead(umidadeAnalog);
  humidityValue = map(humidityValue, 1023, 315, 0, 100);

  // Leitura e alternância do estado dos botões
  int buttonStateOptions = digitalRead(pushbuttonOptions);
  static int lastButtonStateOptions = HIGH;
  if (buttonStateOptions == LOW && lastButtonStateOptions == HIGH) {
    configurandoNivel = !configurandoNivel;
    delay(200); // Debounce
  }
  lastButtonStateOptions = buttonStateOptions;

  // Modo configuração
  if (configurandoNivel) {
    ConfigurandoMenu();

    int buttonStateDone = digitalRead(pushbuttonDone);
    static int lastButtonStateDone = HIGH;
    if (buttonStateDone == LOW && lastButtonStateDone == HIGH) {
      nivelDeIrrigacao++;
      if (nivelDeIrrigacao > 3) {
        nivelDeIrrigacao = 1;
      }
      delay(200); // Debounce
    }
    lastButtonStateDone = buttonStateDone;

  } else {
    // Exibe menu LCD no modo padrão
    MenuLCD();

    unsigned long tempoAtual = millis();

    // Lógica de irrigação =======================================
    if (nivelDeIrrigacao == 1) { // Modo ALTA
      if (humidityValue < 40) {
        abrirBombaRapido();
      }
    } else if (nivelDeIrrigacao == 2) { // Modo DIÁRIA
      intervaloDeIrrigacao = INTERVALO_DIARIO;
      if ((tempoAtual - ultimoTempoDeIrrigacao >= intervaloDeIrrigacao) && humidityValue < 40) {
        abrirBombaRapido();
        ultimoTempoDeIrrigacao = tempoAtual;
      }
    } else if (nivelDeIrrigacao == 3) { // Modo CADA 2 DIAS
      intervaloDeIrrigacao = INTERVALO_2DIAS;
      if ((tempoAtual - ultimoTempoDeIrrigacao >= intervaloDeIrrigacao) && humidityValue < 40) {
        abrirBombaRapido();
        ultimoTempoDeIrrigacao = tempoAtual;
      }
    }
  }

  delay(100);
}
