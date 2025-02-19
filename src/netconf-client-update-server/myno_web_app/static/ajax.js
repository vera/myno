var notificationDiv = document.getElementById('notification');
var errorDiv = document.getElementById('error');
var eventDiv = document.getElementById('events');

document.addEventListener("DOMContentLoaded", function() { 
	getDivs();
	updateValues();
});

function getDivs() {
	notificationDiv = document.getElementById('notification');
	errorDiv = document.getElementById('error');
	eventDiv = document.getElementById('events');
}

function updateValues() {
	// Gets data from "/ajax"-path, then calls updateUI
	var request = new XMLHttpRequest(); 

	request.onload = function() {
		var result = JSON.parse(request.responseText);
		updateUI(result);
	};

	request.open("GET", "/ajax", true);
	request.send();
}

function updateUI(data) {
	if(!errorDiv || !notificationDiv) {
		// Needed because some browsers ignore the "DOMContentLoaded" listener
		// or call it too early so we re-init the two variables
		getDivs();
	}

	// Update sensor values
	for (let key in data.sensors) {
		let value = data.sensors[key];
		var dataElement = document.getElementById(key + "-value");
		if (dataElement) {
			if (key.includes("event")) {
				if (value[1] != "") {
					dataElement.innerText = value[1];
					dataElement.className = '';
					dataElement.parentElement.className = 'sensor-event-triggered';
				}
				else {
					dataElement.className = 'sensor-event-old';
					dataElement.parentElement.className = 'sensor-event-clear';
				}
			} else {
				dataElement.innerText = value[1];
			}
		}
		var timestampElement = document.getElementById(key + "-timestamp");
		if (timestampElement) timestampElement.innerText = " (" + value[0] + ")";
	}

	// Update nonce values
	for (let key in data.nonce) {
		let value = data.nonce[key];
		var element = document.getElementById(key);

		if (element) element.value = value;
	}
		
	//notification and error popups
	if (data.error && data.error.length > 0) {
		errorDiv.innerHTML = '';
		for (let error of data.error) {
			errorDiv.innerHTML += error[0] + ': ' + error[1] + '<br>';
		}
		fadeIn(errorDiv, 1000);
	}
	else {
		fadeOut(errorDiv, 1000);
	}
	
	if (data.notification && data.notification.length > 0){
		notificationDiv.innerHTML = '';
		for (let notification of data.notification) {
			notificationDiv.innerHTML += notification[0] + ': ' + notification[1] + '<br>';
		}
		fadeIn(notificationDiv, 1000);
		}
	else {
		fadeOut(notificationDiv, 1000);
	}

	if(document.getElementById('autom_rpc')) {
		document.getElementById('autom_rpc').addEventListener('change', function () {
			var style = this.value.split(",")[1] == 'union' ? 'block' : 'none';
			for (i = 0; i < document.getElementsByClassName('hidden_rpc').length; i++) {
				document.getElementsByClassName('hidden_rpc')[i].style.display = style;
			}
		});
	}
}

setInterval(function() {
    updateValues();
}, 5000);

//UI animations for notifications and errors

function fadeOut(element) {
	if(!element)
		return;
    	
    var opacity = parseFloat(window.getComputedStyle(element).getPropertyValue("opacity")); 
    var timer = setInterval(function () {
        if (opacity <= 0.001){
            clearInterval(timer);
            element.style.display = 'none';
        }
        element.style.opacity = opacity;
        element.style.filter = 'alpha(opacity=' + opacity * 100 + ")";
        opacity -= opacity * 0.1;
    }, 50);
}

function fadeIn(element, ms)
{
	if(!element)
		return;

	element.style.display = "inline-block";
	element.style.visibility = "visible";

	var opacity = parseFloat(window.getComputedStyle(element).getPropertyValue("opacity")); 

	if(ms) {
		var timer = setInterval(function() {
			opacity += 50 / ms;
			if(opacity >= 1) {
				clearInterval(timer);
				opacity = 1;
			}
			element.style.opacity = opacity;
			element.style.filter = "alpha(opacity=" + opacity * 100 + ")";
		}, 50);
	}
	else
	{
		element.style.opacity = 1;
		element.style.filter = "alpha(opacity=1)";
	}
}