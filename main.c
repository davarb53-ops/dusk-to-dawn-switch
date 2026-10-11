
 * کلید گرگ‌ومیش با ATmega8
 * LDR روی ADC0 (PC0)
 * رله روی PB0
 * LED نشانگر روی PB1
 * فرکانس کلاک: 8MHz (نوسانگر داخلی)
 */

#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

#define RELAY_PIN   PB0
#define LED_PIN     PB1

// آستانه‌ها (بین 0 تا 1023)
#define THRESHOLD_ON   300   // اگر ADC کمتر از این شد => تاریک => روشن کن
#define THRESHOLD_OFF  500   // اگر ADC بیشتر از این شد => روشن => خاموش کن
#define DELAY_COUNT    5     // چند بار متوالی شرط تأیید شود

// تابع خواندن ADC از کانال 0
uint16_t read_adc(void) {
    ADMUX = (1 << REFS0);              // ولتاژ مرجع = AVcc، کانال ADC0
    ADCSRA |= (1 << ADSC);             // شروع تبدیل
    while (ADCSRA & (1 << ADSC));      // انتظار تا پایان تبدیل
    return ADC;
}

int main(void) {
    // تنظیم رله و LED به عنوان خروجی
    DDRB |= (1 << RELAY_PIN) | (1 << LED_PIN);
    PORTB &= ~((1 << RELAY_PIN) | (1 << LED_PIN)); // خاموش در ابتدا

    // راه‌اندازی ADC
    ADMUX = (1 << REFS0);              // AVcc، کانال 0
    ADCSRA = (1 << ADEN) |             // فعال‌سازی ADC
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // تقسیم بر 128

    uint8_t state = 0;                 // 0 = خاموش (روشنایی)، 1 = روشن (تاریکی)
    uint8_t counter = 0;

    while (1) {
        uint16_t adc = read_adc();

        if (state == 0) {
            // الان خاموش است، ببین تاریک شده؟
            if (adc < THRESHOLD_ON) {
                counter++;
                if (counter >= DELAY_COUNT) {
                    state = 1;
                    PORTB |= (1 << RELAY_PIN) | (1 << LED_PIN); // روشن کن
                    counter = 0;
                }
            } else {
                counter = 0;
            }
        } else {
            // الان روشن است، ببین روشن شده؟
            if (adc > THRESHOLD_OFF) {
                counter++;
                if (counter >= DELAY_COUNT) {
                    state = 0;
                    PORTB &= ~((1 << RELAY_PIN) | (1 << LED_PIN)); // خاموش کن
                    counter = 0;
                }
            } else {
                counter = 0;
            }
        }

        _delay_ms(1000); // هر ۱ ثانیه یک بار بررسی کن
    }
}
