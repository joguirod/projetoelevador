//const socket = new WebSocket("wss://contador-de-pessoas.herokuapp.com/");
let socket = new WebSocket("ws://192.168.15.24:4000");

document.addEventListener("DOMContentLoaded", () => {
    let data = document.getElementById("data")
    let qtdPessoas = document.getElementById("contador")


    socket.onmessage = event => {
        const dados = event.data.split(",")
        data.innerHTML = dados[0]
        qtdPessoas.innerHTML = dados[1]
    }
})