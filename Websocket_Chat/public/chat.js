var name= prompt("What's your name?");
while(name==""){
    name=prompt("You have to choose a name. \n What's your name?")
}

// Query DOM
var messaggio = document.getElementById('messaggio'),
   sender = document.getElementById('sender'),
   btn = document.getElementById('send'),
   output = document.getElementById('output'),
   feedback = document.getElementById('feedback');

sender.innerHTML=name;  
sender.value=name;

// Invio richiesta di connessione al server
var webSocket = io.connect();

messaggio.addEventListener('keydown', function(){
    webSocket.emit('typing', {
        sender: sender.value
    });
});

// Listen for events
btn.addEventListener('click', function(){
   if (messaggio.value!="" ){    
 	   webSocket.emit('messaggio', {
        messaggio: messaggio.value,
 	   sender: sender.value,
 	   });
 	   messaggio.value = "";
   }
});

webSocket.on('aggiornamento', function(data){
   feedback.innerHTML = '';
   output.innerHTML += '<p><strong>' + data.sender + ': </strong>' + data.messaggio + '</p>';
});

webSocket.on('typing', function(data){
   feedback.innerHTML = '<p><em>' + data.sender + ' sta scrivendo...</em></p>';
});

