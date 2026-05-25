**Komponen yang Digunakan**

![](media/image1.png){width="4.159148075240595in" height="2.6681321084864393in"}

**Arduino UNO**

Arduino UNO berfungsi sebagai pusat kendali utama dalam rangkaian ini. Semua data dari sensor asap diproses oleh Arduino untuk menentukan apakah kondisi aman atau berbahaya. Arduino kemudian mengontrol output seperti LED, buzzer, dan LCD berdasarkan kondisi tersebut, serta menangani komunikasi UART (Serial) dan interrupt dari tombol reset agar sistem bisa merespons dengan cepat.

![](media/image2.png){width="2.260653980752406in" height="2.260653980752406in"}

**Sensor Asap (MQ-2)**

Sensor asap digunakan untuk mendeteksi keberadaan gas atau asap di lingkungan sekitar. Pada rangkaian ini, sensor memberikan nilai analog ke Arduino yang kemudian dibandingkan dengan nilai threshold. Jika nilai tersebut melebihi batas, sistem akan menganggap kondisi berbahaya dan mengaktifkan alarm.

![](media/image3.jpeg){width="3.3479440069991253in" height="2.8255818022747157in"}

**LCD 16x2 I2C**

LCD berfungsi sebagai media tampilan informasi bagi pengguna. Dalam rangkaian ini, LCD menampilkan status sistem seperti kondisi "aman" atau saat sistem sedang aktif. Penggunaan modul I2C memungkinkan komunikasi hanya dengan dua pin (SDA dan SCL), sehingga lebih efisien dan rapi dalam perakitan.

![](media/image4.png){width="3.222182852143482in" height="2.407904636920385in"}

**Buzzer**

Buzzer digunakan sebagai indikator suara (alarm) ketika terdeteksi kondisi berbahaya. Pada rangkaian ini, buzzer menghasilkan suara sirine dengan frekuensi yang berubah-ubah menggunakan fungsi tone(), sehingga memberikan peringatan yang lebih jelas dan menarik perhatian pengguna.

![](media/image5.png){width="2.075971128608924in" height="2.075971128608924in"}

**LED Merah**

LED merah berfungsi sebagai indikator visual untuk kondisi bahaya. Dalam rangkaian ini, LED merah akan menyala ketika sensor mendeteksi adanya asap di atas ambang batas, sehingga pengguna dapat langsung mengetahui adanya potensi bahaya meskipun tanpa suara.

![](media/image6.png){width="1.9767443132108486in" height="1.9767443132108486in"}

**LED Hijau**

LED hijau berfungsi sebagai indikator kondisi aman. LED ini akan menyala ketika tidak ada asap yang terdeteksi atau nilai sensor masih di bawah threshold, sehingga memberikan informasi bahwa ruangan dalam keadaan normal.

![](media/image4.png){width="2.0273731408573927in" height="1.4109011373578302in"}

**Push Button (Mute)**

Push button digunakan untuk mereset atau mematikan sementara alarm. Dalam rangkaian ini, tombol dihubungkan dengan interrupt sehingga saat ditekan, sistem langsung merespons tanpa harus menunggu proses loop selesai. Ini sangat penting agar alarm bisa dimatikan dengan cepat saat diperlukan.

![](media/image7.png){width="1.8532436570428696in" height="0.4740890201224847in"}

**Resistor**

Resistor digunakan untuk membatasi arus listrik, terutama pada LED, agar tidak terjadi kerusakan akibat arus berlebih. Dalam rangkaian ini, resistor memastikan LED dapat menyala dengan aman dan stabil.

![](media/image8.png){width="3.518616579177603in" height="2.0859831583552055in"}

**Breadboard**

Breadboard berfungsi sebagai tempat merangkai semua komponen tanpa perlu solder. Dalam proyek ini, breadboard memudahkan proses penyusunan dan pengujian rangkaian sebelum dibuat permanen.

![](media/image9.jpeg){width="2.4135673665791777in" height="1.7093022747156605in"}

**Kabel Jumper**

Kabel jumper digunakan untuk menghubungkan antar komponen dalam rangkaian. Kabel ini memungkinkan aliran listrik dan sinyal dari Arduino ke komponen lain seperti sensor, LED, buzzer, dan LCD.
