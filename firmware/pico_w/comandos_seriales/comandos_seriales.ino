
char commandBuffer[20];
int commandIndex = 0;

bool ledState = false;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("CSR-BOT SYSTEM ONLINE");
  Serial.println("Esperando comandos...");
}

void loop() {

  while (Serial.available() > 0) {

    char received = Serial.read();

    if (received == '\n' || received == '\r') {

      commandBuffer[commandIndex] = '\0';

      if (strcmp(commandBuffer, "ON") == 0) {

        ledState = true;
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("LED ENCENDIDO");

      } 
      else if (strcmp(commandBuffer, "OFF") == 0) {

        ledState = false;
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("LED APAGADO");

      } 
      else if (strcmp(commandBuffer, "STATUS") == 0) {

        Serial.print("Estado del LED: ");
        Serial.println(ledState ? "ON" : "OFF");

      } 
      else if (commandIndex > 0) {

        Serial.println("Comando desconocido");

      }

      commandIndex = 0;

    } 
    else if (commandIndex < 19) {

      commandBuffer[commandIndex++] = received;

    }
  }
}
