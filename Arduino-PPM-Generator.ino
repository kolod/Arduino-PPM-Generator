//    Arduino PPM Generator
//    Copyright (C) 2015  Alexandr Kolodkin <alexandr.kolodkin@gmail.com>
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


#include <CmdMessenger.h>

// Attach a new CmdMessenger object to the default Serial port
CmdMessenger cmdMessenger = CmdMessenger(Serial);

#define MAX_CHANEL_COUNT   16          // Максимальное количество каналов

enum {
	kStart,                            // Запустить генерацию
	kStop,                             // Остановить генерацию
	kSetChannelsCount,                 // Установить количество каналов
	kSetPause,                         // Установить длительности паузы           (в импульсах несущей частоты ШИМ или ×0.0625 мксек)
	kSetChannel,                       // Установить длительности канала          (в импульсах несущей частоты ШИМ или ×0.0625 мксек)
	kSetSync,                          // Установить длительность синхроимпульса  (в импульсах несущей частоты ШИМ или ×0.0625 мксек)
	kGetFreaquency,                    // Запрос частоты ШИМ
	kError,
	kFreaquency
};

typedef struct {
	byte count;                        // Количество каналов
	word chanel[MAX_CHANEL_COUNT];     // Длительности паузы           (в импульсах несущей частоты ШИМ или ×0.0625 мксек)
	word pause;                        // Длительности канала          (в импульсах несущей частоты ШИМ или ×0.0625 мксек)
	union {
		long sync;                 // Длительность синхроимпульса  (в импульсах несущей частоты ШИМ или ×0.0625 мксек)
		struct {
			word sync_lo;
			word sync_hi;
		};
	};                       
} TPPM;

byte current = 0; // Текущий номер канала
TPPM tmp;         // Временный набор данных
TPPM ppm;         // Рабочий набор данных

// Инициализация контроллера
void setup() {
	// Инициализация периферии
	pinMode(9, OUTPUT);
        pinMode(10, OUTPUT);

	// Инициализация значений каналов
	tmp.count = 8;                   // 8 каналов
	tmp.pause = 250*16;              // Пауза 250 мксек
	tmp.sync  = long(22500 - 8*300)*16;  // Длительность полного периода PPM 22,5 мсек

	// Длительность канала 300 мксек
	for (byte i = 0; i < MAX_CHANEL_COUNT; i++) tmp.chanel[i] = 300*16;

	// Настройка связи в ПК
	Serial.begin(115200);
	cmdMessenger.attach(OnUnknownCommand);
	cmdMessenger.attach(kStart, OnStart);                       // Запустить генерацию
	cmdMessenger.attach(kStop, OnStop);                         // Остановить генерацию
	cmdMessenger.attach(kSetChannelsCount, OnSetChannelsCount);   // Установить количество каналов
	cmdMessenger.attach(kSetPause, OnSetPause);                 // Установить длительности паузы, мксек
	cmdMessenger.attach(kSetChannel, OnSetChannel);               // Установить длительности канала, мксек
	cmdMessenger.attach(kSetSync, OnSetSync);                   // Установить длительность синхроимпульса, мксек
	cmdMessenger.attach(kGetFreaquency, OnGetFreaquency);       // Запрос частоты ШИМ

	// Запуск генерации с значениями по-умолчанию
	OnStart();
}

// Основной цикл
void loop() {
	cmdMessenger.feedinSerialData();
}

byte state = 0;

enum State {
	Pulse,             // Импульс n-го канала
	StartSync,         // Начало синхроимпульса
	ContinueSync,      // Продолжение синхроимпульса
	FinishSync         // окончание синхроимпульса
};

// Прерывание при переполнении
ISR(TIMER1_OVF_vect) {  
	switch (state) {
		
	// Импульс n-го канала
	case Pulse:
		// Длительность текущего импульса c паузой
		OCR1A = ppm.chanel[current];   

		// Длительность текущего импульса без паузы                         
		OCR1B = ppm.chanel[current++] - ppm.pause;              

	  	// Переходим к формированию синхроимпульса
	  	if (current == ppm.count) {                                 
	  		if (ppm.sync_hi == 0) {
	  		    state = FinishSync;
	  		} else if(ppm.sync_lo > ppm.pause) {
	  			state = ContinueSync;
	  		} else {
	  			state = StartSync;
	  		}
	  	}
	  	break;

	// Начало синхроимпульса
	case StartSync:
		OCR1A = ppm.pause - ppm.sync_lo + 1;
		OCR1B = ppm.pause - ppm.sync_lo + 1;
		ppm.sync -= long(ppm.pause - ppm.sync_lo + 1);
		state = (ppm.sync_hi == 0) ? FinishSync : ContinueSync;
		break;

	// Продолжение синхроимпульса
	case ContinueSync: 
		OCR1A = 0xFFFF;                      
		OCR1B = 0xFFFF;               
		if (--ppm.sync_hi == 0) state = FinishSync;
		break;

	// Завершение синхроимпульса
	case FinishSync: 
		OCR1A = ppm.sync_lo;
		OCR1B = ppm.sync_lo - ppm.pause;
		state = Pulse;
		current = 0;
		ppm = tmp;
	}
}

void OnUnknownCommand()
{
	cmdMessenger.sendCmd(kError, "Command without attached callback");
}

// Запустить генерацию
void OnStart() {
	cli();                                     // Глобальный запрет прерываний
	ppm    = tmp;                              //                        
	TIMSK1 = B00000001;                        // Разрешение прерывания от таймера
	TCCR1A = B10100011;                        // FAST PWM MODE 15
	TCCR1B = B00011001;                        // Предделитель 8(запуск таймера)
	TCCR1C = B00000000;
	OCR1A  = ppm.chanel[0];                    // Длительность первого импульса c паузой
	OCR1B  = ppm.chanel[0] - ppm.pause;        // Длительность первого импульса без паузы
	current = 1;
	sei();                                     // Глобальное разрешение прерываний
}

// Остановить генерацию
void OnStop() {
	cli();                                     // Глобальный запрет прерываний
	TCCR1B = 0;                                // Останавливаем счетчик
	TIMSK1 = 0;                                // Отключаем прерывание
	sei();                                     // Глобальное разрешение прерываний
}

// Установить количество каналов
void OnSetChannelsCount() {
	tmp.count = cmdMessenger.readInt16Arg();
}

// Установить длительности паузы, мксек
void OnSetPause() {
	tmp.pause = cmdMessenger.readInt16Arg();
	for (byte i = 0; i < tmp.count; i++) tmp.chanel[i] = max(tmp.chanel[i], tmp.pause);
}

// Установить длительности канала, мксек
void OnSetChannel() {
	byte chanel = cmdMessenger.readInt16Arg();
	word value  = cmdMessenger.readInt16Arg();
	if (chanel < MAX_CHANEL_COUNT) {
		tmp.chanel[chanel] = value;
	}
}

// Установить длительность синхроимпульса, мксек
void OnSetSync() {
	tmp.sync = cmdMessenger.readInt32Arg();
}  

// Запрос частоты ШИМ
void OnGetFreaquency() {
	cmdMessenger.sendCmd(kFreaquency, 16000000); // 16 МГц
}

