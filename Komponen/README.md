**Komponen yang Digunakan**

<img width="474" height="474" alt="Arduio UNO" src="https://github.com/user-attachments/assets/52f4a193-9515-472c-8225-019080f1e426" />

**Arduino UNO**

Arduino UNO berfungsi sebagai pusat kendali utama dalam rangkaian ini. Semua data dari sensor asap diproses oleh Arduino untuk menentukan apakah kondisi aman atau berbahaya. Arduino kemudian mengontrol output seperti LED, buzzer, dan LCD berdasarkan kondisi tersebut, serta menangani komunikasi UART (Serial) dan interrupt dari tombol reset agar sistem bisa merespons dengan cepat.

<img width="474" height="474" alt="MQ-2 Gas Sensor Module" src="https://github.com/user-attachments/assets/0bb91dd7-5e31-4ae4-a26d-961cac322b97" />

**Sensor Asap (MQ-2)**

Sensor asap digunakan untuk mendeteksi keberadaan gas atau asap di lingkungan sekitar. Pada rangkaian ini, sensor memberikan nilai analog ke Arduino yang kemudian dibandingkan dengan nilai threshold. Jika nilai tersebut melebihi batas, sistem akan menganggap kondisi berbahaya dan mengaktifkan alarm.

<img width="474" height="474" alt="i2c-lcd-display" src="https://github.com/user-attachments/assets/442c398b-4173-4853-bc89-cde861ac1626" />

**LCD 16x2 I2C**

LCD berfungsi sebagai media tampilan informasi bagi pengguna. Dalam rangkaian ini, LCD menampilkan status sistem seperti kondisi "aman" atau saat sistem sedang aktif. Penggunaan modul I2C memungkinkan komunikasi hanya dengan dua pin (SDA dan SCL), sehingga lebih efisien dan rapi dalam perakitan.

<img width="474" height="474" alt="OIP" src="https://github.com/user-attachments/assets/c5eabd2b-4048-443b-aa1d-7f206a9ea064" />

**Buzzer**

Buzzer digunakan sebagai indikator suara (alarm) ketika terdeteksi kondisi berbahaya. Pada rangkaian ini, buzzer menghasilkan suara sirine dengan frekuensi yang berubah-ubah menggunakan fungsi tone(), sehingga memberikan peringatan yang lebih jelas dan menarik perhatian pengguna.

<img width="474" height="474" alt="LED Merah" src="https://github.com/user-attachments/assets/08622533-69c0-46c4-bcf8-4e367d5ad882" />

**LED Merah**

LED merah berfungsi sebagai indikator visual untuk kondisi bahaya. Dalam rangkaian ini, LED merah akan menyala ketika sensor mendeteksi adanya asap di atas ambang batas, sehingga pengguna dapat langsung mengetahui adanya potensi bahaya meskipun tanpa suara.

<img width="474" height="474" alt="LED Hijau" src="https://github.com/user-attachments/assets/fa70a75e-909a-4218-81c2-4d654b8f59e2" />

**LED Hijau**

LED hijau berfungsi sebagai indikator kondisi aman. LED ini akan menyala ketika tidak ada asap yang terdeteksi atau nilai sensor masih di bawah threshold, sehingga memberikan informasi bahwa ruangan dalam keadaan normal.

<img width="474" height="474" alt="OIP (1)" src="https://github.com/user-attachments/assets/330578d7-f3fa-43c4-8218-eeaf1faa4911" />

**Push Button (Mute)**

Push button digunakan untuk mereset atau mematikan sementara alarm. Dalam rangkaian ini, tombol dihubungkan dengan interrupt sehingga saat ditekan, sistem langsung merespons tanpa harus menunggu proses loop selesai. Ini sangat penting agar alarm bisa dimatikan dengan cepat saat diperlukan.

<img width="474" height="474" alt="Resistor 220 Ohm" src="https://github.com/user-attachments/assets/47c30488-e552-44f2-82cf-e2757223e0ad" />

**Resistor**

Resistor digunakan untuk membatasi arus listrik, terutama pada LED, agar tidak terjadi kerusakan akibat arus berlebih. Dalam rangkaian ini, resistor memastikan LED dapat menyala dengan aman dan stabil.

<img width="474" height="281" alt="OIP (2)" src="https://github.com/user-attachments/assets/1bfe8395-bccf-4afd-aa47-879bd61fb313" />

**Breadboard**

Breadboard berfungsi sebagai tempat merangkai semua komponen tanpa perlu solder. Dalam proyek ini, breadboard memudahkan proses penyusunan dan pengujian rangkaian sebelum dibuat permanen.

<img width="474" height="474" alt="kabel-jumper-male-to-male-dupont-20cm-331193" src="https://github.com/user-attachments/assets/54c12215-9a4d-4708-965e-c59b1c7c5858" />

**Kabel Jumper**

Kabel jumper digunakan untuk menghubungkan antar komponen dalam rangkaian. Kabel ini memungkinkan aliran listrik dan sinyal dari Arduino ke komponen lain seperti sensor, LED, buzzer, dan LCD.
