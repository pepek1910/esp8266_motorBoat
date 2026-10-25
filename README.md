# ESP8266 Remote Monitor - Supabase edition v2

Projekt pod PlatformIO dla ESP8266 (NodeMCU) z:
- kolejką binarną w LittleFS,
- zapisem pomiarów offline,
- wysyłką HTTPS do Supabase po odzyskaniu Internetu,
- `timestamp_utc = null`, gdy NTP nie ustawił jeszcze czasu,
- polem `has_valid_time`,
- trwałym licznikiem `seq` zapisanym w LittleFS.

## Co uzupełnić
W pliku `include/app_config.h` ustaw:
- `WIFI_SSID`
- `WIFI_PASS`
- `SUPABASE_URL`
- `SUPABASE_ANON_KEY`
- `DEVICE_ID`

Przykład URL:
`https://YOUR_PROJECT_REF.supabase.co/rest/v1/measurements`

## SQL dla Supabase

```sql
create table if not exists public.measurements (
  id bigint generated always as identity primary key,
  device_id text not null,
  timestamp_utc timestamptz null,
  has_valid_time boolean not null default false,
  temp_c real,
  humidity real,
  weight_kg real,
  seq integer not null,
  created_at timestamptz not null default now()
);

grant usage on schema public to anon;
grant insert on table public.measurements to anon;

alter table public.measurements enable row level security;

drop policy if exists "anon_can_insert_measurements" on public.measurements;

create policy "anon_can_insert_measurements"
on public.measurements
for insert
to anon
with check (device_id = 'esp8266-01');
```

Jeśli tabela już istnieje, możesz ją dopasować tak:

```sql
alter table public.measurements alter column timestamp_utc drop not null;
alter table public.measurements add column if not exists has_valid_time boolean not null default false;
```

## Start
1. Otwórz folder projektu w VS Code z rozszerzeniem PlatformIO.
2. Uzupełnij `include/app_config.h`.
3. W Supabase uruchom SQL z sekcji powyżej.
4. Skompiluj projekt: `PlatformIO: Build`.
5. Wgraj firmware: `PlatformIO: Upload`.
6. Otwórz monitor portu: `PlatformIO: Monitor`.

## Uwagi
- Aby deep sleep wybudzał ESP8266 timerem, połącz GPIO16 z RST.
- `SensorService` zawiera obecnie dane przykładowe - podmień na DHT/HX711.
- `setInsecure()` w HTTPS upraszcza testy, ale w wersji docelowej warto rozważyć pinning certyfikatu.
- Dla telemetrii najlepiej trzymać czas w UTC.

## Podlaczenie
ESP8266, B32v4, Stab 5V, Supply 12V
GND wspolne
D1 -> IN3
D2 -> IN2
D5 -> IN1
D6 -> Servo Yellow

Supply (12v) -> Stab 5v (IN+)
Supply (GND) -> Stab 5V (IN-)
Supply (12v) -> B32v4 (IN+)
Supply (GND) -> B32v4 (IN-)

Stab (GND) -> Servo Brown
Stab (OUT+) -> Servo Red