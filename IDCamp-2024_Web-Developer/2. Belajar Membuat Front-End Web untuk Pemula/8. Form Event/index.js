// Mendapatkan elemen form dengan ID 'formDataDiri' dan menyimpannya dalam variabel submitAction
const submitAction = document.getElementById('formDataDiri');

// Menambahkan event listener pada elemen form untuk menangani event 'submit'
submitAction.addEventListener('submit', function (event) {
  // Mendapatkan nilai input dari elemen dengan ID 'inputNama'
  const inputNama = document.getElementById('inputNama').value;
  // Mendapatkan nilai input dari elemen dengan ID 'inputDomisili'
  const inputDomisili = document.getElementById('inputDomisili').value;
  // Membuat pesan personalisasi berdasarkan input nama dan domisili
  const hiddenMessage = `Halo, ${inputNama}. Bagaimana cuacanya di ${inputDomisili}?`;

  // Menampilkan pesan personalisasi pada elemen dengan ID 'messageAfterSubmit'
  document.getElementById('messageAfterSubmit').innerText = hiddenMessage;
  // Mencegah form dari perilaku default (reload halaman) setelah submit
  event.preventDefault();
});
