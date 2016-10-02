//    Arduino PPM Generator
//    Copyright (C) 2015-2016  Alexandr Kolodkin <alexandr.kolodkin@gmail.com>
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "ModbusSlave.h"

// Максимальное количество каналов
#define MAX_COUNT 16     

enum State {
  Pulse,             // Импульс n-го канала
  StartSync,         // Начало синхроимпульса
  ContinueSync,      // Продолжение синхроимпульса
  FinishSync         // окончание синхроимпульса
};

// little endian
typedef union {
  unsigned long raw;
  struct {
    word low;
    word high;
  };
} long_t;

typedef union __attribute__ ((packed)) {
  word raw[];
  struct __attribute__ ((packed)) {
    word quant;               // 1 мксек в тактах системной частоты
    word max_count;           // Мксимальное количество каналлов
    word state;               // 2 - Вкл. (инверсия) / 1 - Вкл. / 0 - Выкл.
    word count;               // Количество каналов (0 ... MAX_COUNT)
    word pause;               // Длительности паузы (в тактах системной частоты)
    long_t sync;              // Длительность импульса синхронизации (в тактах системной частоты)
    word channel[MAX_COUNT];  // Длительности импульсов (в тактах системной частоты)
  };
} regs_t;

regs_t tmp;                   // Временный набор данных
regs_t ppm;                   // Рабочий набор данных
volatile byte state = Pulse;  // Текущее состояние
volatile byte current = 0;    // Текущий номер канала

byte const modbus_registers_count = sizeof(regs_t) / sizeof(word);

word modbus_get_register(word id) {
	return tmp.raw[id];
}

void modbus_set_register(word id, word value) {
  digitalWrite(9, HIGH);
	if (id > 1) tmp.raw[id] = value;

//  Serial.println(id);
//  Serial.println(value);  
  
  digitalWrite(9, LOW);
}

// Инициализация контроллера
void setup() {
	// Инициализация периферии
  pinMode(9, OUTPUT);         // Debug
  pinMode(10, OUTPUT);        // PPM
  digitalWrite(10, HIGH);

  // Инициализация значений каналов
  tmp.state        = 0;
  tmp.max_count    = MAX_COUNT;
  tmp.count        = 8;
  tmp.quant        = F_CPU / 1000000;
  tmp.pause        = F_CPU / 1000000 * 200;
  tmp.sync.raw     = F_CPU / 1000000 * 22500 - F_CPU / 1000000 * 300 * 8;
  
  // Длительность канала 300 мксек
  for (byte i = 0; i < MAX_COUNT; i++) tmp.channel[i] = 300 * (unsigned long) tmp.quant;

  // Настраиваем MODBUS
  modbus_start();

 
  Serial.println(tmp.state);
  Serial.println(tmp.count);
  Serial.println(tmp.pause);
  Serial.println(tmp.sync.low);
  Serial.println(tmp.sync.high);
  Serial.println(tmp.sync.raw);
  for (byte i = 0; i < MAX_COUNT; i++) {
    Serial.println(tmp.channel[i]);
  }
}

// Основной цикл
void loop() {
	word lastState = tmp.state;
	modbus_update();
	if (lastState != tmp.state) {
		tmp.state > 0 ? Start() : Stop();
	}
}

// Запустить генерацию
void Start() {
  
	cli();                                           // Глобальный запрет прерываний
	ppm = tmp;          
	TIMSK1 = B00000001;                              // Разрешение прерывания от таймера
	TCCR1A = ppm.state == 2 ? B00110011 : B00100011; // FAST PWM MODE 15
	TCCR1B = B00011001;                              // Предделитель = 1
	TCCR1C = B00000000;                              //
	OCR1A  = ppm.channel[0];                         // Длительность первого импульса c паузой
	OCR1B  = ppm.channel[0] - ppm.pause;             // Длительность первого импульса без паузы
	current = 1;                                     //
	sei();                                           // Глобальное разрешение прерываний
}

// Остановить генерацию
void Stop() {
	cli();             // Глобальный запрет прерываний
	TCCR1B = 0;        // Останавливаем счетчик
	TIMSK1 = 0;        // Отключаем прерывание
	sei();             // Глобальное разрешение прерываний
}

// Прерывание при переполнении
ISR(TIMER1_OVF_vect) {

	switch (state) {
		
	// Импульс n-го канала
	case Pulse:		
		OCR1A = ppm.channel[current];                 // Длительность текущего импульса c паузой
		OCR1B = ppm.channel[current] - ppm.pause;     // Длительность текущего импульса без паузы  

	  // Переходим к формированию синхроимпульса
	  if (++current == ppm.count) state = (ppm.sync.high == 0) ? FinishSync : ppm.sync.low > ppm.pause ? ContinueSync : StartSync;                               
	  break;

	// Если младшие ppm.sync.low меньше чем ppm.pause, то начинаем импульс длительностью ppm.pause и
  // уже на следующем проходе ppm.sync.low будет больше ppm.pause
	case StartSync:
    OCR1B = OCR1A = ppm.pause;
 		ppm.sync.raw -= ppm.pause;
		state = ppm.sync.high ? ContinueSync : FinishSync;
		break;

	// Если ppm.sync.high > 0 начинаем импульс максимальной длительности
	case ContinueSync: 
		OCR1A = OCR1B = 0xFFFF;       
		if (--ppm.sync.high == 0) state = FinishSync;
		break;

	// Завершение синхроимпульса
	case FinishSync: 
		OCR1A = ppm.sync.low;
		OCR1B = ppm.sync.low - ppm.pause;
    ppm = tmp;
		state = Pulse;
		current = 0;
	}
}

