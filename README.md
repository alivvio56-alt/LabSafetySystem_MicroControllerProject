# Laboratorium Safety System

## Identitas Kelompok

| Keterangan   | Informasi                  |
| ------------ | -------------------------- |
| Kelas        | A                          |
| Kelompok     | 5                          |
| Topik Proyek | Laboratorium Safety System |

## Anggota Kelompok

| No | Nama                       | NIM       |
| -- | -------------------------- | --------- |
| 1  | Hammed Jastiko Apuranam    | H1H024030 |
| 2  | Ardhis Alivio Rajendra     | H1H024031 |
| 3  | Pandu Adi Utama            | H1H024032 |
| 4  | Wisnu Satya Herlambang     | H1H024033 |
| 5  | Alfian Iskandar Zulkarnain | H1H024034 |

## Deskripsi Proyek

**Laboratorium Safety System** merupakan sistem pemantauan keselamatan laboratorium yang dirancang untuk mendeteksi kondisi berbahaya akibat peningkatan kadar asap atau gas di lingkungan laboratorium. Sistem menggunakan sensor analog sebagai input utama untuk mengukur konsentrasi asap dan diproses oleh **Arduino Uno R4 Minima** dengan pendekatan **Real-Time Operating System (RTOS)** sehingga beberapa tugas dapat berjalan secara bersamaan (*multitasking*).

Apabila nilai sensor melebihi ambang batas (*threshold*), sistem akan mengaktifkan **buzzer** dan **LED merah** sebagai indikator bahaya. Pada kondisi normal, **LED hijau** akan menyala sebagai indikator aman. Informasi status sistem ditampilkan secara *real-time* pada **LCD I2C**.

Selain itu, sistem mendukung konfigurasi melalui komunikasi **UART (Serial Monitor)** untuk mengubah nilai *threshold*, mengaktifkan sistem, maupun mengunci sistem. Pada tahap pengembangan selanjutnya, sistem direncanakan terintegrasi dengan **ESP32** untuk menambahkan fitur **Internet of Things (IoT)** sehingga kondisi laboratorium dapat dipantau dari jarak jauh melalui jaringan internet.
