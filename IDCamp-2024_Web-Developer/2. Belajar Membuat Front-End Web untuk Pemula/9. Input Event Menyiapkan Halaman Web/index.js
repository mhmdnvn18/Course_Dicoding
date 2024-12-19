// Menambahkan event listener ke dokumen yang akan dijalankan setelah semua konten DOM dimuat
document.addEventListener('DOMContentLoaded', function () {
    // Mengambil nilai atribut maxLength dari elemen input dengan ID 'inputNama'
    const inputMaxLengthOnLoad = document.getElementById('inputNama').maxLength;
    // Mengatur teks di elemen dengan ID 'sisaKarakter' untuk menampilkan jumlah maksimal karakter
    document.getElementById('sisaKarakter').innerText = inputMaxLengthOnLoad;
  });
  