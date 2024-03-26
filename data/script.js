function fetchDataAndUpdate() {
    // Make AJAX request to fetch JSON data
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
        if (xhr.readyState == XMLHttpRequest.DONE) {
            if (xhr.status == 200) {
                var data = JSON.parse(xhr.responseText);
                // Update HTML elements with fetched data
                document.getElementById("indoortemp").innerText = data.indoorTemperature.toFixed(2) + "°F";
                document.getElementById("indoorhigh").innerText = data.indoorHigh.toFixed(2) + "°F";
                document.getElementById("indoorlow").innerText = data.indoorLow.toFixed(2) + "°F";
                document.getElementById("indoorhum").innerText = data.indoorHumidity.toFixed(2) + "%";

                document.getElementById("outdoortemp").innerText = data.outdoorTemperature.toFixed(2) + "°F";
                document.getElementById("outdoorhigh").innerText = data.outdoorHigh.toFixed(2) + "°F";
                document.getElementById("outdoorlow").innerText = data.outdoorLow.toFixed(2) + "°F";
                document.getElementById("outdoorhum").innerText = data.outdoorHumidity.toFixed(2)+"%";

                document.getElementById("watertemp").innerText = data.waterTemperature.toFixed(2)+"°F";;
                document.getElementById("waterhigh").innerText = data.waterHigh.toFixed(2) + "°F";
                document.getElementById("waterlow").innerText = data.waterLow.toFixed(2) + "°F";
                
                document.getElementById("light").checked = data.light;

            } else {
                console.error('Failed to fetch data');
            }
        }
    };
    xhr.open("GET", "/json", true);
    xhr.send();
}

function toggleCheckbox(element) {
    var xhr = new XMLHttpRequest();
    if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
    else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
    xhr.send();
  }

// Call fetchDataAndUpdate function on page load
window.onload = fetchDataAndUpdate;

// Call fetchDataAndUpdate function every 10 seconds
setInterval(fetchDataAndUpdate, 10000); // 10000 milliseconds = 10 seconds
