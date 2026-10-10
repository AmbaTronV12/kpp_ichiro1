# Penjelasan Kode: Simulasi Striker ICHIRO

Dokumen ini berisi: (1) cara menjalankan animasi, (2) pembagian Level 1/2/3, (3) alur satu tick,
(4) penjelasan tiap file dan fungsi, (5) daftar Standard Library dan syntax C++ yang dipakai,
(6) cara membaca class diagram.

---------------------------------------------------------------------------------------------

## 1. Animasi: apa yang diubah

| File | Perubahan |
|---|---|
| `Config.hpp` / `Config.cpp` | Field baru `tickDelayMs` (default 500) dan key baru `tick_delay_ms` di `config.txt`. Nilai negatif ditolak. |
| `Simulator.hpp` / `Simulator.cpp` | `run(stepMode, tickDelayMs)`: tiap tick menunggu `std::this_thread::sleep_for(std::chrono::milliseconds(...))`. `render()` kini menyusun satu frame penuh di `std::ostringstream`, lalu mencetaknya sekali dengan `std::flush` supaya layar tidak berkedip. |
| `main.cpp` | Argumen ke-2 opsional untuk menimpa jeda dari terminal. |

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o soccer_sim
./soccer_sim                    # pakai config.txt, jeda 500 ms
./soccer_sim config.txt 200     # lebih cepat (200 ms per tick)
./soccer_sim config.txt 0       # tanpa jeda
```

Cara kerja jeda: di `run()`, sebelum `tick()` dipanggil, program tidur selama `tickDelayMs`.
Jika `step_mode = 1`, jeda diabaikan karena kamu menekan Enter sendiri.
Dengan skenario default, gol terjadi di tick 49, jadi pada 500 ms animasinya berlangsung sekitar 25 detik.
Saya sudah mengompilasi dan menjalankannya: hasilnya gol di tick 49, sama dengan README.

Catatan: `"\033[H\033[2J"` adalah ANSI escape code (kursor ke pojok kiri atas, lalu hapus layar).
Berjalan di terminal Linux/macOS dan Windows Terminal. Di `cmd.exe` lama mungkin tampil sebagai karakter aneh.

---------------------------------------------------------------------------------------------

## 2. Pembagian Level (sesuai PDF soal)

### Level 1: Core Simulation & OOP Basics
| Syarat di PDF | Dipenuhi oleh |
|---|---|
| Class `Field`, `Ball`, `Simulator` | `Field.hpp/.cpp`, `Ball.hpp/.cpp`, `Simulator.hpp/.cpp` |
| Abstract class `Robot`, atribut private/protected + getter/setter tervalidasi | `Robot.hpp/.cpp`: `position_`, `heading_`, `speed_` private. `setPosition`, `setHeading`, `setSpeed` memvalidasi dan melempar exception. `think()` adalah `= 0` (pure virtual), jadi `Robot` abstract. |
| `Striker` mewarisi `Robot` dan override `think()` | `Striker.hpp/.cpp`: `class Striker : public Robot`, `ActionCmd think() override` |
| DRY: jarak, bearing, normalisasi sudut di satu tempat | `namespace MathUtils` di `Types.hpp` (`distance`, `bearing`, `relativeBearing`, `normalizeAngle`). `Robot::distanceTo/bearingTo/relativeBearingTo` hanya membungkusnya. |

### Level 2: Sensor Mechanics & Exception Handling
| Syarat di PDF | Dipenuhi oleh |
|---|---|
| Robot HAS-A Sensor (composition) | `Robot` punya member `Sensor sensor_` (dibuat dan dihancurkan bersama robot). |
| Robot tidak boleh "cheat" membaca posisi bola | `Robot::sense(const Ball&)` memanggil `Sensor::capture`; hasilnya hanya `SensorData` (jarak dan sudut relatif). Posisi dunia diestimasi sendiri di `estimateBallWorldPosition`. |
| try-catch untuk aksi tidak valid | `InvalidActionException` (di `Types.hpp`) dilempar oleh `Simulator::executeAction`, `setPosition`, `setSpeed`, dst., dan ditangkap di `Simulator::tick()`. |

### Level 3: Extra Features (ketiganya dikerjakan)
| Fitur | Dipenuhi oleh |
|---|---|
| File konfigurasi | `Config.hpp/.cpp` membaca `config.txt` |
| State Pattern | `StrikerState` + `SearchState`, `ApproachState`, `AlignState`, `KickState` di `Striker.hpp/.cpp` |
| Unit test | `tests/test_math.cpp` disebut di README, **tetapi file itu tidak ada di file yang kamu kirim**. Pastikan file ini ada di proyek aslimu sebelum dikumpulkan. |

Kode di luar tiga level itu (BFS perencanaan tendangan, patroli waypoint, `tick_delay_ms`) adalah
pengembangan tambahan, bukan syarat soal.

---------------------------------------------------------------------------------------------

## 3. Alur satu tick (Sense, Think, Act)

`main()` memuat `Config`, membuat `Ball` dan `Striker`, lalu `Simulator::run()` memanggil `tick()` berulang:

```
Simulator::tick()
 ├─ ++tick_
 ├─ 1. SENSE : robot_->sense(ball_)
 │              └─ Sensor::capture(): bola di dalam segitiga kamera? -> SensorData
 ├─ 2. THINK : robot_->think()               (Striker::think)
 │              ├─ updateBallMemory()   estimasi posisi bola dari SensorData
 │              └─ state_->think(...)   state aktif memutuskan ActionCmd
 ├─ 3. ACT   : executeAction(cmd)  [try-catch InvalidActionException]
 │              WAIT / MOVE_FORWARD / ROTATE / KICK
 ├─ 4. ball_.update()                        fisika bola
 ├─ cek gol -> goal_ = true
 └─ render(...)                              gambar grid ke terminal
```

Fisika bola: ditendang dengan kecepatan 3 m/tick, turun 1 m/tick.
Tick pertama = 6 petak, kedua = 4 petak, ketiga = 2 petak, total 12 petak (6 m), lalu berhenti.
Bola berhenti jika menabrak batas lapangan atau masuk area gawang.

Kamera: segitiga tinggi 1,5 m (3 petak), alas 3,5 m. Lebar setengah segitiga naik linear:
di 1 petak depan ada 3 petak terlihat, di 2 petak ada 5, di 3 petak ada 7 (total 15 simbol `@`).

---------------------------------------------------------------------------------------------

## 4. Penjelasan per file dan fungsi

### Types.hpp (fondasi, semua file memakainya)
- `namespace Const`: semua konstanta (`CELL_SIZE`, `HALF_WIDTH`, `KICK_INITIAL_SPEED`, dst.). `constexpr` berarti dihitung saat kompilasi.
- `struct Vec2`: vektor 2D (`x`, `y`) dengan operator `+`, `-`, `* skalar`.
- `namespace MathUtils` (semua `inline`, boleh didefinisikan di header):
  - `toRadians/toDegrees`: konversi sudut.
  - `nearlyEqual(a,b,eps)`: bandingkan double dengan toleransi (jangan pakai `==` untuk double).
  - `distance(a,b)`: jarak Euclidean pakai `std::hypot`.
  - `normalizeAngle(deg)`: ke rentang (-180, 180].
  - `bearing(from,to)`: sudut global pakai `std::atan2`.
  - `relativeBearing`: bearing dikurangi arah hadap.
  - `unitVector(deg)`: (cos, sin).
  - `gridDirection(deg)`: unit vector dibulatkan ke -1/0/1, jadi 45 derajat menjadi diagonal (1,1).
- `enum class ActionType` dan `struct ActionCmd`: perintah dari Robot ke Simulator. Ada factory `wait()`, `move()`, `rotate()`, `kick()` dan `toString()` untuk tampilan.
- `InvalidActionException`: turunan `std::runtime_error`. Baris `using std::runtime_error::runtime_error;` mewarisi konstruktornya.

### Field.cpp (geometri lapangan, semua method `static`)
- `inBounds(p)`: `|x| < 4.5` dan `|y| < 3.0`.
- `isInGoal(p)`: kolom paling kanan (`x > 4.0`) dan `|y| <= 1.5`.
- `cellCenter(col,row)`: indeks petak (0..17, 0..11) menjadi koordinat pusat petak.
- `snapToCell(p)`: kebalikannya: bulatkan koordinat ke pusat petak, `std::clamp` agar tidak keluar indeks.

### Ball.cpp
- Konstruktor memanggil `setPosition` (tervalidasi, lempar exception jika di luar lapangan).
- `kick(gridDir)`: validasi arah (komponen -1/0/1, bukan (0,0)), simpan arah, set kecepatan 3.
- `update()`: tiap tick bola maju `round(speed/0.5)` petak satu per satu. Berhenti jika petak berikutnya di luar lapangan atau masuk gawang. Setelah bergerak, kecepatan dikurangi 1.

### Sensor.cpp (Level 2)
- `toLocal`: ubah titik dunia ke koordinat lokal robot (`forward` = depan, `left` = kiri) dengan rotasi.
- `isInView`: titik terlihat jika `0 < forward <= 1.5` dan `|left| <= forward * (3.5/2) / 1.5`.
- `capture`: jika bola terlihat, isi `SensorData` (jarak = `hypot(f,l)`, sudut = `atan2(l,f)`).

### Robot.cpp
- Konstruktor memakai setter, jadi validasi berlaku dari awal.
- `sense`: menyimpan hasil `sensor_.capture` ke `sensorData_`.
- `setPosition/setHeading/setSpeed`: setter tervalidasi (lempar `InvalidActionException`).
- `frontCell`: petak tepat di depan robot (syarat menendang).
- `distanceTo/bearingTo/relativeBearingTo/normalizeAngle`: pembungkus `MathUtils` (DRY).
- `estimateBallWorldPosition`: posisi robot + vektor (arah hadap + sudut bola) x jarak, lalu `snapToCell`.
- `navigateTo(target, avoid)`: jalan 4 arah ke target. Sumbu dengan sisa jarak terbesar didahulukan. Jika terhalang bola (`avoid`), geser ke samping. Mengembalikan `rotate` jika arah belum sesuai, `move(0.5)` jika sudah, `wait` jika sampai.

### Simulator.cpp
- `directionName`: fungsi helper di anonymous namespace (hanya terlihat di file ini).
- Konstruktor: validasi robot tidak null dan `maxTicks > 0` (`std::invalid_argument`).
- `run`: render awal, lalu loop `tick()` (dengan jeda animasi atau menunggu Enter) sampai selesai, lalu cetak hasil.
- `tick`: alur Sense, Think, Act, fisika, render (lihat bagian 3).
- `executeAction`: validasi tiap aksi: kecepatan maksimal 0,5, langkah tepat 1 petak, tidak keluar lapangan, tidak menabrak bola, rotasi kelipatan 90 (maksimal 180), tendangan hanya jika bola tepat di `frontCell`, offset tendangan -1/0/+1.
- `render`: gambar grid. Prioritas simbol: `.` lalu `#` lalu `@` lalu `O` lalu `R` (yang belakangan menimpa).

### Config.cpp (Level 3)
- Membuka file dengan `std::ifstream`, membaca baris per baris, membuang komentar (`#`), mengganti `=` jadi spasi, memecah `key value` dengan `istringstream`, mengubah ke angka dengan `stod`.
- Key tidak dikenal, nilai kosong, atau nilai bukan angka melempar `std::runtime_error` lengkap dengan nomor baris. `main` menangkapnya dan memakai konfigurasi default.
- Akhirnya posisi di-`snapToCell`.

### main.cpp
Membaca path config (argumen 1, default `config.txt`), jeda opsional (argumen 2), cek striker dan bola tidak di petak sama, lalu membangun objek dan menjalankan `Simulator`.

### Striker.cpp (Level 3: State Pattern)
- Konstruktor: membuat 6 titik patroli (waypoint) yang menutup seluruh lapangan.
- `makeState(id)`: factory yang membuat objek state (`std::make_unique`).
- `requestState(id)`: hanya mencatat state berikutnya di `pending_`.
- `think()`: loop paling banyak 8 kali. State aktif dipanggil. Jika mengembalikan `nullopt` (tidak ada aksi, hanya pindah state), state langsung diganti dan state baru dievaluasi di tick yang sama. Pergantian dilakukan setelah `think` state selesai supaya objek state tidak dihapus saat sedang berjalan.
- `updateBallMemory`: jika kamera melihat bola, perbarui `ballPos_`; jika posisinya berubah, rencana lama dibatalkan.
- `simulateKick(start, dir)`: "fisika dalam kepala": membuat `Ball` sementara, menendang, menjalankan `update()` sampai berhenti, mengembalikan posisi akhir.
- `kicksToGoal(start)`: BFS (antrian) untuk jumlah tendangan minimum dari titik itu sampai gol; 99 = mustahil.
- `computePlan()`: coba 4 arah hadap x 3 offset (12 kombinasi), pilih biaya terkecil: utamakan jumlah tendangan paling sedikit (100 per tendangan), lalu jarak jalan robot, lalu sudut putar, lalu tendangan lurus.
- `SearchState`: tunggu bola berhenti (cooldown), jika bola diketahui pindah ke Approach, putar 3x90 derajat untuk scan 4 arah, jalan ke waypoint berikutnya.
- `ApproachState`: hitung rencana, jalan ke posisi tembak tanpa menginjak bola; sampai maka pindah ke Align.
- `AlignState`: putar ke arah rencana, pastikan kamera melihat bola tepat 1 petak di depan, lalu pindah ke Kick; jika tidak, kembali Search.
- `KickState`: kirim `KICK(offset)`, reset memori bola, set cooldown 3 tick, kembali ke Search.

---------------------------------------------------------------------------------------------

## 5. Standard Library dan syntax yang dipakai

### Header dan isinya
| Header | Yang dipakai | Fungsinya di proyek ini |
|---|---|---|
| `<iostream>` | `std::cout`, `std::cerr`, `std::cin.get()` | Output grid, pesan error, menunggu Enter (step mode) |
| `<iomanip>` | `std::setprecision`, `std::fixed` | Menampilkan 2 desimal (`-3.75`) |
| `<sstream>` | `std::ostringstream`, `std::istringstream` | Menyusun teks di memori (render frame, `toString`) dan memecah baris config |
| `<fstream>` | `std::ifstream` | Membuka dan membaca `config.txt` |
| `<string>` | `std::string`, `std::to_string`, `std::getline`, `std::stod`, `find`, `erase`, `npos` | Teks, konversi angka, parsing config |
| `<cmath>` | `fabs`, `hypot`, `atan2`, `cos`, `sin`, `fmod`, `floor`, `round`, `lround`, `isfinite` | Seluruh matematika: jarak, sudut, pembulatan ke petak |
| `<algorithm>` | `std::clamp`, `std::max` | Membatasi indeks petak; jeda tidak negatif |
| `<vector>` | `std::vector` | Daftar waypoint, kandidat arah, penanda `seen` BFS |
| `<queue>` | `std::queue` | Antrian BFS di `kicksToGoal` |
| `<utility>` | `std::pair`, `std::move` | Pasangan (posisi, kedalaman); memindahkan kepemilikan `unique_ptr` |
| `<memory>` | `std::unique_ptr`, `std::make_unique` | Robot dan state dimiliki otomatis, tanpa `new`/`delete` |
| `<optional>` | `std::optional`, `std::nullopt`, `.reset()` | "Mungkin ada aksi, mungkin tidak" dan "state berikutnya mungkin belum ada" |
| `<limits>` | `std::numeric_limits<double>::max()` | Nilai awal "biaya terburuk" di `computePlan` |
| `<stdexcept>` | `std::runtime_error`, `std::invalid_argument`, `std::exception` | Jenis exception dan penangkap umum |
| `<cstdlib>` | `std::abs` (int) | Nilai mutlak offset tendangan |
| `<cstddef>` | `std::size_t` | Tipe indeks waypoint |
| `<chrono>` dan `<thread>` | `std::chrono::milliseconds`, `std::this_thread::sleep_for` | **Jeda animasi (baru)** |

### Syntax C++ penting
| Syntax | Contoh di kode | Arti |
|---|---|---|
| `const T&` | `const Vec2& p` | Dioper tanpa disalin dan tidak boleh diubah |
| `const` di akhir method | `double getSpeed() const` | Method tidak mengubah objek |
| `explicit` | `explicit Ball(...)` | Mencegah konversi implisit ke `Ball` |
| `virtual ... = 0` | `virtual ActionCmd think() = 0;` | Pure virtual, membuat class abstract |
| `override` | `ActionCmd think() override` | Kompiler memastikan benar-benar menimpa method induk |
| `virtual ~Robot() = default` | | Destruktor virtual, agar `delete` lewat pointer induk aman |
| `static` method | `Field::inBounds` | Dipanggil tanpa objek |
| `static_cast<int>(x)` | | Konversi tipe eksplisit dan aman |
| `enum class` | `StateId::SEARCH` | Enum bertipe kuat, tidak bocor ke namespace |
| `friend class` | di `Striker` | Mengizinkan class state mengakses member private Striker |
| Lambda `[&](double h){...}` | `isFree`, `key` | Fungsi kecil tanpa nama; `[&]` menangkap variabel sekitar |
| Range-for `for (double h : {...})` | | Iterasi daftar nilai |
| Structured binding `const auto [pos, depth] = q.front();` | `kicksToGoal` | Membongkar pair jadi dua variabel |
| Initializer list `: base_(base)` | konstruktor | Inisialisasi member sebelum body konstruktor |
| `try { } catch (const X& e) { }` | `tick()`, `main()` | Penanganan exception |
| `constexpr`, `inline` | `Types.hpp` | Konstanta compile-time; fungsi boleh di header |
| Anonymous `namespace { }` | `Simulator.cpp` | Fungsi hanya terlihat di file itu |
| `#pragma once` | semua header | Mencegah header ter-include dua kali |
| `a ? b : c` | `render` | Operator ternary |
| `std::move(robot)` | `main` | Memindahkan kepemilikan `unique_ptr` (tidak bisa disalin) |

---------------------------------------------------------------------------------------------

## 6. Membaca class diagram

Arti garis:
- `A *-- B` **komposisi**: A memiliki B, B hidup dan mati bersama A.
- `A o-- B` **agregasi**: A memegang B, tetapi hubungannya lebih longgar.
- `A <|-- B` **pewarisan**: B adalah turunan A.
- `A ..> B` **dependensi**: A memakai B (parameter atau pemanggilan), tidak menyimpannya.

Alur dari atas ke bawah:

1. **Simulator** adalah pusat. Ia memiliki `Field` dan `Ball` (`*--`), dan memegang `Robot` lewat `unique_ptr` (`o--`, sebenarnya kepemilikan penuh, jadi boleh dianggap komposisi juga). Ia menangkap `InvalidActionException` (`..>`, try-catch).
2. **Robot** adalah class abstract. Ia memiliki `Sensor` (`*--`, **HAS-A**, ini bukti Level 2) dan memakai `SensorData` serta `MathUtils`.
3. **Sensor** hanya *mengamati* `Ball` (`..>`). Inilah satu-satunya jalur Robot mengetahui bola, jadi tidak ada "cheat".
4. **Striker** mewarisi `Robot` (`<|--`, Level 1) dan override `think()`.
5. **Striker** memegang satu `StrikerState` aktif (`o--`). Empat state konkret (`SearchState`, `ApproachState`, `AlignState`, `KickState`) mewarisi `StrikerState` (Level 3: State Pattern).
6. Perpindahan state ada di diagram state di README: Search, Approach, Align, Kick, kembali ke Search (Align juga bisa kembali ke Search jika bola tidak ada di depan).

Satu tick dalam bahasa diagram: `Simulator` memanggil `Robot.sense(Ball)` lalu `Robot.think()`.
`Striker` meneruskan ke `StrikerState.think()`, yang mengembalikan `ActionCmd`.
`Simulator.executeAction()` memvalidasi (melempar `InvalidActionException` jika salah), mengubah posisi robot atau menendang `Ball`, lalu `Ball.update()` menggerakkan bola.

Catatan kecil untuk diagram README: kelas `Config`, `ActionCmd`, `ShotPlan`, dan `StateId` belum digambar.
Jika dosen/asisten meminta diagram lengkap, tambahkan keempatnya
(`main` memakai `Config`, `Striker` memiliki `ShotPlan`, `Robot` dan `Simulator` memakai `ActionCmd`).
Di `Simulator` ada member `Field field_` yang tidak dipakai (semua method `Field` bersifat `static`); aman dibiarkan atau dihapus.

---------------------------------------------------------------------------------------------

## 7. Pernyataan penggunaan AI

README yang kamu kirim sudah menyatakan bantuan AI, tetapi tertulis "isi/ubah sesuai kenyataan".
Pastikan bagian itu kamu sesuaikan, dan tambahkan bahwa fitur jeda animasi (`tick_delay_ms`, perubahan
`Simulator::run/render`) ditambahkan dengan bantuan AI.
