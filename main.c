#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <curl/curl.h>

#define NS_PER_MS 1000000

// === إعدادات تليجرام (ضع بياناتك هنا) ===
#define TELEGRAM_TOKEN "YOUR_BOT_TOKEN"
#define CHAT_ID "YOUR_CHAT_ID"

// === إعدادات المحاكاة للسوق ===
#define INITIAL_PRICE 65000.0
#define BUY_AT 64900.0   // نقطة الشراء
#define SELL_AT 65100.0  // نقطة البيع

// === هيكل المحفظة والسوق ===
typedef struct {
    double price;
    int signal;
    int last_state;
    
    // بيانات المحفظة (Wallet)
    double balance_usd;
    double balance_btc;
    double entry_price; // سعر الدخول للحساب الربح
    
    pthread_mutex_t lock;
} trading_engine_t;

trading_engine_t engine = {
    .price = INITIAL_PRICE,
    .signal = 0,
    .last_state = 0,
    .balance_usd = 1000.0, // نبدأ بـ 1000 دولار وهمي
    .balance_btc = 0.0,
    .entry_price = 0.0,
    .lock = PTHREAD_MUTEX_INITIALIZER
};

// === دالة إرسال التنبيهات مع تفاصيل الصفقة ===
void send_telegram(const char* text) {
    CURL *curl;
    char url[1024];
    curl = curl_easy_init();
    if(curl) {
        char *encoded_text = curl_easy_escape(curl, text, 0);
        snprintf(url, sizeof(url), "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s", 
                 TELEGRAM_TOKEN, CHAT_ID, encoded_text);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);
        curl_easy_perform(curl);
        curl_free(encoded_text);
        curl_easy_cleanup(curl);
    }
}

void add_ms(struct timespec *t, long ms) {
    t->tv_nsec += ms * NS_PER_MS;
    while (t->tv_nsec >= 1000000000) {
        t->tv_sec++;
        t->tv_nsec -= 1000000000;
    }
}

// === 1. محاكي حركة السعر (Market) ===
void* market_task(void* arg) {
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    while (1) {
        add_ms(&next, 100);
        pthread_mutex_lock(&engine.lock);
        // تقلب بسيط يحاكي سوق البيتكوين
        engine.price += ((rand() % 400) - 200) / 10.0; 
        pthread_mutex_unlock(&engine.lock);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
}

// === 2. المحلل واتخاذ القرار (Analysis) ===
void* analysis_task(void* arg) {
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    while (1) {
        add_ms(&next, 50);
        pthread_mutex_lock(&engine.lock);
        
        if (engine.price <= BUY_AT && engine.balance_usd > 0) engine.signal = 1;
        else if (engine.price >= SELL_AT && engine.balance_btc > 0) engine.signal = -1;
        else engine.signal = 0;
        
        pthread_mutex_unlock(&engine.lock);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
}

// === 3. منفذ الصفقات (Trade Executor) ===
void* execution_task(void* arg) {
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    while (1) {
        add_ms(&next, 20);
        pthread_mutex_lock(&engine.lock);

        if (engine.signal != 0 && engine.signal != engine.last_state) {
            char report[512];
            if (engine.signal == 1) { // عملية شراء
                engine.balance_btc = engine.balance_usd / engine.price;
                engine.entry_price = engine.price;
                engine.balance_usd = 0;
                sprintf(report, "🔵 BUY ORDER\nPrice: %.2f $\nQty: %.5f BTC\nStatus: Executed ✅", engine.price, engine.balance_btc);
            } 
            else if (engine.signal == -1) { // عملية بيع
                double payout = engine.balance_btc * engine.price;
                double profit = payout - (engine.balance_btc * engine.entry_price);
                engine.balance_usd = payout;
                engine.balance_btc = 0;
                sprintf(report, "🔴 SELL ORDER\nPrice: %.2f $\nProfit: %+.2f $\nBalance: %.2f $", engine.price, profit, engine.balance_usd);
            }
            
            printf("\n--- TRADE EXECUTED ---\n%s\n----------------------\n", report);
            send_telegram(report);
            engine.last_state = engine.signal;
        }
        
        pthread_mutex_unlock(&engine.lock);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
}

int main() {
    srand(time(NULL));
    curl_global_init(CURL_GLOBAL_ALL);
    pthread_t t1, t2, t3;
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    param.sched_priority = 10;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&t1, &attr, market_task, NULL);

    param.sched_priority = 20;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&t2, &attr, analysis_task, NULL);

    param.sched_priority = 50;
    pthread_attr_setschedparam(&attr, &param);
    pthread_create(&t3, &attr, execution_task, NULL);

    printf("HFT Simulator Running... Monitoring BTC/USD\n");
    printf("Initial Balance: %.2f USD\n", engine.balance_usd);

    pthread_join(t1, NULL); pthread_join(t2, NULL); pthread_join(t3, NULL);
    curl_global_cleanup();
    return 0;
}