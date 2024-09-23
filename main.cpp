#include "mbed.h"
#include "TM1638x8.h"

TM1638 display(D7, D8, D9); // DIO, CLK, STB
Ticker ticker;
uint32_t counter = 0;
DigitalIn sensor(D10);
bool sensorTriggered = false;
int shiftAmount = 0;
bool isRunning = true;
bool shiftingEnabled = false;
bool invertedDisplay = false; // Para el modo de inversión de display

void updateDisplay() {
    // Verificar si el sensor detectó algo y si el contador está en ejecución
    if (sensor == 0 && !sensorTriggered && isRunning) {
        if (!invertedDisplay) {
            counter++;
            if (counter > 99999999) counter = 0;
        } else {
            counter--;
            if (counter > 99999999) counter = 99999999;
        }
        sensorTriggered = true;
    } else if (sensor == 1) {
        sensorTriggered = false;
    }

    // Aplicar el desplazamiento de dígitos si está habilitado
    uint32_t shiftedCounter = counter;
    if (shiftingEnabled) {
        shiftedCounter = (counter << shiftAmount) | (counter >> (32 - shiftAmount));
        shiftedCounter &= 0xFFFFFFFF; // Asegurarse de no exceder 32 bits
    }

    // Mostrar el número en los dígitos
    display.setDisplayToDecNumber(shiftedCounter, true);

    // Encender LEDs para indicar el modo actual
    // Invertir la asignación de LEDs:
    display.setLED(7, isRunning);         // LED 8 (S1): Contador en ejecución
    display.setLED(6, shiftingEnabled);   // LED 7 (S2): Modo de desplazamiento habilitado
    
    // Usar los LEDs restantes para mostrar la cantidad de desplazamientos en binario
    for (int i = 0; i < 3; i++) {
        display.setLED(5 - i, (shiftAmount & (1 << i)) != 0); // LEDs 6 a 4
    }

    // Mostrar si el display está invertido (S8 controla LED 1)
    display.setLED(0, invertedDisplay); // LED 1: Modo de inversión de display
}

void checkButtons() {
    uint8_t buttons = display.readButtons();

    // S1 (Encender/apagar contador, controlar LED 8)
    if (buttons & 0x01) {
        isRunning = !isRunning;
        ThisThread::sleep_for(200ms);  // Debounce
    }

    // S2 (Habilitar/deshabilitar modo de desplazamiento, controlar LED 7)
    if (buttons & 0x02) {
        shiftingEnabled = !shiftingEnabled;
        ThisThread::sleep_for(200ms);  // Debounce
    }

    // S3 (Desplazar hacia la izquierda, controlar LED 6)
    if (buttons & 0x04 && shiftingEnabled) {
        shiftAmount = (shiftAmount + 1) % 8;
        ThisThread::sleep_for(200ms);  // Debounce
    }

    // S4 (Desplazar hacia la derecha, controlar LED 5)
    if (buttons & 0x08 && shiftingEnabled) {
        shiftAmount = (shiftAmount + 7) % 8;
        ThisThread::sleep_for(200ms);  // Debounce
    }

    // S5 (Reiniciar contador, controlar LED 4)
    if (buttons & 0x10) {
        counter = invertedDisplay ? 99999999 : 0;
        shiftAmount = 0;
        updateDisplay();
        ThisThread::sleep_for(200ms);  // Debounce
    }

    // S6 (Incrementar contador manualmente, controlar LED 3)
    if (buttons & 0x20) {
        if (!invertedDisplay) {
            counter++;
            if (counter > 99999999) counter = 0; // Limitar el valor del contador
        } else {
            counter--;
            if (counter > 99999999) counter = 99999999; // Limitar el valor del contador
        }
        updateDisplay();
        ThisThread::sleep_for(200ms);  // Debounce
    }

    // S7 (Decrementar contador manualmente, controlar LED 2)
    if (buttons & 0x40) {
        if (!invertedDisplay) {
            if (counter > 0) {
                counter--;
            } else {
                counter = 99999999; // Si el contador es 0, se establece en el valor máximo
            }
        } else {
            counter++;
            if (counter > 99999999) counter = 0; // Limitar el valor del contador
        }
        updateDisplay();
        ThisThread::sleep_for(200ms);  // Debounce
    }

    // S8 (Invertir display y modo de conteo, controlar LED 1)
    if (buttons & 0x80) {
        invertedDisplay = !invertedDisplay;
        // Ajustar el valor del contador según el nuevo modo
        if (invertedDisplay) {
            if (counter == 0) {
                counter = 99999999;
            }
        } else {
            if (counter == 99999999) {
                counter = 0;
            }
        }
        updateDisplay();
        ThisThread::sleep_for(200ms);  // Debounce
    }
}

int main() {
    display.init();
    display.setBrightness(7); // Máxima intensidad

    // Inicializar el sensor como entrada
    sensor.mode(PullUp); // Configurar resistencia pull-up para evitar lecturas flotantes

    ticker.attach(&updateDisplay, 10ms); // Actualizar con frecuencia para capturar el sensor

    while(1) {
        checkButtons();
        ThisThread::sleep_for(10ms);
    }
}
