# **EKLAMON PROJECT**
*nanti insert gambar setelah bikin repository github*


## **BRIEF EXPLANATION ALAT**
**Eklamon** (jam tangan untuk **memprediksi tekanan darah**) memanfaatkan **sensor MAX30102** dan **Random Forest** untuk mengolah hasil pembacaan sensor (yaitu IR) sebagai inovasi pengukuran tekanan darah *cuffless* (tanpa manset), memanfaatkan jaringan WiFi untuk *data storing* dan komputasi *machine learning* ke *remote* ssh kampus.

**Komponen:**
1. Mikrokontroler Wemos Lolin32 Lite
2. Sensor MAX30102
3. OLED 0.96 inch
4. Baterai LiPo 3.7V 400mAh
5. Push button (untuk trigger start pemeriksaan)
6. Slide switch (sebagai on/off jam tangan)


## **DOKUMENTASI FILE**
Project ini berisi 7 file utama, yaitu:
- **Readme**:
Sebagai *brief explanation* memahami cara kerja alat dan alur program.
- **Folder pasien**:
(1x pemeriksaan terdiri dari 10 menit, di mana waktu dipartisi setiap 1 menit, sehingga 1x pemeriksaan terdiri dari 10 csv).
- **serial.ino**:
Program untuk Arduino IDE agar VSCode dapat dijalankan dengan membaca keluaran *serial monitor*.
- **a-data-storing**:
Mengimpor dan/atau mengekspor data hasil pembacaan sensor MAX30102 ke *server* ssh.
- **b-preprocessing.ipynb**:
Visualisasi data sebelum dan sesudah *filtering* menggunakan *low-pass filter*, serta mengekspor data ke ssh.
- **c-training.ipynb**:
Melakukan *training data* dari dataset yang sudah di-*cleaning* menggunakan Random Forest.
- **d-quantizing.ipynb**:
Mengkuantisasi (mengompres) model agar bisa di-*deploy* ke mikrokontroler sepenuhnya.

## **WIRING DIAGRAM**

## **3D DESIGN**
