var connection = new WebSocket('ws://' + location.hostname + '/ws', ['arduino']);

connection.onopen = function () {
	connection.send('Received from Client');
	console.log('Connected');
};

connection.onerror = function (error) {
  console.log('WebSocket Error', error);
};

connection.onmessage = function (e) {
    console.log('Received from server: ', e.data);
    processReceived(e.data);
};

connection.onclose = function () {
  console.log('WebSocket connection closed');
};

function processReceived(data)
{
	json = JSON.parse(data)
	if(json.command == 'updateGPIO')
	{
		updateGPIOData(json.id, json.status);
	}
}

function sendGPIO(id, status)
{
	let data = {
		command : "setOnOff",
		id: id,
		status: status
	}

  let json = JSON.stringify(data);
  connection.send(json);
}

function sendPWM(id, pwm)
{
	updateSliderText(id, pwm);

	let data = {
		command : "setPWM",
		id: id,
		pwm: pwm
	}

  let json = JSON.stringify(data);
  connection.send(json);
}

function sendAction(id)
{
	let data = {
		command : "doAction",
		id: id,
	}

  let json = JSON.stringify(data);
  connection.send(json);
}

function updateGPIOData(id, status)
{
	if(status == 'ON')
	{
    document.getElementById(id).classList.add('on');
		document.getElementById(id).classList.remove('off');
	}
	else if (status == 'OFF')
    {
		document.getElementById(id).classList.remove('off');
    document.getElementById(id).classList.add('on');
	}
  else{
    document.getElementById(id).innerHTML = status;
  }
}

function updateSliderText(id, value) {
	document.getElementById('slider-pwm-' + id).value = value; 
	document.getElementById('slider-text-pwm-'+ id).value = value; 
  }

$(function() {
    // Side Bar Toggle
    $('.hide-sidebar').click(function() {
        $('#sidebar').hide('fast', function() {
            $('#content').removeClass('span9');
            $('#content').addClass('span12');
            $('.hide-sidebar').hide();
            $('.show-sidebar').show();
        });
    });

    $('.show-sidebar').click(function() {
        $('#content').removeClass('span12');
        $('#content').addClass('span9');
        $('.show-sidebar').hide();
        $('.hide-sidebar').show();
        $('#sidebar').show('fast');
    });
});



//Restablecer
function restablecer() {

    Swal.fire({
        title: 'Restablecer!',
        text: ' ¿Está seguro de restablecer el equipo?',
        icon: 'warning',
        showCancelButton: true,
        confirmButtonColor: '#3085d6',
        cancelButtonColor: '#d33',
        confirmButtonText: 'Si, restablecer',
        cancelButtonText: 'Cancelar',
        reverseButtons: true
    }).then((result) => {
        if (result.isConfirmed) {
            window.location = "/reconfig";
        } else if (
            result.dismiss === Swal.DismissReason.cancel
        ) {
            history.back();
        }
    })

}

