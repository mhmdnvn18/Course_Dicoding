// Menunggu sampai semua konten DOM dimuat sebelum menjalankan script
document.addEventListener('DOMContentLoaded', function () {
    // Mengambil nilai maksimum karakter yang diizinkan pada input 'inputNama'
    const inputMaxLengthOnLoad = document.getElementById('inputNama').maxLength;
    // Menampilkan nilai maksimum karakter ke elemen dengan id 'sisaKarakter'
    document.getElementById('sisaKarakter').innerText = inputMaxLengthOnLoad;
  
    // Menambahkan event listener untuk mendeteksi perubahan pada input 'inputNama'
    document.getElementById('inputNama').addEventListener('input', function () {
      // Menghitung jumlah karakter yang telah diketik oleh pengguna
      const jumlahKarakterDiketik = document.getElementById('inputNama').value.length;
      // Mendapatkan jumlah karakter maksimal yang diizinkan
      const jumlahKarakterMaksimal = document.getElementById('inputNama').maxLength;
  
      // Logging jumlah karakter yang diketik dan maksimal ke konsol
      console.log('jumlahKarakterDiketik: ', jumlahKarakterDiketik);
      console.log('jumlahKarakterMaksimal: ', jumlahKarakterMaksimal);
  
      // Menghitung sisa karakter yang dapat diketik
      const sisaKarakterUpdate = jumlahKarakterMaksimal - jumlahKarakterDiketik;
      // Menampilkan sisa karakter pada elemen dengan id 'sisaKarakter'
      document.getElementById('sisaKarakter').innerText = sisaKarakterUpdate.toString();
  
      // Memberikan pesan khusus jika batas maksimal tercapai
      if (sisaKarakterUpdate === 0) {
        document.getElementById('sisaKarakter').innerText = 'Batas maksimal tercapai!';
      }
      // Mengubah warna teks menjadi merah jika sisa karakter kurang dari atau sama dengan 5
      else if (sisaKarakterUpdate <= 5) {
        document.getElementById('notifikasiSisaKarakter').style.color = 'red';
      }
      // Mengembalikan warna teks menjadi hitam jika sisa karakter lebih dari 5
      else {
        document.getElementById('notifikasiSisaKarakter').style.color = 'black';
      }
    });
  
    // Menambahkan event listener untuk mendeteksi saat input 'inputNama' mendapatkan fokus
    document.getElementById('inputNama').addEventListener('focus', function () {
      // Logging ke konsol bahwa elemen 'inputNama' sedang fokus
      console.log('inputNama: focus');
      // Menampilkan elemen notifikasi sisa karakter dengan mengubah visibilitasnya menjadi terlihat
      document.getElementById('notifikasiSisaKarakter').style.visibility = 'visible';
    });
  
    // Menambahkan event listener untuk mendeteksi saat input 'inputNama' kehilangan fokus
    document.getElementById('inputNama').addEventListener('blur', function () {
      // Logging ke konsol bahwa elemen 'inputNama' kehilangan fokus
      console.log('inputNama: blur');
      // Menyembunyikan elemen notifikasi sisa karakter dengan mengubah visibilitasnya menjadi tersembunyi
      document.getElementById('notifikasiSisaKarakter').style.visibility = 'hidden';
    });
  });
  