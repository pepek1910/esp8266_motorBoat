#pragma once

#include <cstddef>
#include <cstdint>

namespace app {
constexpr char WIFI_SSID[] = "Piotr";
constexpr char WIFI_PASS[] = "BezpieczneHaslo!!";

constexpr char SUPABASE_URL[] = "https://pjdaezcmqorthrutlqod.supabase.co/rest/v1/measurements";
constexpr char SUPABASE_ANON_KEY[] = "sb_publishable_WO9tNZvrtavyuckY4ikpSw_Miimhl_c";
constexpr char DEVICE_ID[] = "esp8266-01";

constexpr char QUEUE_FILE[] = "/queue.bin";
constexpr char TEMP_FILE[]  = "/queue_tmp.bin";
constexpr char SEQ_FILE[]   = "/seq.txt";

constexpr unsigned long WIFI_TIMEOUT_MS = 15000;
constexpr unsigned long NTP_TIMEOUT_MS  = 10000;

constexpr std::size_t MAX_QUEUE_RECORDS = 1000;
constexpr uint32_t SLEEP_SECONDS = 10;
}
